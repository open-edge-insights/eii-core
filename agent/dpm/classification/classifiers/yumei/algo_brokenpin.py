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

import cv2
import numpy as np
from skimage.filters import threshold_mean 

# Detect broken pin defect in input ROI
# Input : defect_roi defect ROIs 
#	  ref        reference image to compare
#	  test       warped test image aligned to exactly overlay reference image
#         thresh     threshold values for defect classification
# Output: list of defect bounding box coordinates

def det_brokenpin(defect_roi, ref, test, thresh):
    # Initialize defect bounding box as an empty array
    bndbx = []

    # Read threshold values
    red_tolerance = thresh["red_tolerance"]
    green_thresh = thresh["green_thresh"]
    blue_thresh = thresh["blue_thresh"]   
    pinhole_6_thresh = thresh["pinhole_6_threshold"]

    pin_count = len(defect_roi)
    for hole in range(0, pin_count):
       # Crop ROI region from test frame and ref image
       x, y, x1, y1 = defect_roi[hole]
       pin_img = test[y:y1, x:x1]
       ref_pin = ref[y:y1, x:x1]
       
       # Resize pin image to fixed size
       pin_img = cv2.resize(pin_img, (100, 100))
       ref_pin = cv2.resize(ref_pin, (100, 100))

       # Find the hole in test images
       cX = 51
       cY = 51
       radius = 9
       
       # Pin hole 6 is smaller than other pin holes 
       if hole == 5 :       
           radius = 8

       # Threshold the image and detect contours on red channel
       gray = pin_img[:, :, 2]
       thresh = threshold_mean(gray)
       ret, binary = cv2.threshold(gray, thresh, 255, cv2.THRESH_BINARY)
       cnts = cv2.findContours(binary.copy(), cv2.RETR_LIST, cv2.CHAIN_APPROX_SIMPLE)
       cnts = cnts[1]
       cnts = sorted(cnts, key = cv2.contourArea, reverse = False)
       for (j, c) in enumerate(cnts) :
           area = cv2.contourArea(c)
           # Select contour that approximate the circular pinhole region in ROI
           if 600 <= area <= 1600 :
                M = cv2.moments(c)
                cX = int(M["m10"] / M["m00"])
                cY = int(M["m01"] / M["m00"])

       # Mask out everything except the pinhole region in ROI
       mask = np.zeros(pin_img.shape[:2], dtype = 'uint8')
       cv2.circle(mask, (cX, cY), radius, 255, -1)  # assuming center of image as center of hole
                                                    # radius not clipping the edge
       # Convert to RGB format
       pin_img = cv2.cvtColor(pin_img, cv2.COLOR_BGR2RGB)
       masked = cv2.bitwise_and(pin_img, pin_img, mask = mask)

       # Need second level classifier for pinhole 6, convert to HSV space and take mean of V channel
       cropimg_hsv = cv2.cvtColor(pin_img, cv2.COLOR_RGB2HSV)
       v = cropimg_hsv[:,:,2].mean()
       pinhole_6_v  = np.mean([pin_img[:,:,i].mean() for i in range(pin_img.shape[-1])])


       # Split RGB channels to read histogram distribution       
       red_chan =  cv2.split(masked)[0]
       green_chan = cv2.split(masked)[1]
       blue_chan = cv2.split(masked)[2]
 
       # Get mean of red channel distribution
       red_chan_nonzero = red_chan[np.nonzero(red_chan)]
       red_metric = np.mean(red_chan_nonzero)

       # Get mean of green channel distribution
       green_metric = np.mean(green_chan)

       # Get mean of blue channel distribution
       blue_metric = np.mean(blue_chan)

       # Repeat the same for ref image to get red channel threshold for the corresponding hole
       r_cX = 51
       r_cY = 51
       gray = ref_pin[:, :, 2]
       thresh = threshold_mean(gray)
       ret, binary = cv2.threshold(gray, thresh, 255, cv2.THRESH_BINARY)
       cnts = cv2.findContours(binary.copy(), cv2.RETR_LIST, cv2.CHAIN_APPROX_SIMPLE)
       cnts = cnts[1]
       cnts = sorted(cnts, key = cv2.contourArea, reverse = False)
       for (j, c) in enumerate(cnts) :
           area = cv2.contourArea(c)
           if 600 <= area <= 1600 :
                M = cv2.moments(c)
                r_cX = int(M["m10"] / M["m00"])
                r_cY = int(M["m01"] / M["m00"])

       r_mask = np.zeros(ref_pin.shape[:2], dtype = 'uint8')
       cv2.circle(r_mask, (r_cX, r_cY), radius, 255, -1)
       ref_pin = cv2.cvtColor(ref_pin, cv2.COLOR_BGR2RGB)
       r_masked = cv2.bitwise_and(ref_pin, ref_pin, mask = r_mask)

       # Split RGB channels to read histogram distribution       
       red_chan =  cv2.split(r_masked)[0]
       green_chan = cv2.split(r_masked)[1]   # TODO: Remove?
       blue_chan = cv2.split(r_masked)[2]    # TODO: Remove?
 
       # Get mean of red channel distribution
       red_chan_nonzero = red_chan[np.nonzero(red_chan)]
       red_thresh = np.mean(red_chan_nonzero)

       # If mean red channel distribution of test frame is more than a tolerance value added to the same for ref image
       # and the green and blue metrics are below threshold values, 
       # append ROI coordinates to bounding box list (bndbx)
       if (hole != 5) & (red_metric > red_thresh + red_tolerance) & ((green_metric > green_thresh) or (blue_metric > blue_thresh)) :
           bndbx.append(defect_roi[hole])
       elif (hole == 5):
           # If pinhole 6, check for only red channel distribution
           if (red_metric > pinhole_6_thresh):
               bndbx.append(defect_roi[hole])
       
    return bndbx
