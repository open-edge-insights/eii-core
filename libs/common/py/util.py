import socket
import logging as log
import time


def check_port_availability(hostname, port):
    """
        Verifies port availability on hostname for accepting connection
        Arguments:
        hostname(str) - hostname of the machine
        port(str)     - port
    """
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    log.debug("Attempting to connect to {}:{}".format(hostname, port))
    numRetries = 1000
    retryCount = 0
    portUp = False
    while(retryCount < numRetries):
        if(sock.connect_ex((hostname, int(port)))):
            log.debug("{} port is up on {}".format(port, hostname))
            portUp = True
            break
        retryCount += 1
        time.sleep(0.1)
    return portUp
