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
import logging
import os
import json

class Publisher:

    def __init__(self, classifier_output_queue):
        """Constructor

        Parameters
        -----------
        classifier_output_queue : Queue
            Input queue for publisher (has [topic, classifier_metadata, 
            keyframe] data entries)
        """
        self.log = logging.getLogger(__name__)
        self.classifier_output_queue = classifier_output_queue
        self.stop_ev = threading.Event()

    def start(self):
        """
        Starts the publisher thread
        """
        self.thread = threading.Thread(target=self.publish)
        self.thread.setDaemon(True)
        self.thread.start()

    def publish(self):
        """
        Send the data to the publish topic
        """
        self.log.info("=======Starting publisher thread=======")
        context = zmq.Context()
        self.socket = context.socket(zmq.PUB)
        topics = os.environ['PubTopics'].split(",")

        # Keeping the logic of being able to publish to multiple topics
        # with each publish happening on different/same bind socket
        # address as per the ENV configuration
        for topic in topics:
            topic_cfg = os.environ["{}_cfg".format(topic)].split(",")
            mode = topic_cfg[0].lower()
            if "tcp" in mode:
                self.socket.bind("tcp://{}".format(topic_cfg[1]))
            elif "ipc" in mode:
                self.socket.bind("ipc://{}".format(topic_cfg[1]))
                
        while not self.stop_ev.is_set():
            result = self.classifier_output_queue.get()
            topic = result[0].encode()
            metadata = json.dumps(result[1]).encode()
            self.socket.send_multipart([topic, metadata, result[2]], copy=False)
            self.log.debug("Published data...")
        self.log.info("=====Stopped publisher thread======")                     

    def stop(self):
        """
        Stops the Subscriber thread
        """
        self.socket.close()
        self.stop_ev.set()

    def join(self):
        """
        Blocks until the publisher thread stops running
        """
        self.thread.join()