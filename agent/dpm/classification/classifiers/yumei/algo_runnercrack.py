"""
Copyright (c) 2018 Intel Corporation.

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
"""

import cv2
import numpy as np


def _rotateImage(image, angle):
    image_center = tuple(np.array(image.shape[1::-1]) /2)
    rot_mat = cv2.getRotationMatrix2D(image_center, angle, 1.0)
    result = cv2.warpAffine(image, rot_mat, image.shape[1::-1], flags=cv2.INTER_LINEAR)
    return result

# Detect runner crack defect in input ROI 
# Input : defect_roi defect ROIs      
#         ref        reference image to compare 
#         test       warped test image aligned to exactly overlay reference image  
#         thresh     threshold values to compare
# Output: list of defect bounding box coordinates   
# Detect the double rainbow pattern on runner links => no defect
# Double rainbow pattern detected by counting the number of black pixels vertically.

def det_runnercrack(defect_roi, ref, test, thresh):
    bndbx = []	    # Defect bounding box
    # Thresholds
    crack_thresh = thresh["crack_thresh"]        # lower pixel threshold value, 
                                                 # will count pixels above this threshold to
                                                 # filter out noise
    dispo_thresh_c1 = thresh["dispo_thresh_c1"]  # final threshold to use for dispo crack 1
    dispo_thresh_c2 = thresh["dispo_thresh_c2"]  # final threshold to use for dispo crack 2
    dispo_thresh_c3 = thresh["dispo_thresh_c3"]  # final threshold to use for dispo crack 3

    # Get ROI crop 
    crack_count = len(defect_roi)
    for crack in range(0, crack_count) :
        x, y, x1, y1 = defect_roi[crack]
        crack_img = test[y:y1, x:x1]
        
        red = crack_img[:,:,2]
        redthresh = cv2.adaptiveThreshold(red, 1, cv2.ADAPTIVE_THRESH_GAUSSIAN_C, cv2.THRESH_BINARY_INV, 21,23)
        img = cv2.cvtColor(crack_img, cv2.COLOR_BGR2HSV)
        img = img[:,:,0]
         
        # Choose correct thresholds for crack 
        dispo_thresh = 0  
        if crack + 1 == 1 :  
            (T, thresh) = cv2.threshold(img, 150, 1, cv2.THRESH_BINARY)
            y = thresh[10:70,:].sum(axis=0)
            high = np.argmax(y<2)+16
            low = np.argmax(y<2)
            new = y[low:high]
            maxval = np.argmax(new)+low
            hdispo = img[10:70,maxval+8:maxval+20].sum(axis=0)
            crack_dispo = hdispo.sum()
        elif crack + 1 == 2 :     
            img = redthresh
            img = _rotateImage(img, -43)
            y = img[20:80,:].sum(axis=0)
            cntr=0
            for j in range(len(y)):
                  if(y[j] > 30):
                      cntr = cntr+1
                  else:
                      cntr = 0
                  if cntr == 3:
                      break
            high = j + 18
            low = j+6
            crack_dispo = y[low:high].sum()
        elif crack + 1 == 3 :        
            a = img[20:80:,:].sum(axis=0)
            maxval = np.argmax(a[:])
            rdispo = redthresh[20:80,maxval+7:maxval+13].sum(axis=0)
            crack_dispo = rdispo.sum()


        if crack_dispo < dispo_thresh_c1 and crack == 0:
            bndbx.append(defect_roi[crack])
        elif crack_dispo < dispo_thresh_c2 and crack ==1:
            bndbx.append(defect_roi[crack])
        elif crack_dispo < dispo_thresh_c3 and crack ==2:
            bndbx.append(defect_roi[crack])

    return bndbx
