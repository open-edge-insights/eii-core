import socket
import logging as log
import time
import os


def check_port_availability(hostname, port):
    """
        Verifies port availability on hostname for accepting connection
        Arguments:
        hostname(str) - hostname of the machine
        port(str)     - port
        Returns:
        portUp(bool)  - Boolean whether port is up
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


def get_topics_from_env(topic_type):
    """
        Returns a list of all topics the module needs to subscribe
        or publish
        Arguments:
        topic_type(str)  - type of the topic(pub/sub)
        Returns:
        topicsList(list) - list of topics in PubTopics
    """
    if topic_type == "pub":
        topicsList = os.environ["PubTopics"].split(",")
    elif topic_type == "sub":
        topicsList = os.environ["SubTopics"].split(",")

    return topicsList


def get_messagebus_config_from_env(topic, topic_type):
    """
        Returns the config associated with the corresponding topic
        Arguments:
        topic(str)      - name of the topic
        topic_type(str) - type of the topic(pub/sub)
        Returns:
        config(dict)    - config dict of corresponding topic
    """
    mode, address = os.environ[topic + "_cfg"].split(",")
    if mode == "zmq_tcp" and topic_type == "pub":
        host, port = address.split(":")
        config = {
                    "type": mode,
                    "zmq_tcp_publish":
                    {
                        "host":   host,
                        "port":   int(port)
                    }
                 }
    elif mode == "zmq_tcp" and topic_type == "sub":
        host, port = address.split(":")
        config = {
                    "type": mode,
                    topic:
                    {
                        "host":   host,
                        "port":   int(port)
                    }
                 }
    elif mode == "zmq_ipc":
        config = {
                    "type":   mode,
                    "socket_dir":   address
                 }
    return config
