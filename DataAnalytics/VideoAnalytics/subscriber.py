# Copyright (c) 2019 Intel Corporation.

# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included in
#  all copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

import threading
import json
import logging
import os
import numpy as np
import cv2
from concurrent.futures import ThreadPoolExecutor
from libs.common.py.util import get_topics_from_env,\
                                get_messagebus_config_from_env
import eis.msgbus as mb


class Subscriber:

    def __init__(self, subscriber_queue):
        """Constructor

        Parameters
        -----------
        subscriber_queue: Queue
            subscriber's output queue (has (metadata,frame) tuple data entries)
        """
        self.log = logging.getLogger(__name__)
        self.subscriber_queue = subscriber_queue
        self.stop_ev = threading.Event()

    def start(self):
        """
        Starts the subscriber thread
        """
        topics = get_topics_from_env("sub")
        self.subscriber_threadpool = \
            ThreadPoolExecutor(max_workers=len(topics))
        for topic in topics:
            topic_cfg = get_messagebus_config_from_env(topic, "sub")
            self.subscriber_threadpool.submit(self.subscribe, topic,
                                              topic_cfg)

    def subscribe(self, topic, config):
        """
        Receives the data for the subscribed topic
        Parameters:
        ----------
        topic: str
            topic name
        config: str
            topic config
        """
        # TODO: Need to have same msgbus context when multiple topics are
        # subscribing to same endpoint
        msgbus = mb.MsgbusContext(config)
        subscriber = msgbus.new_subscriber(topic)
        thread_id = threading.get_ident()
        log_msg = "Thread ID: {} {} with topic:{} and topic_cfg:{}"
        self.log.info(log_msg.format(thread_id, "started", topic, config))
        self.log.info("Subscribing to topic: {}...".format(topic))

        try:
            while not self.stop_ev.is_set():
                data = subscriber.recv()
                self.subscriber_queue.put(data)
                self.log.debug("Subscribed data: {} on topic: {} with config: {}\
                               ...".format(data[0], topic, config))
        except Exception as ex:
            self.log.exception('Error while subscribing data:\
                            {}'.format(ex))
        finally:
            subscriber.close()

        self.log.info(log_msg.format(thread_id, "stopped", topic, config))

    def stop(self):
        """
        Stops the Subscriber thread
        """
        try:
            self.stop_ev.set()
            self.subscriber_threadpool.shutdown(wait=False)
        except Exception as ex:
            self.log.exception(ex)
