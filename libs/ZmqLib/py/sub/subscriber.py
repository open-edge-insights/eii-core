import zmq
import json
import os
import sys
import zmq.auth
from zmq.auth.thread import ThreadAuthenticator


class ZMQ_SUBSCRIBER(object):
    """
    The ZMQ_SUBSCRIBER object to subscriber over a
    desired topic
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
            self.subscriber = self.context.socket(zmq.SUB)
            self.subscriber.connect(address)
        else:
            # Check if certs exist
            if not (os.path.exists(public_keys_dir) and
                    os.path.exists(secret_keys_dir)):
                print("Certificates are missing - run generate_certificates.py\
                    script first")
                sys.exit(1)

            # Prepare our context and suscriber
            self.context = zmq.Context()

            # Start an authenticator for this context.
            self.auth = ThreadAuthenticator(self.context)
            self.auth.start()

            # Tell authenticator to use the certificate in a directory
            self.auth.configure_curve(domain='*', location=public_keys_dir)
            self.subscriber = self.context.socket(zmq.SUB)

            # We need two certificates, one for the client and one for
            # the server. The client must know the server's public key
            # to make a CURVE connection.
            client_secret_file = os.path.join(secret_keys_dir,
                                              "zmq_client.key_secret")
            client_public, client_secret =\
                zmq.auth.load_certificate(client_secret_file)

            # Setting client server and public keys
            self.subscriber.curve_secretkey = client_secret
            self.subscriber.curve_publickey = client_public

            server_public_file = os.path.join(public_keys_dir, "server.key")
            server_public, _ = zmq.auth.load_certificate(server_public_file)

            # The client must know the server's public key to make a
            # CURVE connection.
            self.subscriber.curve_serverkey = server_public
            self.subscriber.connect(address)

    def receive(self, topic):
        """
        This method is used to receive the json meta-data
        and numpy frame

        Parameters :
        topic : Topic for subscription

        Returns :
        meta_data       : JSON serialized string of meta-data
        frame_blob      : The published frame from ZMQ bus
        """
        # Select what topic to subscribe
        self.subscriber.setsockopt_string(zmq.SUBSCRIBE, topic)

        # Flag to check last message
        flags = 0

        # Receive the topic
        topic = self.subscriber.recv_string(flags=flags)

        # Receive the JSON meta-data
        metaData = self.subscriber.recv_json(flags=flags)

        # Receive the numpy frame
        frame_blob = self.subscriber.recv(flags=flags)

        # Printing the subscribed meta-data and topic
        print("Received message {0} on topic {1}".format(metaData, topic))

        return metaData, frame_blob

    def receive_multipart(self, topic):
        """
        This method is used to receive data in multipart

        Parameters :
        topic : Topic for subscription

        Returns :
        meta_data       : JSON serialized string of meta-data
        frame_blob      : The published frame from ZMQ bus
        """
        # Select what topic to subscribe
        self.subscriber.setsockopt_string(zmq.SUBSCRIBE, topic)

        # Flag to check last message
        flags = 0

        # Receive in multipart
        parts = self.subscriber.recv_multipart(flags=flags)

        # Printing the subscribed meta-data and topic
        print("Received message {0} on topic {1}".format(parts[1], parts[0]))

        return parts[1], parts[2]

    def close_socket(self):
        # Close the socket and destroy the context
        self.subscriber.close()
        self.context.term()
        if not self.dev_mode:
            self.auth.stop()
