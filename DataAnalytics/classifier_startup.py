import subprocess
import os.path
import argparse
import time
import stat
import logging
import socket


SUCCESS = 0
FAILURE = -1
KAPACITOR_PORT = 9092
logger = None
args = None
SOCKET_FILE = "/tmp/classifier"

def init_logger():
    """Initialize the logger
    """
    logging.basicConfig(level=logging.DEBUG,
                    format='%(asctime)s : %(levelname)s : \
                    %(name)s : [%(filename)s] :' +
                    '%(funcName)s : in line : [%(lineno)d] : %(message)s')
    return  logging.getLogger()


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
    parser.add_argument('--log-dir', dest='log_dir', default='logs',
                        help='Directory to for log files')
    return parser.parse_args()


def start_classifier():
    """Starts the classifier module
    """
    try:
        subprocess.call("python3.6 classifier.py --config "+ args.config +
         " --log-dir "+ args.log_dir + "&",shell=True)
        logger.info("classifier started successfully")
        return True
    except Exception as e:
        logger.info("Exception Occured in Starting the Classifier "+ str(e))
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
        subprocess.call("kapacitord -hostname "+host_name+" &",shell=True)
        logger.info("Started kapacitor Successfully...")
        return True
    except Exception as e:
        logger.info("Exception Occured in Starting the Kapacitor "+str(e))
        return False

def kapacitor_port_open(host_name):
    """Verify Kapacitor's port is ready for accepting connection
    """
    sock   = socket.socket(socket.AF_INET,socket.SOCK_STREAM)
    logger.info("Attempting to connect to Kapacitor on port 9092")
    result = sock.connect_ex((host_name,KAPACITOR_PORT))
    logger.info("Attempted  Kapacitor on port 9092 : Result " + str(result))
    if result == SUCCESS:
        logger.info("Successful in connecting to Kapacitor on port 9092")
        return True
    else:
        return False


def exit_with_failure_message():
    logger.info("Kapacitor hostname is not Set in the container.So exiting..")
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
    logger = init_logger()
    host_name = read_kapacitor_hostname()
    if not host_name:
        exit_with_failure_message()
    if (start_classifier() == True):
        grant_permission_socket()
        if(start_kapacitor(host_name) == True):
            enable_classifier_task(host_name)
        else:
            logger.info("Kapacitor is not starting.So Exiting...")
            exit(FAILURE)
        logger.info(
            "DataAnalytics Initialized Successfully.Ready to Receive the Data....")
        while(True):
            time.sleep(10)
    else:
        logger.info("Classifier is not able to start.Fix all the Errors &\
                    try again")


