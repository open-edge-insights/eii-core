# Copyright (c) 2019 Intel Corporation.

# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

import zmq
import threading
import json
import logging
import os
import numpy as np
import cv2
from concurrent.futures import ThreadPoolExecutor

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

    def start(self):
        """
        Starts the subscriber thread
        """
        self.context = zmq.Context()
        socket = self.context.socket(zmq.SUB)
        topics = os.environ['SubTopics'].split(",")
        self.sockets = []
        self.subscriber_threadpool = ThreadPoolExecutor(max_workers=len(topics))
        for topic in topics:
            topic_cfg = os.environ["{}_cfg".format(topic)].split(",")
            self.subscriber_threadpool.submit(self.subscribe, socket, topic,
                                              topic_cfg)

    def subscribe(self, socket, topic, topic_cfg):
        """
        Receives the data for the subscribed topic
        Parameters:
        ----------
        socket: ZMQ socket
            socket instance
        topic: str
            topic name
        topic_cfg: str
            topic config
        """
        thread_id = threading.get_ident()
        log_msg = "Thread ID: {} {} with topic:{} and topic_cfg:{}"
        self.log.info(log_msg.format(thread_id, "started", topic, topic_cfg))

        mode = topic_cfg[0].lower()
        try:
            if "tcp" in mode:
                socket.connect("tcp://{}".format(topic_cfg[1]))
            elif "ipc" in mode:
                socket.connect("ipc://{}".format(topic_cfg[1]))
            self.sockets.append(socket)
        except Exception as ex:
            self.log.exception(ex)

        self.log.info("Subscribing to topic: {}...".format(topic))
        socket.setsockopt_string(zmq.SUBSCRIBE, topic)

        while True:
            msg = socket.recv_multipart()
            
            topic = msg[0].decode()
            metadata = json.loads(msg[1].decode())
            frame = msg[2]

            self.subscriber_queue.put((metadata, frame))
            self.log.debug("Added metadata:{} to subscriber queue".format(
                metadata))
        
        self.log.info(log_msg.format(thread_id, "stopped", topic, topic_cfg))

    def stop(self):
        """
        Stops the Subscriber thread
        """
        try:
            self.subscriber_threadpool.shutdown(wait=False)
            for socket in self.sockets:
                socket.close()
                if socket._closed == "False":
                    self.log.error("Unable to close socket connection")
            self.context.term()        
        except Exception as ex:
            self.log.exception(ex)
