import json
import os
import sys
import zmq
import zmq.auth
from zmq.auth.thread import ThreadAuthenticator


class ZMQ_RESPONDER(object):
    """
    The ZMQ_RESPONDER object to send meta-data
    and image frame
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
            self.responder = self.context.socket(zmq.REP)
            self.responder.bind(address)
        else:
            # Check if certs exist
            if not (os.path.exists(public_keys_dir) and
                    os.path.exists(secret_keys_dir)):
                print("Certificates are missing - run generate_certificates.py\
                    script first")
                sys.exit(1)

            # Prepare our context and responder
            self.context = zmq.Context()

            # Start an authenticator for this context.
            self.auth = ThreadAuthenticator(self.context)
            self.auth.start()

            # Tell authenticator to use the certificate in a directory
            self.auth.configure_curve(domain='*', location=public_keys_dir)
            self.responder = self.context.socket(zmq.REP)
            server_secret_file = os.path.join(secret_keys_dir,
                                              "zmq_server.key_secret")
            server_public, server_secret = \
                zmq.auth.load_certificate(server_secret_file)

            # Setting public and secret keys of server
            self.responder.curve_secretkey = server_secret
            self.responder.curve_publickey = server_public
            self.responder.curve_server = True
            self.responder.bind(address)

    def send(self, callback):
        """
        This method is used to send the json meta-data
        and numpy frame

        Parameters :
        callback : The callback to be called once the server
        gets the request from client
        """
        flags = 0

        # Receive the request
        request = self.responder.recv_string(flags)

        # Call the callback using the obtained request
        dataList = callback(request)

        # Print the message to be sent
        print("Sending data %s" % (dataList[0]))

        # Sending JSON meta-data
        self.responder.send_json(dataList[0], flags | zmq.SNDMORE)

        # Sending numpy frame
        self.responder.send(dataList[1], flags)

    def send_multipart(self, callback):
        """
        This method is used to send data using send_multipart API

        Parameters :
        callback : The callback to be called once the server
        gets the request from client
        """
        flags = 0

        # Receive the request
        request = self.responder.recv_string(flags)

        # Call the callback using the obtained request
        dataList = callback(request)

        # Print the message to be sent
        print("Sending data %s" % (dataList[0]))

        # Sending JSON meta-data and numpy frame
        self.responder.send_multipart(dataList)

    def close_socket(self):
        # Close the socket and destroy the context
        self.responder.close()
        self.context.term()
        if not self.dev_mode:
            self.auth.stop()
