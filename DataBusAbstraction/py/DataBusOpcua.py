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
from Util.log import configure_logging, LOG_LEVELS
import open62541W

# TODO: This brings in a limitation of multiple different contexts
# if one wants to receive the callbacks registered for different
# streams from a single process. Need to implement this similar to
# StreamSubLib
func = None
gQueue = None


def cbFunc(topic, msg):
    gQueue.put(msg)


class databOpcua:
    '''Creates and manages a databus OPCUA context'''

    def __init__(self, log):
        self.logger = log
        self.nsIndex = None
        self.direction = None
        self.namespace = None

    def createContext(self, contextConfig):
        '''Creates a new messagebus context
        Arguments:
            contextConfig<dict>: Messagebus params to create the context
                <fields>
                "direction": PUB/SUB/NONE - Mutually exclusive
                "name": context namespace (PUB/SUB context namespaces
                        match)
                "endpoint": messagebus endpoint address
                    <format> proto://host:port/, proto://host:port/.../
                    <examples>
                    OPCUA -> opcua://0.0.0.0:65003
                    MQTT -> mqtt://localhost:1883
                    NATS -> nats://127.0.0.1:4222
                "certFile"   : server/client certificate file
                "privateFile": server/client private key file
                "trustFile"  : ca cert used to sign server/client cert
        Return/Exception: Will raise Exception in case of errors'''

        self.direction = contextConfig["direction"]
        if self.direction == "PUB":
            # Create default endpoint protocol for opcua from given endpoint
            endpoint = contextConfig["endpoint"]
            self.namespace = contextConfig["name"]

            hostPortPath = endpoint.split('//')[1]
            endpoint = "opc.tcp://" + hostPortPath
            self.logger.info("PUB:: {}".format(endpoint))

            hostPort = hostPortPath.split('/')[0]
            host = hostPort.split(':')[0]
            port = int(hostPort.split(':')[1])

            certFile = contextConfig["certFile"]
            privateFile = contextConfig["privateFile"]
            trustFiles = [contextConfig["trustFile"]]
            errMsg = open62541W.py_serverContextCreate(host,
                                                       port,
                                                       certFile,
                                                       privateFile,
                                                       trustFiles)
            pyErrorMsg = errMsg.decode()
            if pyErrorMsg != "0":
                self.logger.error("py_serverContextCreate() API failed!")
                raise Exception(pyErrorMsg)
        elif self.direction == "SUB":
            # Create default endpoint protocol for opcua from given endpoint
            endpoint = contextConfig["endpoint"]
            self.namespace = contextConfig["name"]

            hostPortPath = endpoint.split('//')[1]
            endpoint = "opc.tcp://" + hostPortPath
            self.logger.info("SUB:: {}".format(endpoint))

            hostPort = hostPortPath.split('/')[0]
            host = hostPort.split(':')[0]
            port = int(hostPort.split(':')[1])

            certFile = contextConfig["certFile"]
            privateFile = contextConfig["privateFile"]
            trustFiles = [contextConfig["trustFile"]]
            trustFiles = [contextConfig["trustFile"]]
            errMsg = open62541W.py_clientContextCreate(host,
                                                       port,
                                                       certFile,
                                                       privateFile,
                                                       trustFiles)
            pyErrorMsg = errMsg.decode()
            if pyErrorMsg != "0":
                self.logger.error("py_clientContextCreate() API failed!")
                raise Exception(pyErrorMsg)
        else:
            raise Exception("Wrong Bus Direction!!!")

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
            self.nsIndex = open62541W.py_serverStartTopic(self.namespace,
                                                          topicConfig["name"])
            if self.nsIndex == 100:
                self.logger.error("py_serverStartTopic() API failed!")
                raise Exception("Topic creation failed!")
        elif self.direction == "SUB":
            self.nsIndex = open62541W.py_clientStartTopic(self.namespace,
                                                          topicConfig["name"])
            if self.nsIndex == 100:
                self.logger.error("py_clientStartTopic() API failed!")
                raise Exception("Topic creation failed!")
        else:
            raise Exception("Wrong Bus Direction!!!")

    def send(self, topic, data):
        '''Publish data on the topic
        Arguments:
            topic: topic name as in the topicConfig
            data: actual message
        Return/Exception: Will raise Exception in case of errors'''

        if self.direction == "PUB":
            # TODO: Support for different data types
            if type(data) == str:
                try:
                    errMsg = open62541W.py_serverPublish(self.nsIndex, topic,
                                                         data)
                    pyErrorMsg = errMsg.decode()
                    if pyErrorMsg != "0":
                        self.logger.error("py_serverPublish() API failed!")
                        raise Exception(pyErrorMsg)
                except Exception as e:
                    self.logger.error("{} Failure!!!".format(self.send.__name__))
                    raise
            else:
                raise Exception("Wrong Data Type!!!")
        else:
            raise Exception("Wrong Bus Direction!!!")

    def receive(self, topic, trig, queue):
        '''Subscribe data from the topic
        Arguments:
            topic: topic name as in the topicConfig
            trig: START/STOP to start/stop the subscription
            queue: A queue to which the message should be pushed on arrival
        Return/Exception: Will raise Exception in case of errors'''

        if (self.direction == "SUB") and (trig == "START"):
            global gQueue
            gQueue = queue
            nsIndex = self.nsIndex
            errMsg = open62541W.py_clientSubscribe(nsIndex, topic, cbFunc)
            pyErrorMsg = errMsg.decode()
            if pyErrorMsg != "0":
                self.logger.error("py_clientSubscribe() API failed!")
                raise Exception(pyErrorMsg)
        elif (self.direction == "SUB") and (trig == "STOP"):
            # TODO: To be implemented - stop subscription
            pass
        else:
            raise Exception("Wrong Bus Direction or Trigger!!!")

    def stopTopic(self, topic):
        '''Delete topic
        Arguments:
            topicConfig<dict>: Topic parameters
                <fields>
                "name": Topic name
                "type": Data type associated with the topic
        Return/Exception: Will raise Exception in case of errors'''

        # TODO:
        # if self.direction == "PUB":
        #    objs = self.server.get_objects_node()
        #    obj_itr = objs
        #    topic_ls = topic.split("/")
        #    for topic in topic_ls[0:]:
        #        try:
        #            child = obj_itr.get_child("{}:{}".format(self.nsIndex,
        #                                                      topic))
        #        except Exception as e:
        #            print(type(e).__name__)
        #            return
        #        obj_itr = child
        #    # Now get & delete the variable object
        #    var = obj_itr.get_child("{}:{}".format(self.nsIndex,
        #                                       "{}_var".format(topic_ls[-1])))
        #    # delete the var & object
        #    var.delete()

        #    obj_itr.delete()
        return

    def destroyContext(self):
        '''Destroy the messagebus context'''

        if self.direction == "PUB":
            self.logger.info("OPCUA Server Stopping...")
            open62541W.py_serverContextDestroy()
            self.logger.error("OPCUA Server Stopped")
        elif self.direction == "SUB":
            try:
                self.logger.info("OPCUA Client Disconnecting...")
                open62541W.py_clientContextDestroy()
                self.logger.info("OPCUA Client Disconnected")
            except Exception as e:
                self.logger.error("{} Failure!!!".format(
                    self.destroyContext.__name__))
                raise
        else:
            raise Exception("Wrong Bus Direction!!!")
