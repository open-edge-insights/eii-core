import json
import os
import sys
import zmq
import zmq.auth
from zmq.auth.thread import ThreadAuthenticator


class ZMQ_PUBLISHER(object):
    """
    The ZMQ_PUBLISHER object to publish over a
    given topic
    """
    def __init__(self, address, public_keys_dir, secret_keys_dir):
        """
        Constructor to initialize the socket
        and create context
        """
        if public_keys_dir == "" and secret_keys_dir == "":
            self.dev_mode = True
        else:
            self.dev_mode = False

        if self.dev_mode:
            self.context = zmq.Context()
            self.publisher = self.context.socket(zmq.PUB)
            self.publisher.bind(address)
        else:
            # Check if certs exist
            if not (os.path.exists(public_keys_dir) and
                    os.path.exists(secret_keys_dir)):
                print("Certificates are missing - run generate_certificates.py\
                    script first")
                sys.exit(1)

            # Prepare our context and publisher
            self.context = zmq.Context()

            # Start an authenticator for this context.
            self.auth = ThreadAuthenticator(self.context)
            self.auth.start()

            # Tell authenticator to use the certificate in a directory
            self.auth.configure_curve(domain='*', location=public_keys_dir)
            self.publisher = self.context.socket(zmq.PUB)
            server_secret_file = os.path.join(secret_keys_dir,
                                              "zmq_server.key_secret")
            server_public, server_secret = \
                zmq.auth.load_certificate(server_secret_file)

            # Setting public and secret keys of server
            self.publisher.curve_secretkey = server_secret
            self.publisher.curve_publickey = server_public
            self.publisher.curve_server = True
            self.publisher.bind(address)

    def send(self, dataList=[], *args):
        """
        This method is used to publish the json meta-data
        and numpy frame

        Parameters :
        dataList : A list containing the topic, meta-data
                   and numpy frame to publish
        """
        flags = 0

        # Print the topic and message to be published
        print("Publish data %s over topic %s" % (dataList[1], dataList[0]))

        # Sending topic
        self.publisher.send_string(dataList[0], flags | zmq.SNDMORE)

        # Sending JSON meta-data
        self.publisher.send_json(dataList[1], flags | zmq.SNDMORE)

        # Sending numpy frame
        self.publisher.send(dataList[2], flags)

    def send_multipart(self, dataList=[], *args):
        """
        This method is used to publish data using send_multipart API

        Parameters :
        dataList : A list containing the elements to be published
        """

        # Print the topic and message to be published
        print("Publish data %s over topic %s" % (dataList[1], dataList[0]))

        # Sending topic
        self.publisher.send_multipart(dataList)

    def close_socket(self):
        # Close the socket and destroy the context
        self.publisher.close()
        self.context.term()
        if not self.dev_mode:
            self.auth.stop()
