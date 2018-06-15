"""Visual trigger for the Yu Mei factory.
"""

import logging
import cv2
import numpy as np
from . import BaseTrigger, TriggerIter

class Trigger(BaseTrigger):
    """Yu Mei factory trigger object.
    """

    def __init__(self, ref_imgdir, ref_imgcount, kp_threshold, no_part_in_frame_thresh):
        """Constructor.
        """
        super(Trigger, self).__init__()
        self.log = logging.getLogger(__name__)
        self.kp_threshold = kp_threshold
        self.no_part_in_frame_thresh = no_part_in_frame_thresh
        self.last_in_frame_count = 0
        self.brisk = cv2.BRISK_create()
        self.ref_imgs = []
        self._data = []
        self.detector = cv2.BRISK_create() #cv2.ORB_create()
        self.extractor = cv2.BRISK_create()
        self.ref_kps = []
        self.ref_dess = []
        self.state = 0
        self.ANGLE_3_READY = False
        self.angle_score = [0,0,0,0]
        #self.historical_data = []
        self.matches_threshold_flag = False
        for count in range(1, ref_imgcount+1):
            img_path = ref_imgdir + '/A' + str(count) + '.png'
            self.ref_imgs.append(cv2.imread(img_path))
        for count in range(0, ref_imgcount):
            ref_gray =  cv2.cvtColor(self.ref_imgs[count], cv2.COLOR_BGR2GRAY)
            kp = self.detector.detect(ref_gray, None)
            kp, des = self.extractor.compute(ref_gray, kp)
            self.ref_kps.append(kp)
            self.ref_dess.append(des)

    def _calculate_angle(self, img):
        
        if self.state==1:
            scoreA1 = self._calculate_homography(img, 2)
            if (scoreA1 is not None) and( scoreA1 > self.angle_score[0]) or (len(self.angle_score) == 0):
                self.angle_score[0] = scoreA1
        elif self.state==2:
            scoreA2 = self._calculate_homography(img, 1)
            if (scoreA2 is not None) and (scoreA2 > self.angle_score[1]): 
                self.angle_score[1] = scoreA2
        elif self.state==3:
            scoreA3 = self._calculate_homography(img, 3)
            if (scoreA3 is not None): 
                if(scoreA3 < 30 and self.matches_threshold_flag == False):
                    #matches too low, skip frame
                    self.angle_score[2] = self.angle_score[2]
                elif(scoreA3 > 30):
                    self.matches_threshold_flag = True
                if(scoreA3 < self.angle_score[2]*0.1 and self.ANGLE_3_READY==False and self.matches_threshold_flag == True):
                    self.ANGLE_3_READY = True
                    self.log.info("Printing from angle 3, with angle_score of {}".format(self.angle_score[2]))
                    self.matches_threshold_flag = False
                elif (scoreA3 > self.angle_score[2] and self.ANGLE_3_READY == False and self.matches_threshold_flag == True): 
                    self.angle_score[2] = scoreA3
        elif self.state ==4:
            scoreA4 = self._calculate_homography(img, 4)
            if (scoreA4 is not None) and (scoreA4 > self.angle_score[3]): 
                self.angle_score[3] = scoreA4
        
    def _calculate_homography(self, img, index):
        ref_kp = self.ref_kps[index-1]
        ref_des = self.ref_dess[index-1]
        src_pts = []
        dst_pts = []
        matches = []
        img_gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
        img_kp = self.detector.detect(img_gray, None)
        img_kp, img_des = self.extractor.compute(img_gray, img_kp)
        #matcher = cv2.BFMatcher(cv2.NORM_HAMMING)
        #rawMatches = matcher.knnMatch(ref_des, img_des, k=2)
        FLANN_INDEX_LSH = 6
        index_params = dict(algorithm=FLANN_INDEX_LSH, table_number=6, key_size=12, multi_probe_level=1)
        search_params = dict(checks=50)
        flann = cv2.FlannBasedMatcher(index_params, search_params)
        rawMatches = flann.knnMatch(ref_des, img_des, k=2)
        for m in rawMatches:
            if len(m) == 2 and m[0].distance < m[1].distance *0.7:
                matches.append(m[0])
        if matches:
            try:
                src_pts = np.float32([ ref_kp[m.queryIdx].pt for m in matches ]).reshape(-1,1,2)
                dst_pts = np.float32([ img_kp[m.trainIdx].pt for m in matches ]).reshape(-1,1,2)
            except:
                print('[Debug] Not enough good matches skipping')
                return None
        #self.historical_data.append(len(matches))
        if len(src_pts) >0 and len(dst_pts) >0:
                M, mask = cv2.findHomography(src_pts, dst_pts, cv2.RANSAC, 4.0)
                score = float(mask.sum()) / mask.size
                self._data = [M, mask, score]
                self.log.debug('{}:{} = {}/{} {}'.format(self.state, index, len(matches), len(img_kp), score or 0))
                return len(matches)

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
            self.send_start_signal(data, user_data=[-1,0])
            self.state = 1
            self.last_in_frame_count = 0
        elif(len(kp) >= self.kp_threshold and self.is_triggered()):
            #self.log.info('Sending data from trigger')
            self._calculate_angle(data[1])
            if (self.angle_score[0] > 100) and (self.state ==1):
                self.state = 2
                usr_data = [2, self._data]
                self.send_data(data=data, user_data=usr_data)
            elif (self.angle_score[1] > 100) and (self.state ==2):
                usr_data = [1, self._data]
                self.state = 3
                self.send_data(data=data, user_data=usr_data)
            #elif (self.angle_score[2] > 100) and (self.state ==3):
            elif (self.ANGLE_3_READY == True) and (self.state ==3):
                usr_data = [3, self._data]
                self.send_data(data=data, user_data=usr_data)
                self.state = 4
            elif (self.angle_score[3] > 50) and (self.state ==4):
                usr_data = [4, self._data]
                self.send_data(data=data, user_data=usr_data)
                self.state = 0
            self.last_in_frame_count = 0 
            #a= np.array(self.historical_data, dtype=np.uint8)
            #np.savetxt("matches.csv",a,fmt='%1f', delimiter=",")
        else:
            self.log.debug('No part seen')
            self.last_in_frame_count += 1

        #if part has not been in X frames send stop signal
        if(self.last_in_frame_count >= self.no_part_in_frame_thresh and self.is_triggered()):
            self.log.info('sending stop signal')
            self.send_stop_signal()
            self.state = 0
