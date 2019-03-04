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

"""Visual trigger for PCB anomaly detection.
"""
import logging
import cv2
import numpy as np
from . import BaseTrigger


class Trigger(BaseTrigger):
    """PCB anomaly detection trigger object.
    """

    def __init__(self, training_mode):
        """Constructor.
        """
        super(Trigger, self).__init__()
        self.log = logging.getLogger(__name__)
        self.training_mode = training_mode
        self.count = 0

    def get_supported_ingestors(self):
        return ['video', 'video_file']

    def on_data(self, ingestor, data):
        """Process video frames as they are received and call the callback
        registered by the `register_trigger_callback()` method if the frame
        should trigger the execution of the classifier.

        Parameters
        ----------
        ingestor : str
            String name of the ingestor which received the data
        data : tuple
            Tuple of (camera serial number, camera frame)
        """
        if self.training_mode is True:
            self.send_start_signal(data, -1)
            cv2.imwrite("./frames/"+str(self.count)+".png", data[1])
        else:
            # Send trigger start signal and send frame to classifier
            self.send_start_signal(data, -1)
            self.log.info("Sending frame")
            self.send_data(data, 1)
            # Send trigger stop signal and lock trigger
            self.send_stop_signal()
