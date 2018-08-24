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

# Detect missing riser defect in input ROI   
# Input : defect_roi defect ROIs   
#         ref        reference image to compare   
#         test       warped test image aligned to exactly overlay reference image 
#         thresh     threshold values for defect classification 
# Output: list of defect bounding box coordinates 

def det_missingriser(defect_roi, ref, test, thresh, index):

    # Initialize defect bounding box as an empty array 
    bndbx = []

    # Angle #2 defect detection
    if index == 2:
        # Get thresholds
        red_thresh_A2 = thresh["red_thresh_A2"]
        # loop through all risers
        for riser in range (0, len(defect_roi)):
            x, y, x1, y1 = defect_roi[riser]
            riser_test = test[y:y1, x:x1]
           
            # Risers in Angle #2 use mean of red channel for classification. If riser is present,
            # ROI has shiny metal surface to reflect light. Hence if mean red is below threshold,
            # no metal surface => defect
            red_chan = cv2.split(riser_test)[0]
            red_chan_nonzero = red_chan[np.nonzero(red_chan)]
            if red_chan_nonzero.any():
                red_metric = np.mean(red_chan_nonzero)
            else :
                red_metric = 0

            if red_metric < red_thresh_A2 :
                bndbx.append(defect_roi[riser])
               
    elif index == 3:
        # Get thresholds
        bin_r1_A3 = thresh["bin_r1_A3"]
        dispo_r1_A3 = thresh["dispo_r1_A3"]
        bin_r6_A3 = thresh["bin_r6_A3"]
        dispo_r6_A3 = thresh["dispo_r6_A3"]
        binary_thresh_r2_A3 = thresh["binary_thresh_r2_A3"]
        area_r2_A3 = thresh["area_r2_A3"]
        binary_thresh_r3_A3 = thresh["binary_thresh_r3_A3"]
        binary_thresh_r5_A3 = thresh["binary_thresh_r5_A3"]
        binary_thresh_r4_A3 = thresh["binary_thresh_r4_A3"]
        area_r4_A3 = thresh["area_r4_A3"]
        
        # loop through all risers
        for riser in range (0, len(defect_roi)):
            x, y, x1, y1 = defect_roi[riser]
            riser_test = test[y:y1, x:x1]
               
            # Risers 1,6 have reflective edges. Background if riser missing is metal part
            # Histogram shows a second spike outside of the background color range if riser present
            # If in this extended range red channel distrb is less than dispo_th(no spike) -> defect
            if (riser + 1) in [1, 6]: 
               gray = riser_test[:, :, 0]
               if riser + 1 == 1:
                   bin_limits = bin_r1_A3
                   dispo_th = dispo_r1_A3
               else:
                   bin_limits = bin_r6_A3
                   dispo_th = dispo_r6_A3
               newdispo = (np.logical_and(gray >= bin_limits[0], gray <= bin_limits[1])).sum()

               if newdispo < dispo_th :
                   bndbx.append(defect_roi[riser])

            # Riser 2,3,5,4 use contour extraction
            # Riser 4 : Riser 4 has a shiny front view, surrounded by a black border. 
            #           Detected by drawing contours
            # Riser 3,5 : Small tabs, edge is shiny than background if riser is missing. 
            #             Contour detection after binary thresholding to detect the shiny edge.
            #             If area of contour detected is above '80', shiny edge present -> no defect
            # Riser 2 : Riser 2 is always dark in comparison to background metal. 
            #           If missing, reflection from background metal allows easy contour detection
            #                       Contours detected are large in size -> defect
            #           If persent, Contours detected are scattered and small in size -> no defect
            #           Need to check this only for the largest contour detected 
            else:
                gray = riser_test[:, :, 2]
                def_det = True
                if riser + 1 == 2:
                     binary_thresh = binary_thresh_r2_A3
                     area_limits = area_r2_A3
                elif riser + 1 == 3:
                     binary_thresh = binary_thresh_r3_A3
                elif riser + 1 == 5:
                     binary_thresh = binary_thresh_r5_A3
                else:
                     binary_thresh = binary_thresh_r4_A3
                     area_limits = area_r4_A3
                ret, binary = cv2.threshold(gray, binary_thresh, 255, cv2.THRESH_BINARY)
                cnts = cv2.findContours(binary.copy(), cv2.RETR_LIST, cv2.CHAIN_APPROX_SIMPLE)
                cnts = cnts[1]
                cnts = sorted(cnts, key = cv2.contourArea, reverse = True)
                for (j, c) in enumerate(cnts) :
                    area = cv2.contourArea(c)
                    if riser+1 == 4 :
                        if area_limits[0] <= area <= area_limits[1] :
                            h, w, _ = riser_test.shape
                            M = cv2.moments(c)
                            cX = int(M["m10"] / M["m00"])
                            cY = int(M["m01"] / M["m00"])
                            if ((w/4) <= cX <= (3*w/4)) & ((h/4) <= cY <= (3*h/4)) :          
                                # Check if aspect ratio is < 2 (~1 for square. +1 as r4 is trapezoid
                                area = cv2.contourArea(c)
                                peri = cv2.arcLength(c, True)
                                approx = cv2.approxPolyDP(c, 0.04 * peri, True)
                                (x, y, w, h) = cv2.boundingRect(approx)
                                ar = w / float(h)
                                if (ar < 2) & (len(approx) <= 5) : # Check aspect ratio & # of sides (5 after adding margin for error)
                                    def_det = False
                    elif riser+1 in [3, 5]:
                        if 600 >= area >= 80 :
                            def_det = False
                        else:
                            break
                    else:   # riser2
                        if area <= area_limits:
                            def_det = False
                        break           # Need to check only the largest area for riser 2

                if def_det :
                    bndbx.append(defect_roi[riser])

    elif index == 4:
        # Get thresholds
        red_thresh_A4 = thresh["red_thresh_A4"]
        red_thresh_67_A4 = thresh["red_thresh_67_A4"]
        red_thresh_r16_A4 = thresh["red_thresh_r16_A4"]

        center_x_r10_A4 = thresh["center_x_r10_A4"]
        center_y_r10_A4 = thresh["center_y_r10_A4"]
        template_xy_ratio_r10_A4 = thresh["template_xy_ratio_r10_A4"]
        distance_th_r10_A4 = thresh["distance_th_r10_A4"]

        center_x_r11_A4 = thresh["center_x_r11_A4"]
        center_y_r11_A4 = thresh["center_y_r11_A4"]
        template_xy_ratio_r11_A4 = thresh["template_xy_ratio_r11_A4"]
        val_th_r11_A4 = thresh["value_th_r11_A4"]

        center_x_r12_A4 = thresh["center_x_r12_A4"]
        center_y_r12_A4 = thresh["center_y_r12_A4"]
        template_xy_ratio_r12_A4 = thresh["template_xy_ratio_r12_A4"]
        val_th_r12_A4 = thresh["value_th_r12_A4"]

        center_x_r13_A4 = thresh["center_x_r13_A4"]
        center_y_r13_A4 = thresh["center_y_r13_A4"]
        template_xy_ratio_r13_A4 = thresh["template_xy_ratio_r13_A4"]
        val_th_r13_A4 = thresh["value_th_r13_A4"]

        center_x_r14_A4 = thresh["center_x_r14_A4"]
        center_y_r14_A4 = thresh["center_y_r14_A4"]
        template_xy_ratio_r14_A4 = thresh["template_xy_ratio_r14_A4"]
        distance_th_r14_A4 = thresh["distance_th_r14_A4"]
        val_th_r14_A4 = thresh["value_th_r14_A4"]

        for riser in range(0, len(defect_roi)):
            x, y, x1, y1 = defect_roi[riser]
            riser_ref = ref[y:y1, x:x1]
            riser_test = test[y:y1, x:x1]
              
            # To defect defect in riser 1-9, 15, 16 use red channel distribution            
            # If missing riser, no metal surface to reflect light back. Hence low red channel distrib
            # Above risers if missing will show background wall (no reflective surface) 
            # If mean nonzero red channel distribution is less than threshold -> defect
            if (1 <= (riser + 1) <= 9) or (15 <= (riser + 1) <= 16) :
                red_chan = cv2.split(riser_test)[0]
                red_chan_nonzero = red_chan[np.nonzero(red_chan)] 
                if red_chan_nonzero.any():
                    red_metric = np.mean(red_chan_nonzero)
                else :
                    red_metric = 0
                if ((riser + 1 == 6) or (riser + 1 == 7)):
                    red_thresh = red_thresh_67_A4
                elif (riser + 1 == 16):
                    red_thresh = red_thresh_r16_A4
                else :
                    red_thresh = red_thresh_A4
                if red_metric < red_thresh :
                    bndbx.append(defect_roi[riser])
            
            # To detect defect in riser 10 - 14, use template matching.
            # Threshold based on min val and distance from ref  
            elif 10 <= riser + 1 <= 14:
                def_det = False
                riser_ref = cv2.cvtColor(riser_ref, cv2.COLOR_BGR2GRAY)
                riser_test = cv2.cvtColor(riser_test, cv2.COLOR_BGR2GRAY)
                if riser + 1 == 10:
                    center_x = center_x_r10_A4
                    center_y = center_y_r10_A4
                    template_xy_ratio = template_xy_ratio_r10_A4
                elif riser + 1 == 11:
                    center_x = center_x_r11_A4
                    center_y = center_y_r11_A4
                    template_xy_ratio = template_xy_ratio_r11_A4
                elif riser + 1 == 12:
                    center_x = center_x_r12_A4
                    center_y = center_y_r12_A4
                    template_xy_ratio = template_xy_ratio_r12_A4
                elif riser + 1 == 13:
                    center_x = center_x_r13_A4
                    center_y = center_y_r13_A4
                    template_xy_ratio = template_xy_ratio_r13_A4
                else: # riser 14
                    center_x = center_x_r14_A4
                    center_y = center_y_r14_A4
                    template_xy_ratio = template_xy_ratio_r14_A4

                w, h = riser_ref.shape[::-1]
                temp_center = (int(w*center_x), int(h*center_y))
                template = riser_ref[temp_center[1]-int(0.5*template_xy_ratio*h):temp_center[1]+int(0.5*template_xy_ratio*h), temp_center[0]-int(0.5*template_xy_ratio*w):temp_center[0]+int(0.5*w*template_xy_ratio)]
                w, h = template.shape[::-1]

                riser_test_copy = riser_test.copy()
                method = eval('cv2.TM_SQDIFF')
                # Apply template matching
                res = cv2.matchTemplate(riser_test,template,method)
                min_val, max_val, min_loc, max_loc = cv2.minMaxLoc(res)
                top_left = min_loc
                bottom_right = (top_left[0] + w, top_left[1] + h)      
                new_center = (top_left[0] + w/2, top_left[1] + h/2)
                distance_from_ref = np.sqrt( (new_center[0] - temp_center[0])**2 + (new_center[1] - temp_center[1])**2 )
                
                if (riser + 1 == 10) & (distance_from_ref > distance_th_r10_A4) :
                    def_det = True
                elif (riser + 1 == 11) & (min_val > val_th_r11_A4) :
                    def_det = True
                elif (riser + 1 == 12) & (min_val > val_th_r12_A4) :
                    def_det = True
                elif (riser + 1 == 13) & (min_val > val_th_r13_A4) :
                    def_det = True
                elif (riser + 1 == 14) & (distance_from_ref > distance_th_r14_A4) & (min_val > val_th_r14_A4) :
                    def_det = True
                if def_det == True:
                    bndbx.append(defect_roi[riser])            

    return bndbx


    



