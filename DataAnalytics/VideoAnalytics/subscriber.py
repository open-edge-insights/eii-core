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
            subscriber's output queue (has [topic, metadata, 
            keyframe] data entries)
        """
        self.log = logging.getLogger(__name__)
        self.subscriber_queue = subscriber_queue
        self.stop_ev = threading.Event()

    def start(self):
        """
        Starts the subscriber thread
        """
        context = zmq.Context()
        self.socket = context.socket(zmq.SUB)
        topics = os.environ['SubTopics'].split(",")
        self.subscriber_threadpool = ThreadPoolExecutor(max_workers=len(topics))
        for topic in topics:
            topic_cfg = os.environ["{}_cfg".format(topic)].split(",")
            self.subscriber_threadpool.submit(self.subscribe, topic,
                                              topic_cfg)

    def subscribe(self, topic, topic_cfg):
        """
        Receives the data for the subscribed topic
        Parameters:
        ----------
        topic: str
            topic name
        topic_cfg: str
            topic config
        """
        thread_id = threading.get_ident()
        self.log.info("Subscriber thread ID: {} started...".format(thread_id))

        mode = topic_cfg[0].lower()
        try:
            if "tcp" in mode:
                self.socket.connect("tcp://{}".format(topic_cfg[1]))
            elif "ipc" in mode:
                self.socket.connect("ipc://{}".format(topic_cfg[1]))
        except Exception as ex:
            self.log.exception(ex)

        self.log.info("Subscribing to topic:{}...".format(topic))
        self.socket.setsockopt_string(zmq.SUBSCRIBE, topic)
        while not self.stop_ev.is_set():
            data = self.socket.recv_multipart()
            topic = data[0].decode()
            metadata = json.loads(data[1].decode())
            frame = data[2]

            # Convert the buffer into np array.
            Frame = np.frombuffer(frame, dtype=np.uint8)
            encoding = metadata["encoding"]
            if encoding is not None:
                reshape_frame = np.reshape(Frame, (Frame.shape))
                reshape_frame = cv2.imdecode(reshape_frame, 1)
            else:
                reshape_frame = np.reshape(Frame, (int(metadata["height"]),
                                                int(metadata["width"]),
                                                int(metadata["channel"])))

            data = [topic, metadata, reshape_frame]
            self.log.debug("Added data to subscriber queue: {}".format(data[:2]))
            self.subscriber_queue.put(data)
        self.log.info("Subscriber thread ID: {} stopped...".format(thread_id))   

    def stop(self):
        """
        Stops the Subscriber thread
        """
        self.stop_ev.set()
        self.socket.close()
        self.subscriber_threadpool.shutdown()
