
"""
Copyright (c) 2018 Intel Corporation.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
"""

import logging
import ssl
from influxdb import InfluxDBClient
import socket
from random import randint
from DataAgent.da_grpc.client.py.\
    client_internal.client import GrpcInternalClient
from concurrent.futures import ThreadPoolExecutor
from uuid import uuid4
import threading
from collections import defaultdict
from Util.format_converter import lf_to_json_converter
from Util.exception import DAException
from Util.util import (write_certs, create_decrypted_pem_files)
from http.server import BaseHTTPRequestHandler, HTTPServer


callback_executor = ThreadPoolExecutor()
hostname = socket.gethostname()

CLIENT_CERT = "/etc/ssl/grpc_int_ssl_secrets/" + \
    "grpc_internal_client_certificate.pem"
CLIENT_KEY = "/etc/ssl/grpc_int_ssl_secrets/" + \
    "grpc_internal_client_key.pem"
CA_CERT = "/etc/ssl/grpc_int_ssl_secrets/" + \
    "ca_certificate.pem"


class StreamSubLib:
    '''This Class creates a subscription to influx db and provides
    the streams as callbacks like a pub-sub message bus'''

    def init(self, log_level=logging.INFO):
        '''Creates a subscription to to influxdb
        Arguments:
        log_level:(Optional) Log levels are used to track the severity
                  events
        '''

        self.stream_map = defaultdict(list)
        self.port = (randint(49153, 65500))
        logging.basicConfig(level=log_level)
        self.log = logging.getLogger(__name__)
        try:
            self.maxbytes = 1024
            client = GrpcInternalClient(CLIENT_CERT, CLIENT_KEY, CA_CERT)
            self.config = client.GetConfigInt("InfluxDBCfg")

            self.certBlobMap = client.GetConfigInt("StrmLibServerCert")
        except Exception as e:
            raise DAException("Seems to be some issue with GRPC Server." +
                              "Exception: {0}".format(e))

        # Creates the influxDB client handle.
        try:
            srcFiles = [CA_CERT]
            filesToDecrypt = ["/etc/ssl/ca/ca_certificate.pem"]
            create_decrypted_pem_files(srcFiles, filesToDecrypt)

            self.influx_c = InfluxDBClient(self.config["Host"],
                                           self.config["Port"],
                                           self.config["UserName"],
                                           self.config["Password"],
                                           self.config["DBName"],
                                           True
                                           if self.config["Ssl"] ==
                                           "True"
                                           else False,
                                           filesToDecrypt[0])

        except Exception as e:
            raise DAException("Failed creating the InfluxDB client " +
                              "Exception: {0}".format(e))

        try:
            self.database = self.config["DBName"]

            # create subscription query cannot parse "-",
            # hence replacing it with "_"

            self.subscriptionName = (self.database + "_" + str(
                uuid4())).replace("-", "_")

            subscriptionLink = 'https://' + hostname + ':' + str(self.port)
            query_in = "create subscription " + self.subscriptionName + \
                " ON " + self.database + ".autogen DESTINATIONS ANY \'" + \
                subscriptionLink + "\'"

            self.influx_c.query(query_in)
            self.log.info(
                "Subscription successfull on database")

            self.initialized = True
            self.listening_thread = threading.Thread(
                target=self.listen_on_server)
            self.listening_thread.start()
        except Exception as e:
            self.log.error("Subscription failed due to : " + str(e))
            raise e

    def deinit(self):
        ''' Removes the subscription from influxdb '''

        remove_subscription = "drop subscription " + self.subscriptionName + \
            " ON " + self.database + ".autogen"

        self.influx_c.query(remove_subscription)
        self.initialized = False

    def listen_on_server(self):
        ''' Receives data from the socket and converts it to json format
        and sends the formated data to callback'''
        try:
            path = "/etc/ssl/streamsublib"
            crt = path + "/streamsublib_server_certificate.pem"
            key = path + "/streamsublib_server_key.pem"
            file_list = [crt, key]
            write_certs(file_list, self.certBlobMap)

            def handler(*args):
                HttpHandlerFunc(self.stream_map, *args)

            httpd = HTTPServer((socket.gethostbyname(
                hostname), self.port), handler)

            httpd.socket = ssl.wrap_socket(
                httpd.socket, server_side=True, keyfile=key, certfile=crt)

            while(self.initialized):
                httpd.handle_request()

        except Exception as e:
            self.log.error("Error in connection due to : " + str(e))
            raise e

    def Subscribe(self, streamName, cb):
        ''' Mapping of stream name with the associated callbacks.
        Arguments:
            streamName(string): measurement to which subscription is made.
            cb(function): Callback function to which data (from influxdb) will
            be sent back to.
        '''
        self.stream_map[streamName].append(cb)


class HttpHandlerFunc(BaseHTTPRequestHandler):

    def __init__(self, stream_map, *args):
        self.stream_map = stream_map
        BaseHTTPRequestHandler.__init__(self, *args)

    def send_to_callback(self, data, data_stream):
        cbs = self.stream_map[data_stream]
        for cb in cbs:
            cb(data)

    def do_POST(self):
        postdata = (self.rfile.read(int(
            self.headers['Content-Length'])).decode("UTF-8"))

        data_stream = postdata.split(" ")
        measurement = data_stream[0].split(",")
        if measurement[0] in self.stream_map:
            data = lf_to_json_converter(postdata)
            self.send_to_callback(data, measurement[0])
