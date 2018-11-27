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

import paho.mqtt.client as mqtt
import logging

logging.basicConfig(level=logging.DEBUG,
                    format='%(asctime)s : %(levelname)s : \
                    %(name)s : [%(filename)s] :' +
                    '%(funcName)s : in line : [%(lineno)d] : %(message)s')
logging.getLogger("opcua").setLevel(logging.WARNING)
logger = logging.getLogger(__name__)


def onMsgCb(client, udata, msg):
    '''cb function on message arrival'''

    # logger.info("topic: {}".format(msg.topic))
    # logger.info("msg: {}".format(str(msg.payload.decode("utf-8"))))
    udata.subElements[msg.topic]["queue"].put(str(msg.payload.decode("utf-8")))


class databMqtt:
    '''Creates and manages a databus MQTT context'''

    def createContext(self, contextConfig):
        '''Creates a new messagebus context
        Arguments:
            contextConfig<dict>: Messagebus params to create the context
                <fields>
                "direction": PUB/SUB/NONE - Mutually exclusive
                "name": context namespace (PUB/SUB context namespaces should
                        match)
                "endpoint": messagebus endpoint address
                    <format> proto://host:port/, proto://host:port/.../
                    <examples>
                    OPCUA -> opcua://0.0.0.0:4840
                    MQTT -> mqtt://localhost:1883/
                    NATS -> nats://127.0.0.1:4222/
        Return/Exception: Will raise Exception in case of errors'''

        self.direction = contextConfig["direction"]
        # self.ns = contextConfig["name"]
        # Create default endpoint protocol for mqtt from given endpoint
        endpoint = contextConfig["endpoint"]
        host = endpoint.split('//')[1].split(':')[0]
        port = endpoint.split('//')[1].split(':')[1].split('/')[0]
        logger.info("Host: {}, port: {}".format(host, port))
        if self.direction == "PUB":
            try:
                self.client = mqtt.Client(userdata=self)
                # TODO: cb registrations
                # self.client.on_publish = self._on_publish
                # self.client.on_connect = self._on_connect
                self.client.connect(host, int(port))
                self.client.loop_start()
            except Exception as e:
                logger.error("{} Failure!!!".format(
                    self.createContext.__name__))
                raise
        if self.direction == "SUB":
            try:
                self.client = mqtt.Client(userdata=self)
                # TODO: cb registrations
                # self.client.on_connect = self._on_connect
                self.client.on_message = onMsgCb
                self.client.connect(host, int(port))
                self.client.loop_start()
            except Exception as e:
                logger.error("{} Failure!!!".format(
                    self.createContext.__name__))
                raise

    def startTopic(self, topicConfig):
        '''Topic creation for the messagebus
        Arguments:
            topicConfig<dict>: Publish topic parameters
                <fields>
                "name": Topic name (in hierarchical form with '/' as delimiter)
                    <example> "root/level1/level2/level3"
                "type": Data type associated with the topic
        Return/Exception: Will raise Exception in case of errors'''

        if self.direction == "PUB":
            return
        if self.direction == "SUB":
            self.subElements[topicConfig["name"]] = {"queue": None,
                                                     "type": None}
            self.subElements[topicConfig["name"]]["type"] = topicConfig["type"]
            return

    def send(self, topic, data):
        '''Publish data on the topic
        Arguments:
            topic: topic name as in the topicConfig
            data: actual message
        Return/Exception: Will raise Exception in case of errors'''

        try:
            if type(data) == str:
                self.client.publish(topic, data)
        except Exception as e:
            logger.error("{} Failure!!!".format(self.send.__name__))
            raise

    def receive(self, topic, trig, queue=None):
        '''Subscribe data from the topic
        Arguments:
            topic: topic name as in the topicConfig
            trig: START/STOP to start/stop the subscription
            queue: A queue to which the message should be pushed on arrival
        Return/Exception: Will raise Exception in case of errors'''

        if (self.direction == "SUB") and (trig == "START") and (queue is
                                                                not None):
            # logger.info("MQTT recieve START....")
            # logger.info(topic)
            self.subElements[topic]["queue"] = queue
            # logger.info(self.subElements)
            try:
                self.client.subscribe(topic)
            except Exception as e:
                logger.error("{} Failure!!!".format(self.receive.__name__))
                raise
        elif (self.direction == "SUB") and (trig == "STOP"):
            # logger.info("MQTT recieve STOP....")
            # logger.info(topic)
            try:
                self.client.unsubscribe(topic)
                self.subElements[topic]["queue"] = None
            except Exception as e:
                logger.error("{} Failure!!!".format(self.receive.__name__))
                raise
        else:
            raise Exception("Wrong Bus Direction or Trigger!!!")

    def stopTopic(self, topicConfig):
        '''Delete topic
        Arguments:
            topicConfig<dict>: Topic parameters
                <fields>
                "name": Topic name (in hierarchical form with '/' as delimiter)
                    <example> "root/level1/level2/level3"
                "type": Data type associated with the topic
        Return/Exception: Will raise Exception in case of errors'''

        if self.direction == "PUB":
            return
        if self.direction == "SUB":
            return

    def destroyContext(self):
        '''Destroy the messagebus context'''

        try:
            logger.info("MQTT Client Disconnecting...")
            if self.direction == "PUB":
                self.client.loop_stop()
                self.client.disconnect()
            elif self.direction == "SUB":
                self.client.loop_stop()
                self.client.disconnect()
            else:
                raise Exception("Wrong Bus Direction!!!")
            logger.info("MQTT Client Disconnected")
        except Exception as e:
            logger.error("{} Failure!!!".format(self.destroyContext.__name__))
            raise

    def __init__(self):
        self.client = None
        self.direction = None
        # self.ns = None
        self.subElements = {}
        return
