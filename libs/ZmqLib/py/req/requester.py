import zmq
import json
import os
import sys
import zmq.auth
from zmq.auth.thread import ThreadAuthenticator


class ZMQ_REQUESTER(object):
    """
    The ZMQ_REQUESTER object to send request
    to responder
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
            self.requester = self.context.socket(zmq.REQ)
            self.requester.connect(address)
        else:
            # Check if certs exist
            if not (os.path.exists(public_keys_dir) and
                    os.path.exists(secret_keys_dir)):
                print("Certificates are missing - run generate_certificates.py\
                    script first")
                sys.exit(1)

            # Prepare our context and requester
            self.context = zmq.Context()

            # Start an authenticator for this context.
            self.auth = ThreadAuthenticator(self.context)
            self.auth.start()

            # Tell authenticator to use the certificate in a directory
            self.auth.configure_curve(domain='*', location=public_keys_dir)
            self.requester = self.context.socket(zmq.REQ)

            # We need two certificates, one for the client and one for
            # the server. The client must know the server's public key
            # to make a CURVE connection.
            client_secret_file = os.path.join(secret_keys_dir,
                                              "zmq_client0.key_secret")
            client_public, client_secret =\
                zmq.auth.load_certificate(client_secret_file)

            # Setting client server and public keys
            self.requester.curve_secretkey = client_secret
            self.requester.curve_publickey = client_public

            server_public_file = os.path.join(public_keys_dir, "server.key")
            server_public, _ = zmq.auth.load_certificate(server_public_file)

            # The client must know the server's public key to make a
            # CURVE connection.
            self.requester.curve_serverkey = server_public
            self.requester.connect(address)

    def receive(self, request):
        """
        This method is used to receive the json meta-data
        and numpy frame

        Parameters :
        request : request to be sent to responder

        Returns :
        meta_data       : JSON serialized string of meta-data
        frame_blob      : The published frame from ZMQ bus
        """

        # Flag to check last message
        flags = 0

        # Send the request
        self.requester.send_string(request, flags=flags)

        # Receive the JSON meta-data
        metaData = self.requester.recv_json(flags=flags)

        # Receive the numpy frame
        frame_blob = self.requester.recv(flags=flags)

        # Printing the received meta-data
        print("Received message {0}".format(metaData))

        return metaData, frame_blob

    def receive_multipart(self, request):
        """
        This method is used to receive data in multipart

        Parameters :
        request : request to be sent to responder

        Returns :
        meta_data       : JSON serialized string of meta-data
        frame_blob      : The published frame from ZMQ bus
        """

        # Flag to check last message
        flags = 0

        # Send the request
        self.requester.send_string(request, flags=flags)

        # Receive in multipart
        parts = self.requester.recv_multipart(flags=flags)

        # Printing the received meta-data
        print("Received message {0}".format(parts[0]))

        return parts[0], parts[1]

    def close_socket(self):
        # Close the socket and destroy the context
        self.requester.close()
        self.context.term()
        if not self.dev_mode:
            self.auth.stop()
