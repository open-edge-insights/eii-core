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

# type of databus
busTypes = {"OPCUA": "opcua:"}


def worker(qu, ev, fn, log):
    '''! Worker function to fetch data from queue and trigger cb
    @param qu(Queue)    queue to store/get data
    @param ev(Event)    event to notify the worker thread status
    @param fn(callback) that sends out the subscribed data back to the caller
    @param              log logger object
    '''

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

    def __init__(self, log):
        """! Creates an instance of databus"""

        self.logger = log
        self.busType = None
        self.direction = "NONE"
        self.pubTopics = {}
        self.subTopics = {}
        self.bus = None
        self.mutex = Lock()

    def ContextCreate(self, contextConfig):

        '''! ContextCreate function creates the opcua server/client (pub/sub)
             context based on `ContextConfig`.direction field
        @param contextConfig(dict)  ContextConfig for opcua pub/sub
                                    publisher (server) - If all certs/keys are
                                    set to empty string in ContextConfig,the
                                    opcua server starts in insecure mode.
                                    If not, it starts in secure mode.

                                    subscriber (client)- If all certs/keys are
                                    set to empty string in ContextConfig,the
                                    opcua client tries to establishes insecure
                                    connection. If not, it tries to establish
                                    secure connection with the opcua server

               contextConfig(dict) fields:
                 - "direction"       : PUB/SUB/NONE - Mutually exclusive
                 - "endpoint"        : messagebus endpoint address
                                       ex:OPCUA -> opcua://0.0.0.0:4840/
                 - "certFile"        : server/client certificate file
                 - "privateFile"     : server/client private key file
                 - "trustFile"       : ca cert used to sign server/client cert
                 - "trustedListSize" : number of trustFiles
         @return Exception: raise Exception in case of errors
        '''
        try:
            self.mutex.acquire()
            endpoint = contextConfig["endpoint"]
            # TODO: Check for unique pub/sub contextName?
            if endpoint.split('//')[0] == busTypes["OPCUA"]:
                self.busType = busTypes["OPCUA"]
                self.bus = databOpcua(self.logger)
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

        '''! Publish function for publishing the data by opcua server process
        @param  topicConfig(dict)   opcua `struct TopicConfig` structure

                topicConfig(dict) fields:
                  - "ns"  : Namespace name
                  - "name": Topic name
                  - "type": Data type associated with the topic
        @param  data(string)        data to be written to opcua variable
        @return Exception:  raise Exception in case of errors
        '''

        if "opcua" in self.busType:
            try:
                self.bus.send(topicConfig, data)
            except Exception:
                self.logger.error("{} Failure!!!".format(
                                 self.Publish.__name__))
                raise

    def Subscribe(self, topicConfig, topicConfigCount, trig, cb=None):

        '''! Subscribe function makes the subscription to the list of
             opcua variables (topics) in topicConfig array
        @param  topicConfig(array)    array of topicConfig instances

                topicConfig(dict) fields:
                  - "ns"  : Namespace name
                  - "name": Topic name
                  - "type": Data type associated with the topic
        @param  topicConfigCount(int) length of topicConfig array
        @param  trig(string)          opcua trigger ex: START | STOP
        @param  cb(c_callback)        callback that sends out the subscribed
                                      data back to the caller
        @return Exception:  raise Exception in case of errors
        '''

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
                    self.bus.receive(topicConfig, topicConfigCount,
                                     "START", qu)
            except Exception:
                self.logger.error("receive {} Failure!!!".format(
                                 self.Subscribe.__name__))
                raise

    def ContextDestroy(self):

        '''! ContextDestroy function destroys the opcua server/client
             context
        '''

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
