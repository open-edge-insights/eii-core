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

"""FLANN algorithm classifier
"""
import os
import logging
import cv2
import numpy as np
import json
from skimage.measure import compare_ssim
from agent.db.defect import Defect

# YuMei algorithm files
from .algo_brokenpin import det_brokenpin
from .algo_missingriser import det_missingriser
from .algo_runnercrack import det_runnercrack
from .algo_hole609 import det_hole609

minMatches = 10

class Classifier:
    """Classifier object
    """
    # Contrast limited adaptive histogram equalization
    # We are using this to equality distribute lighting across the image

    def __init__(self, ref_imgdir, ref_imgcount, ref_config_roi):
        """Constructor

        Parameters
        ----------
        ref_imgdir     : Reference image directory path
        ref_imgcount   : # of reference images in ref_imgdir
        ref_config_roi : Path to configuration and roi file for the reference images
            Classifier configuration
        """
        self.log = logging.getLogger('DEFECT_DETECTION')

        # Assert reference image directory exists
        assert os.path.exists(ref_imgdir), 'Reference image directory does not exist: {}'.format(ref_imgdir)

        self.ref_imgs = []
        self.brisk = cv2.BRISK_create()
        self.ref_kps = []
        self.ref_dess = []
        #Flann arguments
        self.FLANN_INDEX_LSH = 6
        self.index_params = dict(algorithm=self.FLANN_INDEX_LSH, table_number=6, key_size=12, multi_probe_level=1)
        self.search_params = dict(checks=50)
        self.flann = cv2.FlannBasedMatcher(self.index_params, self.search_params)

        # Assert all reference images exist {format A<angle#>.png}
        for count in range (1, ref_imgcount + 1) :
            img_path = ref_imgdir + '/A' + str(count) + '.png'
            assert os.path.exists(img_path), 'Reference image for Angle #' + str(count) + 'does not exist: {}'.format(img_path)
            self.ref_imgs.append(self._clahe(cv2.imread(img_path))) # Apply CLAHE to the reference images

        # Detect keypoints and extract features of all reference images and store in array for 
        # later comparison with production run images
        for count in range(0, ref_imgcount):
            ref_gray  = cv2.cvtColor(self.ref_imgs[count], cv2.COLOR_BGR2GRAY)
            kp = self.brisk.detect(ref_gray, None)
            kp, des = self.brisk.compute(ref_gray, kp)
            self.ref_kps.append(kp)
            self.ref_dess.append(des)

        # Assert reference image ROI file exists
        assert os.path.exists(ref_config_roi), 'Reference image ROI file does not exist: {}'.format(ref_config_roi)
        
        # Load ROI config file
        with open(ref_config_roi, 'r') as f:
            self.config_roi = json.load(f)

    # Convert input image to CLAHE'd image (Histogram equalization method to overcome lighting variations)
    def _clahe(self, frame):
        lab = cv2.cvtColor(frame, cv2.COLOR_BGR2LAB)
        l,a,b = cv2.split(lab)
        clahe = cv2.createCLAHE(clipLimit=3.0, tileGridSize=(8,8))
        cl = clahe.apply(l)
        limg = cv2.merge((cl, a, b))
        final = cv2.cvtColor(limg, cv2.COLOR_LAB2BGR)
        return final

    # Calculate homography score and # good matches between production frame and matching ref image
    def _calculate_homography(self, frame, index):
        ref_kp  = self.ref_kps[index-1]
        ref_des = self.ref_dess[index-1]
        src_pts = []
        dst_pts = []
        matches = []
        # Perform BRISK on incoming frame
        img_gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
        img_kp = self.brisk.detect(img_gray, None)
        img_kp, img_des = self.brisk.compute(img_gray, img_kp)
        raw_matches = self.flann.knnMatch(ref_des, img_des, k=2)
        for m in raw_matches:
            if len(m) == 2 and m[0].distance < m[1].distance * 0.7: #Lowe's Ratio Test
                matches.append(m[0])
        
        self.log.info("Number of good matches: {}".format(len(matches)))
        if matches:
            try:
                src_pts = np.float32([ ref_kp[m.queryIdx].pt for m in matches ]).reshape(-1,1,2)
                dst_pts = np.float32([ img_kp[m.trainIdx].pt for m in matches ]).reshape(-1,1,2)
            except:
                self.log.debug("Not enough good matches")
                return None
            if len(src_pts) > 0 and len(dst_pts) > 0:
                M, mask = cv2.findHomography(src_pts, dst_pts, cv2.RANSAC, 4.0)
                score = float(mask.sum()) / mask.size
                return (M, mask, score)

   
    # Main classification algorithm
    def classify(self, frame_num, img, user_data):
        """Classify the given image against the specified reference image.

        Parameters
        ----------
        frame_num : int
            Frame count since the start signal was received from the trigger
        img : NumPy Array
            Image to classify
        user_data : tuple
            Angle index of frame

        Returns
        -------
            List of defects on the part in the frame, empty array if no defects
            exist on the part
        """
        index = user_data   # Angle information of frame

        # Current trigger algorithm sends a trigger start frame that is used for debugging purposes
        # TODO: Remove once debugging complete
        if(index == -1):
            defects = []
            self.log.debug('index not in range')
            return defects

        # If frame # matches existing ref images continue with image classification algorithm
        self.log.info('Classifying image')

        # Read correct reference image using angle information of frame to perform keypoint detection and overlay
        ref_img = self.ref_imgs[index-1].copy()
        # Apply CLAHE on frame
        img_clahe = self._clahe(img)
        img_clahe_copy = img_clahe.copy()
        img_orig = img.copy()
        # Calculate homography info between frame and corresponding ref image
        M, mask, score = self._calculate_homography(frame=img_clahe, index=index)
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
        #cv2.polylines(img_clahe, [np.int32(dst)], True, 255, 3, cv2.LINE_AA)
        #keypoint_match = cv2.drawMatches(ref_img, ref_kp, img_clahe, img_kp, matches, None, flags = 2)
     
        # Warp ROI in frame to overlay on ref image
        # Determine top-left, top-right, bottom-left, bottom-right corners to warp ROI
        points = np.int32(dst).reshape(4,2)
        rect = np.zeros((4, 2), dtype = "float32")
        rect[0] = points[0]
        rect[1] = points[3]
        rect[2] = points[2]
        rect[3] = points[1]
        (tl, tr, br, bl) = rect
        
        maxWidth = w_ref
        maxHeight = h_ref

        # Top down view of ROI
        destination = np.array([
            [0, 0],
            [maxWidth - 1, 0],
            [maxWidth - 1, maxHeight - 1],
            [0, maxHeight - 1]], dtype = "float32")
        mat = cv2.getPerspectiveTransform(rect, destination)
        img_warp = cv2.warpPerspective(img_clahe_copy, mat, (maxWidth, maxHeight))

        src = ref_img.copy()
        overlay = img_warp.copy()

        # Get defect ROI and threshold values from config file
        BP_roi = []
        RC_roi = []
        H609_roi = []
        R_roi = []
        if index == 1 :
            # Defects in Angle #1 : Broken Pin
            BP_roi = self.config_roi["brokenpin"]["A1_roi"]
            BP_thresh = self.config_roi["brokenpin"]["thresholds"]
        if index == 2 :
            # Defects in Angle #2 : Runner crack, Missing riser
            RC_roi = self.config_roi["runnercrack"]["A2_roi"]
            RC_thresh = self.config_roi["runnercrack"]["thresholds"]
            R_roi = self.config_roi["missingriser"]["A2_roi"]
            R_thresh = self.config_roi["missingriser"]["thresholds"]
        if index == 3 :
            # Defects in Angle #3 : Hole 609 block, Missing riser
            H609_roi = self.config_roi["hole609"]["A3_roi"]
            H609_thresh = self.config_roi["hole609"]["thresholds"]
            R_roi = self.config_roi["missingriser"]["A3_roi"]
            R_thresh = self.config_roi["missingriser"]["thresholds"]
        if index == 4 :
            # Defects in Angle #4 : Missing riser
            R_roi = self.config_roi["missingriser"]["A4_roi"]
            R_thresh = self.config_roi["missingriser"]["thresholds"]
                
        defects = []
    
        # Broken Pin defect classification 
        BP_bndbx = []
        if len(BP_roi) > 0 :
            thresh = BP_thresh
            BP_bndbx = det_brokenpin(BP_roi,  src, overlay, thresh)
            for count in range (0, len(BP_bndbx)) :
                [x, y, x1, y1] = BP_bndbx[count]
                # Convert coordinates to correspond to un-warped image
                coord = np.asarray([[[x, y], [x1, y1]]], dtype = "float32")
                new_coord = cv2.perspectiveTransform(coord, np.linalg.inv(mat))[0]
                x  = new_coord[0][0]
                y  = new_coord[0][1]
                x1 = new_coord[1][0]
                y1 = new_coord[1][1]
                # Defect class of 0 => BP defect
                # (x,y) -> top left bounding box coordinates
                # (x1,y1) -> bottom right bounding box coordinates
                defects.append(Defect(0,(x, y), (x1, y1)))
        
        # Missing riser defect classification
        R_bndbx = []
        if len(R_roi) > 0 :
            thresh = R_thresh    
            R_bndbx = det_missingriser(R_roi, src, overlay, thresh, index)
            for count in range (0, len(R_bndbx)) :
                [x, y, x1, y1] = R_bndbx[count]
                # Convert coordinates to correspond to un-warped image
                coord = np.asarray([[[x, y], [x1, y1]]], dtype = "float32")
                new_coord = cv2.perspectiveTransform(coord, np.linalg.inv(mat))[0]
                x  = new_coord[0][0]
                y  = new_coord[0][1]
                x1 = new_coord[1][0]
                y1 = new_coord[1][1]
                # Defect class of 1 => MR defect
                # (x,y) -> top left bounding box coordinates
                # (x1,y1) -> bottom right bounding box coordinates
                defects.append(Defect(1,(x, y), (x1, y1)))
        
        # Runner crack defect classification        
        RC_bndbx = []
        if len(RC_roi) > 0 :
            thresh = RC_thresh
            RC_bndbx = det_runnercrack(RC_roi, src, overlay, thresh)
            for count in range (0, len(RC_bndbx)) :
                [x, y, x1, y1] = RC_bndbx[count]
                # Convert coordinates to correspond to un-warped image
                coord = np.asarray([[[x, y], [x1, y1]]], dtype = "float32")
                new_coord = cv2.perspectiveTransform(coord, np.linalg.inv(mat))[0]
                x  = new_coord[0][0]
                y  = new_coord[0][1]
                x1 = new_coord[1][0]
                y1 = new_coord[1][1]
                # Defect class of 2 => RC defect
                # (x,y) -> top left bounding box coordinates
                # (x1,y1) -> bottom right bounding box coordinates
                defects.append(Defect(2,(x, y), (x1, y1)))
 
        # Hole 609 defect classification
        H_bndbx = []
        if len(H609_roi) > 0 :
            thresh = H609_thresh
            H_bndbx = det_hole609(H609_roi, src, overlay, thresh)
            for count in range (0, len(H_bndbx)) :
                [x, y, x1, y1] = H_bndbx[count]
                # Convert coordinates to correspond to un-warped image
                coord = np.asarray([[[x, y], [x1, y1]]], dtype = "float32")
                new_coord = cv2.perspectiveTransform(coord, np.linalg.inv(mat))[0]
                x  = new_coord[0][0]
                y  = new_coord[0][1]
                x1 = new_coord[1][0]
                y1 = new_coord[1][1]
                # Defect class of 3 => H609 defect
                # (x,y) -> top left bounding box coordinates
                # (x1,y1) -> bottom right bounding box coordinates
                defects.append(Defect(3,(x, y), (x1, y1)))

        # Set state of random number generator after every frame to overcome the probabilistic 
        # nature of Flann matcher. This might need to be changed. 
        cv2.setRNGSeed(0)

        # Return list of defect classes with bounding box coordinates
        return defects
