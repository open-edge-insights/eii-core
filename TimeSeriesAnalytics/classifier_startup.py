import subprocess
import os.path
import argparse
import time
import stat
import json
import socket
import datetime
from libs.ConfigManager import ConfigManager
from Util.log import configure_logging, LOG_LEVELS
from distutils.util import strtobool
import os
from Util.util \
import (write_certs,
        create_decrypted_pem_files,
        check_port_availability,
        delete_certs)

ETCD_CLIENT_CERT = "/run/secrets/etcd_Kapacitor_cert"
ETCD_CLIENT_KEY = "/run/secrets/etcd_Kapacitor_key"
ETCD_CA_CERT = "/run/secrets/ca_etcd"
KAPACITOR_CERT = "/etc/ssl/kapacitor/kapacitor_server_certificate.pem"
KAPACITOR_KEY = "/etc/ssl/kapacitor/kapacitor_server_key.pem"
KAPACITOR_CA = "/etc/ssl/ca/ca_certificate.pem"

SUCCESS = 0
FAILURE = -1
KAPACITOR_PORT = 9092
KAPACITOR_NAME = 'kapacitord'
logger = None
args = None
POINT_SOCKET_FILE = "/tmp/point_classifier"


def parse_args():
    """Parse command line arguments
    """
    parser = argparse.ArgumentParser()
    parser.add_argument('--log-name', help='Logfile name')
    parser.add_argument('--log-dir', dest='log_dir', default='logs',
                        help='Directory to for log files')
    return parser.parse_args()


def start_classifier(logFileName):
    """Starts the classifier module
    """
    try:
        subprocess.call("./point_classifier &", shell=True)
        logger.info("classifier started successfully")
        return True
    except Exception as e:
        logger.info("Exception Occured in Starting the Classifier " + str(e))
        return False


def grant_permission_socket(socket_path):
    """Grants chmod 0x777 permission for the classifier's socket file
    """
    while not os.path.exists(socket_path):
        pass
    logger.info("Socket file present...")
    if os.path.isfile(socket_path):
        os.chmod(socket_path, stat.S_IRWXU | stat.S_IRWXG | stat.S_IRWXO)
    logger.info("Permission Granted for Socket files")


def write_cert(file_name, cert):
    """Write certificate to given file path
    """
    try:
        with open(file_name, 'wb+') as fd:
            fd.write(cert.encode())
    except Exception as e:
        logger.debug("Failed creating file: {}, Error: {} ".format(file_name,
                                                                    e))

def read_config(client, dev_mode):
    """Read the configuration from etcd
    """
    app_name = os.environ["AppName"]
    config_key_path = "config"
    configfile = client.GetConfig("/{0}/{1}".format(
                 app_name, config_key_path))
    config = json.loads(configfile)
    os.environ["KAPACITOR_INFLUXDB_0_USERNAME"] = config["influxdb"]["username"]
    os.environ["KAPACITOR_INFLUXDB_0_PASSWORD"] = config["influxdb"]["password"]

    if not dev_mode:
        cert = client.GetConfig("/{0}/{1}".format(
               app_name, "server_cert"))
        write_cert(KAPACITOR_CERT, cert)
        key = client.GetConfig("/{0}/{1}".format(
               app_name, "server_key"))
        write_cert(KAPACITOR_KEY, key)
        ca = client.GetConfig("/{0}/{1}".format(
               app_name, "ca_cert"))
        write_cert(KAPACITOR_CA, ca)



def start_kapacitor(client, host_name, dev_mode):
    """Starts the kapacitor Daemon in the background
    """
    HTTP_SCHEME = "http://"
    HTTPS_SCHEME = "https://"
    KAPACITOR_HOSTNAME_PORT = os.environ["KAPACITOR_URL"].split("://")[1]
    INFLUXDB_HOSTNAME_PORT = os.environ["KAPACITOR_INFLUXDB_0_URLS_0"].split(
        "://")[1]
    try:
        if dev_mode:
            kapacitor_conf = "/etc/kapacitor/kapacitor_devmode.conf"
            os.environ["KAPACITOR_URL"] = "{}{}".format(HTTP_SCHEME,
                                                        KAPACITOR_HOSTNAME_PORT)
            os.environ["KAPACITOR_UNSAFE_SSL"] = "true"
            os.environ["KAPACITOR_INFLUXDB_0_URLS_0"] = "{}{}".format(
                HTTP_SCHEME, INFLUXDB_HOSTNAME_PORT)
        else:
            # Populate the certificates for kapacitor server
            kapacitor_conf = "/etc/kapacitor/kapacitor.conf"
            os.environ["KAPACITOR_URL"] = "{}{}".format(HTTPS_SCHEME,
                                                        KAPACITOR_HOSTNAME_PORT)
            os.environ["KAPACITOR_UNSAFE_SSL"] = "false"
            os.environ["KAPACITOR_INFLUXDB_0_URLS_0"] = "{}{}".format(
                HTTPS_SCHEME, INFLUXDB_HOSTNAME_PORT)

        read_config(client, dev_mode)
        subprocess.call("kapacitord -hostname " + host_name +
                        " -config " + kapacitor_conf + " &", shell=True)

        logger.info("Started kapacitor Successfully...")
        return True
    except Exception as e:
        logger.info("Exception Occured in Starting the Kapacitor " + str(e))
        return False


def process_zombie(process_name):
    """Checks the given process is Zombie State & returns True or False
    """
    try:
        out = subprocess.check_output('ps -eaf | grep ' + process_name +
                                      '| grep -v grep | grep defunct | wc -l',
                                      shell=True).strip()
        return True if (out == b'1') else False
    except Exception as e:
        logger.info("Exception Occured in Starting Kapacitor " + str(e))


def kapacitor_port_open(host_name):
    """Verify Kapacitor's port is ready for accepting connection
    """
    if process_zombie(KAPACITOR_NAME):
        exit_with_failure_message("Kapacitor fail to start.Please verify the \
            ia_data_analytics logs for UDF/kapacitor Errors.")
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    logger.info("Attempting to connect to Kapacitor on port 9092")
    result = sock.connect_ex((host_name, KAPACITOR_PORT))
    logger.info("Attempted  Kapacitor on port 9092 : Result " + str(result))
    if result == SUCCESS:
        logger.info("Successful in connecting to Kapacitor on port 9092")
        return True
    else:
        return False


def exit_with_failure_message(message):
    logger.error(message)
    exit(FAILURE)


def enable_classifier_task(host_name):
    """Enable the classifier TICK Script using the kapacitor CLI
    """
    retry_count = 5
    retry = 0
    while not kapacitor_port_open(host_name):
        time.sleep(1)
    logger.info("Kapacitor Port is Open for Communication....")
    while(retry < retry_count):
        definePointClCmd = ["kapacitor", "-skipVerify", "define",
                            "point_classifier_task", "-tick",
                            "point_classifier.tick"]

        if (subprocess.check_call(definePointClCmd) == SUCCESS):
            definePointClCmd = ["kapacitor", "-skipVerify", "enable",
                                "point_classifier_task"]

            if (subprocess.check_call(definePointClCmd) == SUCCESS):
                logger.info("Kapacitor Tasks Enabled Successfully")
                break
            else:
                logger.info("ERROR:Cannot Communicate to Kapacitor. ")
        else:
            logger.info("ERROR:Cannot Communicate to Kapacitor. ")
        logger.info("Retrying Kapacitor Connection")
        time.sleep(0.0001)
        retry = retry + 1

    try:
        file_list = ["/etc/ssl/kapacitor/kapacitor_server_certificate.pem",
                     "/etc/ssl/kapacitor/kapacitor_server_key.pem"]
        delete_certs(file_list)
    except Exception as e:
        logger.error("Exception Occured while removing kapacitor certs")


if __name__ == '__main__':

    args = parse_args()
    dev_mode = bool(strtobool(os.environ["DEV_MODE"]))
    # Initializing Etcd to set env variables
    conf = {
        "certFile": "",
        "keyFile": "",
        "trustFile": ""
    }
    if not dev_mode:
        conf = {
            "certFile": ETCD_CLIENT_CERT,
            "keyFile": ETCD_CLIENT_KEY,
            "trustFile": ETCD_CA_CERT
        }
    cfg_mgr = ConfigManager()
    config_client = cfg_mgr.get_config_client("etcd", conf)
    currentDateTime = str(datetime.datetime.now())
    listDateTime = currentDateTime.split(" ")
    currentDateTime = "_".join(listDateTime)
    logFileName = 'dataAnalytics_' + currentDateTime + '.log'

    logger = configure_logging(os.environ['PY_LOG_LEVEL'].upper(), logFileName,
                               args.log_dir, __name__)
    logger.info("=============== STARTING data_analytics ==============")

    host_name = os.environ["KAPACITOR_SERVER"]
    if not host_name:
        exit_with_failure_message('Kapacitor hostname is not Set in the \
         container.So exiting..')
    if (start_classifier(logFileName) is True):
        grant_permission_socket(POINT_SOCKET_FILE)
        if(start_kapacitor(config_client, host_name, dev_mode) is True):
            enable_classifier_task(host_name)
        else:
            logger.info("Kapacitor is not starting.So Exiting...")
            exit(FAILURE)
        logger.info(
            "DataAnalytics Initialized Successfully.Ready to Receive the \
            Data....")
        while(True):
            time.sleep(10)
    else:
        logger.info("Classifier is not able to start.Fix all the Errors &\
                    try again")
