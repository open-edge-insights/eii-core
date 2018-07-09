"""Visual trigger for the Yu Mei factory.
"""
import logging
import cv2
import numpy as np
from time import time
from . import BaseTrigger, TriggerIter

class Trigger(BaseTrigger):
    """Yu Mei factory trigger object.
    """

    def __init__(self, angle_sequence, white_threshold, light_threshold, roi_y1, roi_y2):
        """Constructor.
        """
        super(Trigger, self).__init__()
        self.log = logging.getLogger(__name__)
        self.fgbg = cv2.createBackgroundSubtractorMOG2()
        self.roi_y1 = roi_y1
        self.roi_y2 = roi_y2
        self.frame_count = 0
        self.light_threshold = light_threshold
        self.white_threshold = white_threshold
        self.angle_sequence = angle_sequence
        self.angle_order = [2, 1, 3, 4] # :)
        self.current_angle = 0
        self.trigger_lock = False
        self.lock_start_time = 0
        self.first_frame = False
        self.etr_first_start = True

    def _trigger_start(self, frame):
        fgmask = self.fgbg.apply(frame)
        h,w = fgmask.shape
        robot_crop = fgmask[self.roi_y1:self.roi_y2,:] #ROI over robot arm (not part)
        n_robot_white = np.sum(robot_crop==255) #only count white pixels
        n_black = np.sum(robot_crop==0) #only count white pixels
        roi_crop = fgmask[650:750, 860:960]
        n_roi_white = np.sum(roi_crop==255)
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

        # this needs to be refactored ....
        if self.etr_first_start == True:
            #check to see if frame is all black
            _, _, n_black = self._trigger_start(data[1])
            if n_black > 1400000:
               self.etr_first_start = False
        else:
            if self.is_triggered() == False:
               if (self.trigger_lock == False) :
                  num_robot_white_px, num_roi_white_px, _ = self._trigger_start(data[1])
                  if  (self.first_frame != False) & (num_robot_white_px >= self.white_threshold) & (num_roi_white_px <= 20):
                        self.send_start_signal(data, -1)
                        self.log.debug("Sending start signal from trigger")
                        self.log.debug("Num_robot_white_px: {} num_roi_white_px: {}".format(num_robot_white_px, num_roi_white_px))
                  elif (self.first_frame is False):
                        self.first_frame = True

               else:
                  current_frame_time = int(time())
                  if current_frame_time - self.lock_start_time > 30 :
                        self.first_frame = False
                        self.trigger_lock = False #unlock

            else:
               self.frame_count = self.frame_count + 1
               if self.frame_count in self.angle_sequence:
                  self.send_data(data, self.angle_order[self.current_angle])
                  self.current_angle = self.current_angle + 1
                  #if self.current_angle == 4 :
                  if self.frame_count == 1091 :
                        self.send_stop_signal()
                        self.trigger_lock = True
                        self.lock_start_time = int(time())
                        self.frame_count = 0
                        self.current_angle = 0
                        self.log.debug("Sending stop signal from trigger")
