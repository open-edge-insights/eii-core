import cv2
import numpy as np

bndbx = []

def det_hole609(defect_roi, ref, test, thresh):
    # Threshold
    dispo_thresh = thresh["dispo_thresh"]

    x, y, x1, y1 = defect_roi[0]
    img = test[y:y1, x:x1]
  
    img = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)
    img = cv2.resize(img, (100, 100))
    img = img[:, :, 0]  # use red color channel
    mask = np.zeros(img.shape[:2], dtype = 'uint8')
    cv2.circle(mask, (51, 52), 24, 255, -1)
    masked = cv2.bitwise_and(img, img, mask = mask)
    newdispo = (np.logical_and(masked >= 1, masked <= 100)).sum()    # TODO: Check

    threshold = [newdispo]
    def_index = 'Hole609_' + str(1)
    
    if newdispo > dispo_thresh :
        bndbx.append(defect_roi[0])

    return bndbx 
