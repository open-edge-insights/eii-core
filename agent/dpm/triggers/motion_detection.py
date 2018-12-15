# Copyright (c) 2018 Intel Corporation.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included
# in all
# copies or substantial portions of the Software. Software to be used for
# Made in China 2025 initiatives.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
# Visual trigger for the Yu Mei factory.
import logging
import cv2
import numpy as np
from time import time
from . import BaseTrigger


class Trigger(BaseTrigger):
    """Yu Mei factory trigger object.
    """

    def __init__(self, angle_sequence, white_threshold,
                 light_threshold, roi_y1, roi_y2):
        """Constructor.
        """
        super(Trigger, self).__init__()
        self.log = logging.getLogger(__name__)
        # MOG background subtractor to detect trigger start
        self.fgbg = cv2.createBackgroundSubtractorMOG2()
        self.roi_y1 = roi_y1  # ROI for robot arm TODO: Remove?
        self.roi_y2 = roi_y2  # ROI for robot arm TODO: Remove?
        self.frame_count = 0  # Frame count once trigger starts
        # TODO: Remove. This variable is an artifact from a previous logic.
        # Removing this variable required a modified config file.
        # Keeping the variable for now to reduce
        # confusion between US,India,China teams.
        self.light_threshold = light_threshold
        # Threshold for # of white pixels in robot roi
        self.white_threshold = white_threshold
        # Frame count values that correspond to A1, A2, A3, A4
        self.angle_sequence = angle_sequence
        self.angle_order = [2, 1, 3, 4]
        # Last angle extracted
        self.current_angle = 0
        # Flag to lock/unlock visual trigger
        self.trigger_lock = False
        # Timestamp of visual trigger lock start
        self.lock_start_time = 0
        # First frame under consideration for trigger
        self.first_frame = False
        # ETR reboot flag
        self.etr_first_start = True

    # Calculate the # of black and white pixels in a MOG subtracted image
    # to decide if visual trigger start
    def _trigger_start(self, frame):
        fgmask = self.fgbg.apply(frame)
        h, w = fgmask.shape
        # ROI over robot arm (not part)
        robot_crop = fgmask[self.roi_y1:self.roi_y2, :]
        # only count white pixels
        n_robot_white = np.sum(robot_crop == 255)
        # only count black pixels
        n_black = np.sum(robot_crop == 0)
        # ROI over center region to detect part on frame
        roi_crop = fgmask[650:750, 860:960]
        # only count white pixels
        n_roi_white = np.sum(roi_crop == 255)
        return n_robot_white, n_roi_white, n_black

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

        # ETR booting up. Wait till light fluctuations die down
        if self.etr_first_start is True:
            # check to see if frame is
            # all black => no motion/light variations, blank background
            _, _, n_black = self._trigger_start(data[1])
            if n_black > 1400000:
                self.etr_first_start = False
        else:
            # If visual trigger is False, check if trigger is
            # locked (locked after A4 is captured till next run starts)
            if self.is_triggered() is False:
                if (self.trigger_lock is False):
                    num_robot_white_px, num_roi_white_px, _ = \
                            self._trigger_start(data[1])
                    # Logic to start trigger.
                    # If first frame is not False (not initial condition)
                    # and robot arm in view on robot_crop roi
                    # (=> number of white pixels is above thresh)
                    # and MOG motion detection output is
                    # only concentrated on the left side of
                    # frame => center roi is black
                    # send trigger start signal. First
                    # trigger frame (frame with a sliver of
                    # robot arm visible on left side)
                    # is sent to classification algo for debug purposes.
                    if ((self.first_frame is not False) &
                        (
                         (num_robot_white_px >=
                          self.white_threshold) &
                         (num_roi_white_px <= 20))):
                        self.send_start_signal(data, -1)
                        self.log.debug("Sending start signal from trigger")
                    # If first frame is true (initial condition),
                    # change flag. First frame output from MOG subtractor is
                    # not used for trigger logic. (Gray image)
                    elif (self.first_frame is False):
                        self.first_frame = True
                else:
                    # If trigger is locked, all angles of current run
                    # already extracted. Lockdown enabled so as not
                    # to trigger on any frames after A4, before next
                    # run starts. The 2 events are expected to be atleast
                    # 30 seconds apart. If incoming frame timestamp is
                    # atlest 30 sec in future from trigger lock time,
                    # change trigger_lock (unlock) and reinitialize
                    # first frame to inital condition.
                    current_frame_time = int(time())
                    if current_frame_time - self.lock_start_time > 30:
                            self.first_frame = False
                            self.trigger_lock = False

            else:
                # If is_triggered() is True
                # (=> visual trigger start signal has been sent)
                # start counting frames
                # to extract A1 - A4 angles. Logic is based on frame counting.
                self.frame_count = self.frame_count + 1
                if self.frame_count in self.angle_sequence:
                    # If frame count in angle_sequence, send frame
                    # for classification and increment current_angle
                    self.send_data(data, self.angle_order[self.current_angle])
                    self.current_angle = self.current_angle + 1
                    if self.current_angle == 4:
                        # If A4 extracted, send visual trigger stop signal,
                        # lock trigger until next run (which we expect
                        # to be atleast 30 seconds in the future).
                        # Save lock start time to enable comparison to incoming
                        # frame timestamp. Revert all variables
                        # to initial conditions
                        self.send_stop_signal()
                        self.trigger_lock = True
                        self.lock_start_time = int(time())
                        self.frame_count = 0
                        self.current_angle = 0
                        self.log.debug("Sending stop signal from trigger")
