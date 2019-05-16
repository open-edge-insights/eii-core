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

from DataBusOpcua import databOpcua
from queue import Queue
from threading import Thread, Lock
from threading import Event
import logging
from Util.log import configure_logging, LOG_LEVELS

busTypes = {"OPCUA": "opcua:"}


def worker(qu, ev, fn, log):
    '''Worker function to fetch data from queue and trigger cb'''
    logger = log
    while True:
        if ev.is_set():
            logger.info("Worker Done")
            break
        try:
            subDict = qu.get(timeout=2)
            fn(subDict["topic"], subDict["data"])
            qu.task_done()
        except Exception:
            # TODO: pass only for Empty exception
            # logger.error("Exception passed")
            pass
    return

# TODO: This library needs to be architected well to have an
# interface which needs to be implemented by it's sub-classes


class databus:
    '''Creates an instance of databus'''

    def __init__(self, log):
        self.logger = log
        self.busType = None
        self.direction = "NONE"
        self.pubTopics = {}
        self.subTopics = {}
        self.bus = None
        self.mutex = Lock()

    def ContextCreate(self, contextConfig):
        '''Create an underlying messagebus context for the databus
        Arguments:
            contextConfig<dict>: Messagebus params to create the context
                <fields>
                "direction": PUB/SUB/NONE - Mutually exclusive
                "name": context namespace (PUB/SUB context namespaces should
                        match)
                "endpoint": messagebus endpoint address
                    <format> proto://host:port/, proto://host:port/.../
                    <examples>
                    OPCUA -> opcua://0.0.0.0:4840/
                "certFile"   : server/client certificate file
                "privateFile": server/client private key file
                "trustFile"  : ca cert used to sign server/client cert
        Return/Exception: Will raise Exception in case of errors'''

        try:
            self.mutex.acquire()
            endpoint = contextConfig["endpoint"]
            # TODO: Check for unique pub/sub contextName?
            if endpoint.split('//')[0] == busTypes["OPCUA"]:
                self.busType = busTypes["OPCUA"]
                self.bus = databOpcua(self.logger)
                self.logger.info("DataBus type: {}".format(busTypes["OPCUA"]))
            else:
                raise Exception("Not a supported BusType")
            self.logger.info(contextConfig)
            try:
                self.bus.createContext(contextConfig)
            except Exception:
                self.logger.error("{} Failure!!!".format(
                    self.ContextCreate.__name__))
                raise
            if contextConfig["direction"] == "PUB":
                self.direction = "PUB"
            elif contextConfig["direction"] == "SUB":
                self.direction = "SUB"
            else:
                raise Exception("Not a supported BusDirection")
        finally:
            self.mutex.release()

    def Publish(self, topicConfig, data):
        '''Publish data on the databus
        Arguments:
            topicConfig<dict>: Publish topic parameters
                <fields>
                "name": Topic name (in hierarchical form with '/' as delimiter)
                    <example> "root/level1/level2/level3"
                "type": Data type associated with the topic
            data: The message whose type should match the topic data type
        Return/Exception: Will raise Exception in case of errors'''

        if "opcua" in self.busType:
            try:
                self.bus.send(topicConfig, data)
            except Exception:
                self.logger.error("{} Failure!!!".format(
                                 self.Publish.__name__))
                raise

    def Subscribe(self, topicConfigs, topicConfigCount, trig, cb=None):
        '''Subscribe data from the databus
        Arguments:
            topicConfigs<list of dicts>: Subscribe topic parameters
                <dict_fields>
                "name": Topic name
                "dType": Data type associated with the topic
            topicConfigCount: length of topicConfigs list
            trig: START/STOP- Start OR Stop Subscription
            cb: A callback function with data as argument
        Return/Exception: Will raise Exception in case of errors'''
        print("self.busType: ", self.busType)
        if "opcua" in self.busType:
            try:
                if (self.direction == "SUB") and (trig == "START") and \
                   (cb is not None):
                    qu = Queue()
                    ev = Event()
                    ev.clear()
                    th = Thread(target=worker,
                                args=(qu, ev, cb, self.logger))
                    th.deamon = True
                    th.start()
                    self.bus.receive(topicConfigs, topicConfigCount,
                                     "START", qu)
            except Exception:
                self.logger.error("receive {} Failure!!!".format(
                                 self.Subscribe.__name__))
                raise

    def ContextDestroy(self):
        '''Destroys the underlying messagebus context
        It unsubscribe all the existing subscriptions too'''

        try:
            if "opcua" in self.busType:
                self.mutex.acquire()
                try:
                    self.bus.destroyContext()
                    self.bus.direction = " "
                except Exception:
                    self.logger.error("{} Failure!!!".format(
                        self.ContextDestroy.__name__))
                    raise
                finally:
                    self.mutex.release()
        except Exception:
            self.logger.error("{} Failure!!!".format(
                self.ContextDestroy.__name__))
            raise

    def checkMsgType(self, topicType, msgData):
        '''Pvt function'''

        if type(msgData) == str:
            if topicType == "string":
                return True
            else:
                return False
