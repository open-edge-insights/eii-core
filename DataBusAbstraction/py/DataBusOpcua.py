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
    # TODO: Find the root cause for this behavior and fix it
    # This is to address extra character that seems to come in the json
    # formatted `msg` C string in pyx callback
    # Here, number 123 and 125 refers to ascii values of "{" and "}"
    # respectively
    if msg[0] == 123 and msg[-1] != 125:
        msgByteArr = bytearray(msg)
        # This is to ignore the 2 additional hex chars eg. \x01\x02
        msg = bytes(msgByteArr[:-2])
    msg = msg.decode("utf-8")
    gQueue.put(msg)


class databOpcua:
    '''Creates and manages a databus OPCUA context'''

    def __init__(self, log):
        self.logger = log
        self.nsIndex = None
        self.direction = None
        self.namespace = None
        self.devMode = False

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
        certFile = contextConfig["certFile"]
        privateFile = contextConfig["privateFile"]
        trustFiles = [contextConfig["trustFile"]]

        if not certFile or not privateFile or not trustFiles:
            self.devMode = True

        self.direction = contextConfig["direction"]
        # Create default endpoint protocol for opcua from given endpoint
        endpoint = contextConfig["endpoint"]
        self.namespace = contextConfig["name"]

        errMsg = open62541W.ContextCreate(endpoint,
                                          self.direction,
                                          self.namespace,
                                          contextConfig["certFile"],
                                          contextConfig["privateFile"],
                                          [contextConfig["trustFile"]])
        pyErrorMsg = errMsg.decode()
        if pyErrorMsg != "0":
            self.logger.error("ContextCreate() API failed!")
            raise Exception(pyErrorMsg)

    def startTopic(self, topicConfig):
        '''
        Topic creation for the messagebus
        Arguments:
            topicConfig<dict>: Publish topic parameters
                <fields>
                "name": Topic name (in hierarchical form with '/' as delimiter)
                    <example> "root/level1/level2/level3"
                "type": Data type associated with the topic
        Return/Exception: Will raise Exception in case of errors
        '''
        pass

    def send(self, topicConfig, data):
        '''
	    Publish data on the topic
        Arguments:
            topicConfig: topicConfig for opcua, with topic name & it's type
            data: actual message
        Return/Exception: Will raise Exception in case of errors
        '''

        if self.direction == "PUB":
            # TODO: Support for different data types
            if type(data) == str:
                try:
                    errMsg = open62541W.Publish(topicConfig, data)
                    pyErrorMsg = errMsg.decode()
                    if pyErrorMsg != "0":
                        self.logger.error("Publish() API failed!")
                        raise Exception(pyErrorMsg)
                except Exception as e:
                    self.logger.error("{} Failure!!!".format(
                                                             self.send.__name__
                                                            ))
                    raise
            else:
                raise Exception("Wrong Data Type!!!")
        else:
            raise Exception("Wrong Bus Direction!!!")

    def receive(self, topicConfig, trig, queue):
        '''Subscribe data from the topic
        Arguments:
            topicConfig: topicConfig for opcua, with topic name & it's type
            trig: START/STOP to start/stop the subscription
            queue: A queue to which the message should be pushed on arrival
        Return/Exception: Will raise Exception in case of errors'''

        if (self.direction == "SUB") and (trig == "START"):
            global gQueue
            gQueue = queue
            errMsg = open62541W.Subscribe(topicConfig, trig, cbFunc)
            pyErrorMsg = errMsg.decode()
            if pyErrorMsg != "0":
                self.logger.error("Subscribe() API failed!")
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
        try:
            open62541W.ContextDestroy()
            self.logger.debug("OPCUA context is Terminated")
        except Exception as e:
            self.logger.error("{} Failure!!!".format(
                self.destroyContext.__name__))
            raise
