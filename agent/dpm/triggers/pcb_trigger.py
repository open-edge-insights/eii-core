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

    def __init__(self, training_mode, n_total_px, n_left_px, n_right_px):
        """Constructor.
        """
        super(Trigger, self).__init__()
        self.log = logging.getLogger(__name__)
        # Initialize background subtractor
        self.fgbg = cv2.createBackgroundSubtractorMOG2()
        # Total white pixel # on MOG applied
        # frame after morphological operations
        self.n_total_px = n_total_px
        # Total white pixel # on left edge of MOG
        # applied frame after morphological operations
        self.n_left_px = n_left_px
        # Total white pixel # on right edge of MOG
        # applied frame after morphological operations
        self.n_right_px = n_right_px
        # Flag to lock trigger from forwarding frames to classifier
        self.trigger_lock = False
        # count frames when trigger is locked
        self.lock_frame_count = 0
        self.training_mode = training_mode
        self.count = 0

    def get_supported_ingestors(self):
        return ['video', 'video_file']

    def _check_frame(self, frame):
        # Apply background subtractor on frame
        fgmask = self.fgbg.apply(frame)
        rows, columns = fgmask.shape
        if self.trigger_lock is False:
            # Applying morphological operations
            kernel = cv2.getStructuringElement(cv2.MORPH_RECT, (20, 20))
            ret, thresh = cv2.threshold(fgmask, 0, 255,
                                        cv2.THRESH_BINARY+cv2.THRESH_OTSU)
            thresh = cv2.morphologyEx(thresh, cv2.MORPH_CLOSE, kernel)

            # Crop left and right edges of frame
            left = thresh[:, 0:10]
            right = thresh[:, (columns - 10):(columns)]

            # Count the # of white pixels in thresh
            n_total = np.sum(thresh == 255)
            n_left = np.sum(left == 255)
            n_right = np.sum(right == 255)
            # If the PCB is in view of camera & is not
            # touching the left, right edge of frame
            if (n_total > self.n_total_px) & \
                (n_left < self.n_left_px) & \
                    (n_right < self.n_right_px):
                # Find the PCB contour
                im, contours, hier = cv2.findContours(thresh.copy(),
                                                      cv2.RETR_EXTERNAL,
                                                      cv2.CHAIN_APPROX_NONE)
                if len(contours) != 0:
                    # Contour with largest area would be bounding the PCB
                    c = max(contours, key=cv2.contourArea)

                    # Obtain the bounding rectangle
                    # for the contour and calculate the center
                    x, y, w, h = cv2.boundingRect(c)
                    cX = int(x + (w / 2))

                    # If the rectangle bounding the
                    # PCB doesn't touch the left or right edge
                    # of frame and the center x lies within
                    if (x != 0) & ((x + w) != columns) & \
                       ((columns/2 - 100) <= cX <= (columns/2 + 100)):
                        return True
                    else:
                        return False
        return False

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
        self.count = self.count + 1
        if self.training_mode is True:
                self.send_start_signal(data, -1)
                cv2.imwrite("./frames/"+str(self.count)+".png", data[1])
        else:
            if self.trigger_lock is False:
                if self._check_frame(data[1]) is True:
                    # Send trigger start signal and send frame to classifier
                    self.send_start_signal(data, -1)
                    self.log.info("Sending frame")
                    self.send_data(data, 1)
                    # Send trigger stop signal and lock trigger
                    self.send_stop_signal()
                    self.trigger_lock = True
                    # Re-initialize frame count during trigger lock to 0
                    self.lock_frame_count = 0
            else:
                # Continue applying background subtractor to
                # keep track of PCB positions
                self._check_frame(data[1])
                # Increment frame count during trigger lock phase
                self.lock_frame_count = self.lock_frame_count + 1
                if self.lock_frame_count == 7:
                    # Clear trigger lock after timeout
                    # period (measured in frame count here)
                    self.trigger_lock = False
