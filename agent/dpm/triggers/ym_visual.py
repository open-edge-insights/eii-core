"""
Copyright (c) 2018 Intel Corporation.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software. Software to be used for 
Made in China 2025 initiatives.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
"""

"""Visual trigger for the Yu Mei factory.
"""

import logging
import cv2
import numpy as np
from . import BaseTrigger, TriggerIter

class Trigger(BaseTrigger):
    """Yu Mei factory trigger object.
    """

    def __init__(self, kp_threshold, no_part_in_frame_thresh):
        """Constructor.
        """
        super(Trigger, self).__init__()
        self.log = logging.getLogger(__name__)
        self.kp_threshold = kp_threshold
        self.no_part_in_frame_thresh = no_part_in_frame_thresh
        self.last_in_frame_count = 0
        self.brisk = cv2.BRISK_create()

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
        kp, des = self.brisk.detectAndCompute(data[1], None)
        self.log.debug('Keypoints: %d', len(kp))
        # If not triggered and kp Is greater than the thresh send_start_signal
        if(len(kp) >= self.kp_threshold and self.is_triggered() == False):
            self.log.debug('is_triggered %d', self.is_triggered())
            self.log.info('Sending start signal')
            self.send_start_signal(data)
            #self.log.debug('Resetting last_in_frame_count after sending start signal')
            self.last_in_frame_count = 0
        elif(len(kp) >= self.kp_threshold and self.is_triggered()):
            self.log.debug('Sending data from trigger')
            self.send_data(data)
            #self.log.debug('Resetting last_in_frame_count after sending data')
            self.last_in_frame_count = 0 
        else:
            self.log.debug('No part seen')
            self.last_in_frame_count += 1

        #if part has not been in X frames send stop signal
        if(self.last_in_frame_count >= self.no_part_in_frame_thresh and self.is_triggered()):
            self.log.info('sending stop signal')
            self.send_stop_signal()
