
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
from influxdb import InfluxDBClient
import socket
from DataAgent.da_grpc.client.py.\
    client_internal.client import GrpcInternalClient
from concurrent.futures import ThreadPoolExecutor
from uuid import uuid4
import threading
from collections import defaultdict
from Util.format_converter import lf_to_json_converter
from Util.exception import DAException


class StreamSubLib:
    '''This Class creates a subscription to influx db and provides
    the streams as callbacks like a pub-sub message bus'''

    def init(self, log_level=logging.INFO):
        '''Creates a subscription to to influxdb
        Arguments:
        log_level:(Optional) Log levels are used to track the severity
                  events
        '''

        logging.basicConfig(level=log_level)
        self.log = logging.getLogger(__name__)
        try:
            self.maxbytes = 1024
            self.hostname = socket.gethostname()
            self.stream_map = defaultdict(list)
            self.callback_executor = ThreadPoolExecutor()
            client = GrpcInternalClient()
            self.config = client.GetConfigInt("InfluxDBCfg")
        except Exception as e:
            raise DAException("Seems to be some issue with gRPC server." +
                              "Exception: {0}".format(e))

        # Creates the influxDB client handle.
        try:
            self.influx_c = InfluxDBClient(self.config["Host"],
                                           self.config["Port"],
                                           self.config["UserName"],
                                           self.config["Password"],
                                           self.config["DBName"],
                                           True
                                           if self.config["Ssl"] == "True"
                                           else False,
                                           True
                                           if self.config["VerifySsl"] ==
                                           "True"
                                           else False)
        # listenport=0 will take a random available ephemeral port
            self.listenerport = 0
        except Exception as e:
            raise DAException("Failed creating the InfluxDB client " +
                              "Exception: {0}".format(e))

        try:
            self.database = self.config["DBName"]

            # create subscription query cannot parse "-",
            # hence replacing it with "_"

            self.subscriptionName = (self.database + "_" + str(
                uuid4())).replace("-", "_")
            self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

            self.sock.bind((
                socket.gethostbyname(self.hostname), self.listenerport))

            subscriptionLink = 'udp://' + socket.gethostbyname(
                self.hostname) + ':'+str(self.sock.getsockname()[1])

            query_in = "create subscription " + self.subscriptionName + \
                " ON " + self.database + ".autogen DESTINATIONS ANY \'" + \
                subscriptionLink + "\'"

            self.influx_c.query(query_in)
            self.log.info(
                "Subscription successfull on database: " + self.database)

            self.initialized = True

            self.listening_thread = threading.Thread(
                target=self.listen_on_udp_server)
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

    def send_to_callback(self, data, data_stream):
        cbs = self.stream_map[data_stream]
        for cb in cbs:
            cb(data)

    def listen_on_udp_server(self):
        ''' Receives data from the socket and converts it to json format
        and sends the formated data to callback'''

        while(self.initialized):
            data, res_addr = self.sock.recvfrom(self.maxbytes)
            data = data.decode()
            data_stream = data.split(" ")
            measurement = data_stream[0].split(",")
            if measurement[0] in self.stream_map:
                data = lf_to_json_converter(data)
                self.callback_executor.submit(
                    self.send_to_callback, data, measurement[
                        0])

    def Subscribe(self, streamName, cb):
        ''' Mapping of stream name with the associated callbacks.
        Arguments:
            streamName(string): measurement to which subscription is made.
            cb(function): Callback function to which data (from influxdb) will
            be sent back to.
        '''
        self.stream_map[streamName].append(cb)
