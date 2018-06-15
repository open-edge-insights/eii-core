import cv2
import numpy as np

bndbx = []	    # Defect bounding box

# Detect runner crack defect in input ROI 
# Input : defect_roi defect ROIs      
#         ref        reference image to compare 
#         test       warped test image aligned to exactly overlay reference image  
#         thresh     threshold values to compare
# Output: list of defect bounding box coordinates   

def det_runnercrack(defect_roi, ref, test, thresh):
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
         
        # Gabor filter kernel
        g_kernel = cv2.getGaborKernel((3, 3), 8.0, np.pi/4, 50.0, 0.5, 0, ktype = cv2.CV_32F)
        crack_img = cv2.cvtColor(crack_img, cv2.COLOR_BGR2GRAY)
        # Apply gabor filter to cropped crack ROI
        filtered_img = cv2.filter2D(crack_img, cv2.CV_8UC3, g_kernel)
        # Invert image to make it easier to visualize and count
        filtered_img = cv2.bitwise_not(filtered_img)
        # count pixels above 20
        crack_metric = np.sum(filtered_img >= crack_thresh)

        # Choose correct thresholds for crack 
        dispo_thresh = 0  
        if crack + 1 == 1 :  
            dispo_thresh = dispo_thresh_c1     
        elif crack + 1 == 2 :     
            dispo_thresh = dispo_thresh_c2  
        elif crack + 1 == 3 :        
            dispo_thresh = dispo_thresh_c3   

        # true if crack, false if no crack
        crack_dispo = crack_metric > dispo_thresh 

        if crack_dispo == True:
            bndbx.append(defect_roi[crack])

    return bndbx
