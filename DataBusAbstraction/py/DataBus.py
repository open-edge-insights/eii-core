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

from DataBusMqtt import databMqtt
from DataBusOpcua import databOpcua
from DataBusNats import databNats
from Queue import Queue
from threading import Thread, Lock
from threading import Event
import logging

busTypes = {"OPCUA": "opcua:", "MQTT": "mqtt:", "NATS": "nats:"}

logging.basicConfig(level=logging.DEBUG,
                    format='%(asctime)s : %(levelname)s : \
                    %(name)s : [%(filename)s] :' +
                    '%(funcName)s : in line : [%(lineno)d] : %(message)s')
logging.getLogger("opcua").setLevel(logging.WARNING)
logger = logging.getLogger(__name__)


def worker(topic, qu, ev, fn):
    '''Worker function to fetch data from queue and trigger cb'''

    while True:
        if ev.is_set():
            logger.info("Worker Done")
            break
        try:
            fn(topic, qu.get(timeout=2))
            qu.task_done()
        except Exception:
            # TODO: pass only for Empty exception
            # logger.error("Exception passed")
            pass
    return


class databus:
    '''Creates an instance of databus'''

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
                    OPCUA -> opcua://0.0.0.0:65003/elephanttrunk/
                    MQTT -> mqtt://localhost:1883/
                    NATS -> nats://127.0.0.1:4222/
        Return/Exception: Will raise Exception in case of errors'''

        try:
            self.mutex.acquire()
            endpoint = contextConfig["endpoint"]
            # TODO: Check for unique pub/sub contextName?
            if endpoint.split('//')[0] == busTypes["OPCUA"]:
                self.busType = busTypes["OPCUA"]
                self.bus = databOpcua()
                logger.info("DataBus type: {}".format(busTypes["OPCUA"]))
            elif endpoint.split('//')[0] == busTypes["MQTT"]:
                self.busType = busTypes["MQTT"]
                self.bus = databMqtt()
                logger.info("DataBus type: {}".format(busTypes["MQTT"]))
                logger.info(busTypes["MQTT"])
            elif endpoint.split('//')[0] == busTypes["NATS"]:
                self.busType = busTypes["NATS"]
                self.bus = databNats()
                logger.info("DataBus type: {}".format(busTypes["NATS"]))
            else:
                raise Exception("Not a supported BusType")
            logger.info(contextConfig)
            try:
                self.bus.createContext(contextConfig)
            except Exception as e:
                logger.error("{} Failure!!!".format(
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

        try:
            self.mutex.acquire()
            if self.direction == "PUB":
                try:
                    if not self.checkMsgType(topicConfig["type"], data):
                        raise Exception("Not a supported Message Type")
                    if topicConfig["name"] not in self.pubTopics.keys():
                        self.bus.startTopic(topicConfig)
                        self.pubTopics[topicConfig["name"]] = {}
                        self.pubTopics[topicConfig["name"]]["type"] = \
                            topicConfig["type"]
                    # Now pubTopics has the topic for sure.
                    # (either created now/was present. Now check for type
                    pubTopicType = self.pubTopics[topicConfig["name"]]["type"]
                    if pubTopicType != topicConfig["type"]:
                        raise Exception("Topic name & Type not matching")
                    # We are good to publish now
                    self.bus.send(topicConfig["name"], data)
                except Exception as e:
                    logger.error("{} Failure!!!".format(self.Publish.__name__))
                    raise
            else:
                raise Exception("Not a supported BusDirection")
        finally:
            self.mutex.release()

    def Subscribe(self, topicConfig, trig, cb=None):
        '''Subscribe data from the databus
        Arguments:
            topicConfig<dict>: Subscribe topic parameters
                <fields>
                "name": Topic name (in hierarchical form with '/' as delimiter)
                    <example> "root/level1/level2/level3"
                "type": Data type associated with the topic
            trig: START/STOP- Start OR Stop Subscription
            cb: A callback function with data as argument
        Return/Exception: Will raise Exception in case of errors'''

        try:
            self.mutex.acquire()
            if self.direction == "SUB":
                try:
                    if topicConfig["name"] not in self.subTopics.keys():
                        self.bus.startTopic(topicConfig)
                        self.subTopics[topicConfig["name"]] = {}
                        self.subTopics[topicConfig["name"]]["type"] = \
                            topicConfig["type"]
                    subTopicType = self.subTopics[topicConfig["name"]]["type"]
                    if subTopicType != topicConfig["type"]:
                        raise Exception("Topic name & Type not matching")
                    # We are good to receive now
                    if (trig == "START") and (cb is not None):
                        subTopics = self.subTopics[topicConfig["name"]]
                        if "thread" in subTopics.keys():
                            raise Exception("Already Subscribed!!!")
                        qu = Queue()
                        ev = Event()
                        ev.clear()
                        th = Thread(target=worker, args=(topicConfig["name"],
                                                         qu, ev, cb))
                        th.deamon = True
                        th.start()
                        self.subTopics[topicConfig["name"]]["queue"] = qu
                        self.subTopics[topicConfig["name"]]["event"] = ev
                        self.subTopics[topicConfig["name"]]["thread"] = th
                        self.bus.receive(topicConfig["name"], "START", qu)
                    elif trig == "STOP":
                        # This should stop putting data into queue
                        self.bus.receive(topicConfig["name"], "STOP")
                        # Block until all data is read
                        self.subTopics[topicConfig["name"]]["queue"].join()
                        self.subTopics[topicConfig["name"]]["event"].set()
                        self.subTopics[topicConfig["name"]]["thread"].join()
                        # Remove entry from subTopics
                        self.subTopics.pop(topicConfig["name"])
                    else:
                        raise Exception("Unknown Trigger!!!")
                except Exception as e:
                    logger.error("{} Failure!!!".format(
                        self.Subscribe.__name__))
                    raise
            else:
                raise Exception("Not a supported BusDirection")
        finally:
            self.mutex.release()

    def ContextDestroy(self):
        '''Destroys the underlying messagebus context
        It unsubscribe all the existing subscriptions too'''

        try:
            self.mutex.acquire()
            try:
                # self.bus.stopTopic()
                for key, item in self.subTopics.items():
                    self.bus.receive(key, "STOP")
                    self.subTopics[key]["queue"].join()
                    self.subTopics[key]["event"].set()
                    self.subTopics[key]["thread"].join()
                    self.subTopics.pop(key)
                self.bus.destroyContext()
            except Exception as e:
                logger.error("{} Failure!!!".format(
                    self.ContextDestroy.__name__))
                raise
        finally:
            self.mutex.release()

    def checkMsgType(self, topicType, msgData):
        '''Pvt function'''

        if type(msgData) == str:
            if topicType == "string":
                return True
            else:
                return False

    def __init__(self):
        self.busType = None
        self.direction = "NONE"
        self.pubTopics = {}
        self.subTopics = {}
        self.bus = None
        self.mutex = Lock()
