import cv2
import numpy as np


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

def det_hole609(defect_roi, ref, test, thresh):
    bndbx = []
    # Threshold
    dispo_thresh = thresh["dispo_thresh"]

    x, y, x1, y1 = defect_roi[0]
    img = test[y:y1, x:x1]
  
    img = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)
    img = cv2.resize(img, (100, 100))
    img_copy = img.copy()
    img = img[:, :, 0]  # use red color channel
    mask = np.zeros(img.shape[:2], dtype = 'uint8')
    cv2.circle(mask, (51, 52), 24, 255, -1)
    masked = cv2.bitwise_and(img, img, mask = mask)
    hist = _calc_hist(img, mask)
    newdispo = hist#np.max(hist)

    if newdispo >= dispo_thresh :
        bndbx.append(defect_roi[0])
    
    return bndbx 
