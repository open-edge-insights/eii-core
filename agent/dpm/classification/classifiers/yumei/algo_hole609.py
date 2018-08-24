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

import cv2
import numpy as np

# Calculate histogram and mean sliding window in range +-3
def _calc_hist(image, mask = None):
    chans = cv2.split(image)
    hist_test = []
    for chan in chans:
        hist = cv2.calcHist([chan], [0], mask, [250], [0,250])
        hist_max = np.max(hist)
        hist_test.append(hist[np.argmax(hist)+1])
        hist_test.append(hist[np.argmax(hist)+2])
        hist_test.append(hist[np.argmax(hist)+3])
        hist_test.append(hist[np.argmax(hist)-1])
        hist_test.append(hist[np.argmax(hist)-2])
        hist_test.append(hist[np.argmax(hist)-3])
        hist_max = np.mean(hist_test)
    return hist_max

# Detect Hole 609 block in input ROI   
# Input : defect_roi defect ROIs      
#         ref        reference image to compare    
#         test       warped test image aligned to exactly overlay reference image  
#         thresh     threshold values for defect classification   
# Output: list of defect bounding box coordinates  

def det_hole609(defect_roi, ref, test, thresh):
    # Initialize defect bounding box as an empty array
    bndbx = []

    # Read threshold values
    dispo_thresh = thresh["dispo_thresh"]

    # Crop ROI region from test frame
    x, y, x1, y1 = defect_roi[0]
    img = test[y:y1, x:x1]
  
    # Mask out everything except center hole region in H609
    img = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)
    img = cv2.resize(img, (100, 100))
    img_copy = img.copy()
    img = img[:, :, 0]  # use red color channel
    mask = np.zeros(img.shape[:2], dtype = 'uint8')
    cv2.circle(mask, (51, 52), 24, 255, -1)
    masked = cv2.bitwise_and(img, img, mask = mask)

    # Calculate the histogram metric for classification dispo
    newdispo = _calc_hist(img, mask)

    if newdispo >= dispo_thresh :
        bndbx.append(defect_roi[0])
    
    return bndbx 
