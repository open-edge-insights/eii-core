import cv2
import numpy as np
from skimage.measure import compare_ssim 

bndbx = []                 # Defect bounding box

def det_missingriser(defect_roi, ref, test, thresh, index):
    # Thresholds
    A2_ssim_thresh = thresh["A2_ssim_thresh"]          # Threshold for deciding defect occurence in A2
    A3_ssim_thresh = thresh["A3_ssim_thresh"]          # Threshold for deciding defect occurence in A3
    A4_ssim_thresh = thresh["A4_ssim_thresh"]          # Threshold for deciding defect occurence in A4
    A4_r16_ssim_mask = thresh["A4_r16_ssim_mask"]      # Mask for A4 riser #16 (big surface area)
    A4_r16_ssim_thresh = thresh["A4_r16_ssim_thresh"]  # Threshold for A4 riser #16

    if index == 2 :
        diff_thresh = A2_ssim_thresh
    if index == 3 :
        diff_thresh = A3_ssim_thresh
    if index == 4 :
        diff_thresh = A4_ssim_thresh
   
    for riser in range(0, len(defect_roi)):
        # Crop ROI region from test and ref image
        x, y, x1, y1 = defect_roi[riser]
        riser_ref = ref[y:y1, x:x1]
        riser_test = test[y:y1, x:x1]

        # Difference between warped test and ref image using ssim    
        (score,ssim_diff) = compare_ssim(cv2.cvtColor(riser_ref, cv2.COLOR_BGR2GRAY),
                                         cv2.cvtColor(riser_test, cv2.COLOR_BGR2GRAY), full = True)
        # Differences between src and overlay appear as dark spots in ssim_diff.
        # Mask lighter areas to remove similar regions in src and overlay.
        ssim_diff = (ssim_diff * 255).astype("uint8")

        # Threshold ssim difference 
        mask = ssim_diff.copy()
        maskedDiff = mask.copy()

        if riser == 16 - 1 :
            # If riser #16, mask differences due to irregularities on surface
            imask = mask < A4_r16_ssim_mask
            maskedDiff = np.zeros_like(ssim_diff, np.uint8)
            maskedDiff[imask] = ssim_diff[imask]
        
        # Apply OTSU thresholding to remove noise in masked ssim-difference
        ret, thresh = cv2.threshold(maskedDiff, 0, 255, cv2.THRESH_BINARY+cv2.THRESH_OTSU)

        if riser == 16 - 1 :
            # If riser #16, use the masked ssim difference
            diff_thresh = A4_r16_ssim_thresh
            area = (x1 - x) * (y1 - y)
            diff_area = cv2.countNonZero(thresh) / area
 
        else :
            # If count of white pixels > threshold, riser is missing
            area = (x1 - x) * (y1 - y)
            diff_area = (area - cv2.countNonZero(thresh)) / area

        if diff_area > diff_thresh :
            bndbx.append(defect_roi[riser])
           

    return(bndbx)


    



