import subprocess
import os.path
import argparse
import time
import stat
import logging
import socket
import datetime
from Util.log import configure_logging, LOG_LEVELS

SUCCESS = 0
FAILURE = -1
KAPACITOR_PORT = 9092
KAPACITOR_NAME = 'kapacitord'
logger = None
args = None
SOCKET_FILE = "/tmp/classifier"

logger = None


def read_kapacitor_hostname():
    """Get the Kapacitor Hostname from ENV
    """
    if 'KAPACITOR_HOSTNAME' in os.environ:
        return os.environ['KAPACITOR_HOSTNAME']
    else:
        return None


def parse_args():
    """Parse command line arguments
    """
    parser = argparse.ArgumentParser()
    parser.add_argument('--config', dest='config', default='factory.json',
                        help='JSON configuration file')
    parser.add_argument('--log', choices=LOG_LEVELS.keys(), default='DEBUG',
                        help='Logging level (df: DEBUG)')

    parser.add_argument('--log-name', help='Logfile name')

    parser.add_argument('--log-dir', dest='log_dir', default='logs',
                        help='Directory to for log files')
    return parser.parse_args()


def start_classifier(logFileName):
    """Starts the classifier module
    """
    try:

        subprocess.call("python3.6 classifier.py --config " + args.config +
            " --log-dir " + args.log_dir + " --log-name " + logFileName +
            " --log " + args.log.upper() + "&", shell=True)
        logger.info("classifier started successfully")
        return True
    except Exception as e:
        logger.info("Exception Occured in Starting the Classifier " + str(e))
        return False


def grant_permission_socket():
    """Grants chmod 0x777 permission for the classifier's socket file
    """
    while not os.path.exists(SOCKET_FILE):
        pass
    logger.info("Socket file present...")
    if os.path.isfile(SOCKET_FILE):
        os.chmod(SOCKET_FILE, stat.S_IRWXU | stat.S_IRWXG | stat.S_IRWXO)
    logger.info("Permission Granted for Socket files")


def start_kapacitor(host_name):
    """Starts the kapacitor Daemon in the background
    """
    try:
        subprocess.call("kapacitord -hostname "+host_name+" &", shell=True)
        logger.info("Started kapacitor Successfully...")
        return True
    except Exception as e:
        logger.info("Exception Occured in Starting the Kapacitor "+str(e))
        return False


def process_zombie(process_name):
    """Checks the given process is Zombie State & returns True or False
    """
    try:
        out = subprocess.check_output('ps -eaf | grep ' + process_name +
         '| grep -v grep | grep defunct | wc -l', shell=True).strip()
        return True if (out == b'1') else False
    except Exception as e:
        logger.info("Exception Occured in Starting Kapacitor "+str(e))
    

def kapacitor_port_open(host_name):
    """Verify Kapacitor's port is ready for accepting connection
    """
    if process_zombie(KAPACITOR_NAME):
        exit_with_failure_message("Kapacitor failed to start.Please verify the \
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
    logger.info(message)
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
        if (subprocess.check_call(
            ["kapacitor", "define", "classifier_task", "-tick",
             "classifier.tick"]) == SUCCESS):
            if(subprocess.check_call(
                ["kapacitor", "enable", "classifier_task"]) == SUCCESS):
                logger.info("Kapacitor Tasks Enabled Successfully")
                break
            else:
                logger.info("ERROR:Cannot Communicate to Kapacitor. ")
        else:
            logger.info("ERROR:Cannot Communicate to Kapacitor. ")
        logger.info("Retrying Kapacitor Connection")
        time.sleep(0.0001)
        retry = retry + 1

if __name__ == '__main__':
    args = parse_args()

    currentDateTime = str(datetime.datetime.now())
    listDateTime = currentDateTime.split(" ")
    currentDateTime = "_".join(listDateTime)
    logFileName = 'dataAnalytics_' + currentDateTime + '.log'

    logger = configure_logging(args.log.upper(), logFileName, 
                    args.log_dir, __name__)
    host_name = read_kapacitor_hostname()
    if not host_name:
        exit_with_failure_message('Kapacitor hostname is not Set in the \
         container.So exiting..')
    if (start_classifier(logFileName) == True):
        grant_permission_socket()
        if(start_kapacitor(host_name) == True):
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
