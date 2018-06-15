"""FLANN algorithm classifier
"""
import os
import logging
import cv2
import numpy as np
import json
from skimage.measure import compare_ssim
from agent.db.defect import Defect

# TODO: Change
from .algo_brokenpin import det_brokenpin
from .algo_missingriser import det_missingriser
from .algo_runnercrack import det_runnercrack
from .algo_hole609 import det_hole609

minMatches = 10

class Classifier:
    """FLANN Classifier object
    """
    def __init__(self, ref_imgdir, ref_imgcount, ref_config_roi):
        """Constructor

        Parameters
        ----------
        config : dict
            Classifier configuration
        """
        self.log = logging.getLogger('DEFECT_DETECTION')

        # Assert reference image directory exists
        assert os.path.exists(ref_imgdir), 'Reference image directory does not exist: {}'.format(ref_imgdir)

        self.ref_imgs = []
        # Assert all reference images exist {format A<angle#>.png}
        for count in range (1, ref_imgcount + 1) :
            img_path = ref_imgdir + '/A' + str(count) + '.png'
            assert os.path.exists(img_path), 'Reference image for Angle #' + str(count) + 'does not exist: {}'.format(img_path)
            self.ref_imgs.append(cv2.imread(img_path))

        # Assert reference image ROI file exists
        assert os.path.exists(ref_config_roi), 'Reference image ROI file does not exist: {}'.format(ref_config_roi)
        
        # Load ROI config file
        with open(ref_config_roi, 'r') as f:
            self.config_roi = json.load(f)
         

    def classify(self, frame_num, img, user_data):
        """Classify the given image against the specified reference image.

        Parameters
        ----------
        frame_num : int
            Frame count since the start signal was received from the trigger
        img : NumPy Array
            Image to classify

        Returns
        -------
            List of defects on the part in the frame, empty array if no defects
            exist on the part
        """
 
        
        index = user_data[0]
        if(index == -1):
            defects = []
            self.log.debug('index not in range')
            return defects

        # If frame# matches existing ref images continue with image classification algorithm
        self.log.info('Classifying image')

        # Read reference image to perform keypoint detection and overlay
        ref_img = self.ref_imgs[index-1].copy()
 
        
        img_orig = img.copy()
        mask = user_data[1][1]
        M = user_data[1][0]
        match_mask = mask.ravel().tolist()
        h_ref, w_ref, z_ref = ref_img.shape
        pts = np.float32([ 
            [0, 0],
            [0, h_ref - 1],
            [w_ref - 1, h_ref - 1],
            [w_ref - 1,0] 
            ]).reshape(-1, 1, 2)
        dst = cv2.perspectiveTransform(pts, M)

        # Draw keypoint matches between ref and img
        #cv2.polylines(img, [np.int32(dst)], True, 255, 3, cv2.LINE_AA)
        #keypoint_match = cv2.drawMatches(ref_img, ref_kp, img, img_kp, matches, None, flags = 2)

        # Warp ROI in img to overlay on ref
        # Determine top-left, top-right, bottom-left, bottom-right corners to warp ROI
        points = np.int32(dst).reshape(4,2)
        rect = np.zeros((4, 2), dtype = "float32")
        rect[0] = points[0]
        rect[1] = points[3]
        rect[2] = points[2]
        rect[3] = points[1]
        (tl, tr, br, bl) = rect
        
        # Note coordinate change due to overlay and warp
        warp_x = int(rect[0][0])
        warp_y = int(rect[0][1])

        maxWidth = w_ref
        maxHeight = h_ref

        # Top down view of ROI
        destination = np.array([
            [0, 0],
            [maxWidth - 1, 0],
            [maxWidth - 1, maxHeight - 1],
            [0, maxHeight - 1]], dtype = "float32")
        mat = cv2.getPerspectiveTransform(rect, destination)
        img_warp = cv2.warpPerspective(img_orig, mat, (maxWidth, maxHeight))

        src = ref_img.copy()
        overlay = img_warp.copy()

        defects = []

        # Get defect ROI and thresholds
        BP_roi = []
        RC_roi = []
        H609_roi = []
        R_roi = []
        if index == 1 :
            BP_roi = self.config_roi["brokenpin"]["A1_roi"]
            BP_thresh = self.config_roi["brokenpin"]["thresholds"]
        if index == 2 :
            RC_roi = self.config_roi["runnercrack"]["A2_roi"]
            RC_thresh = self.config_roi["runnercrack"]["thresholds"]
            R_roi = self.config_roi["missingriser"]["A2_roi"]
            R_thresh = self.config_roi["missingriser"]["thresholds"]
        if index == 3 :
            H609_roi = self.config_roi["hole609"]["A3_roi"]
            H609_thresh = self.config_roi["hole609"]["thresholds"]
            R_roi = self.config_roi["missingriser"]["A3_roi"]
            R_thresh = self.config_roi["missingriser"]["thresholds"]
        if index == 4 :
            R_roi = self.config_roi["missingriser"]["A4_roi"]
            R_thresh = self.config_roi["missingriser"]["thresholds"]
                
        defects = []
     
        if len(BP_roi) > 0 :
            thresh = BP_thresh
            bndbx = det_brokenpin(BP_roi,  src, overlay, thresh)
            for count in range (0, len(bndbx)) :
                [x, y, x1, y1] = bndbx[count]
                # Add in changes due to warping
                x  = x  + warp_x
                y  = y  + warp_y
                x1 = x1 + warp_x
                y1 = y1 + warp_y
                defects.append(Defect(0,(x, y), (x1, y1)))
        
        if len(R_roi) > 0 :
            thresh = R_thresh    
            bndbx = det_missingriser(R_roi, src, overlay, thresh, index)
            for count in range (0, len(bndbx)) :
                [x, y, x1, y1] = bndbx[count]
                # Add in changes due to warping
                x  = x  + warp_x
                y  = y  + warp_y
                x1 = x1 + warp_x
                y1 = y1 + warp_y
                defects.append(Defect(1,(x, y), (x1, y1)))
 
        if len(RC_roi) > 0 :
            thresh = RC_thresh
            bndbx = det_runnercrack(RC_roi, src, overlay, thresh)
            for count in range (0, len(bndbx)) :
                [x, y, x1, y1] = bndbx[count]
                # Add in changes due to warping
                x  = x  + warp_x
                y  = y  + warp_y
                x1 = x1 + warp_x
                y1 = y1 + warp_y
                defects.append(Defect(2,(x, y), (x1, y1)))

        if len(H609_roi) > 0 :
            thresh = H609_thresh
            bndbx = det_hole609(H609_roi, src, overlay, thresh)
            for count in range (0, len(bndbx)) :
                [x, y, x1, y1] = bndbx[count]
                # Add in changes due to warping
                x  = x  + warp_x
                y  = y  + warp_y
                x1 = x1 + warp_x
                y1 = y1 + warp_y
                defects.append(Defect(3,(x, y), (x1, y1)))

        return defects
