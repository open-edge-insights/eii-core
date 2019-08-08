import socket
import logging as log
import time
import os


class Util:

    @staticmethod
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

    @staticmethod
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

    @staticmethod
    def get_messagebus_config(topic, topic_type,
                              zmq_clients, config_client, dev_mode):
        """
            Returns the config associated with the corresponding topic
            Arguments:
            topic(str)      - name of the topic
            topic_type(str) - type of the topic(pub/sub)
            zmq_clients - List of subscribers when used by publisher
                                    or publisher string when used by subscriber
            dev_mode(bool) - check if dev mode or prod mode
            config_client(Class Object) - Used to get keys value from ETCD.
            Returns:
            config(dict)    - config dict of corresponding topic
        """
        app_name = os.environ["AppName"]
        mode, address = os.environ[topic + "_cfg"].split(",")

        if mode == "zmq_tcp":
            host, port = address.split(":")
            config = {
                        "type": mode
            }
            host_port_details = {
                  "host":   host,
                  "port":   int(port)
            }
            topic_type = topic_type.lower()
            if topic_type == "pub":
                config["zmq_tcp_publish"] = host_port_details

                if not dev_mode:
                    allowed_clients = []
                    for subscriber in zmq_clients:
                        allowed_clients_keys = config_client.GetConfig(
                                        "/Publickeys/{0}".format(subscriber))

                        allowed_clients.append(allowed_clients_keys)

                    config["allowed_clients"] = allowed_clients
                    config["zmq_tcp_publish"]["server_secret_key"] = \
                        config_client.GetConfig("/" + app_name +
                                                "/private_key")

            elif topic_type == "sub":
                config[topic] = host_port_details
                if not dev_mode:
                    config[topic]["server_public_key"] = \
                        config_client.GetConfig("/Publickeys/{0}".
                                                format(zmq_clients))

                    config[topic]["client_public_key"] = \
                        config_client.GetConfig("/Publickeys/" + app_name)

                    config[topic]["client_secret_key"] = \
                        config_client.GetConfig("/" + app_name +
                                                "/private_key")
            else:
                raise ValueError("{} type is not valid".format(topic_type))
        elif mode == "zmq_ipc":
            config = {
                        "type":   mode,
                        "socket_dir":   address
                    }
        else:
            raise ValueError("{} mode is not valid".format(mode))

        return config
