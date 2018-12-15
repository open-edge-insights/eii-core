# Copyright (c) 2018 Intel .
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included
# in all
# copies or substantial portions of the Software. Software to be used for
# Made in China 2025 initiatives.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

import cv2
import numpy as np
import math

DEBUG = 0


def get_part_anchor_points(roi, ref_anchor_pt, min_distance):
    """
    This function returns the anchor points on the part
     for the given roi (upperROI or lowerROI)
    roi : ROI of the part in which the anchor point needs
     to be calculated (upperROI or lowerROI)
    ref_anchor_pt : Reference point for localizing the anchor point
    min_distance : minimum distance parameter used in calculating
     anchor points of upperROI and lowerROI
    return : anchor point for the part (upperROI or lowerROI)
    """
    # Converting the ROI to grayscale
    gray = cv2.cvtColor(roi, cv2.COLOR_BGR2GRAY)

    # Increasing contrast of gray image using eqalizeHist
    hist = cv2.equalizeHist(gray)

    # Increasing the intensity of the ROI
    matrix = np.ones(hist.shape, dtype="uint8") * 90
    subtracted = cv2.subtract(hist, matrix)

    # Sharpening the ROI to get more clarity
    kernel = np.array([[-1, -1, -1], [-1, 20, -1], [-1, -1, -1]])
    sharpened = cv2.filter2D(subtracted, -1, kernel)

    # Detecting edges using canny edge detector
    edges = cv2.Canny(sharpened, 200, 255, apertureSize=3)

    # Detecting the corners in the ROI
    corners = cv2.goodFeaturesToTrack(edges, 10, 0.01, 1)
    corners = np.int0(corners)

    list_of_corners = []
    ref_x, ref_y = ref_anchor_pt
    roi_mat_copy = roi.copy()
    if DEBUG:
        cv2.circle(roi_mat_copy, (ref_x, ref_y), 3, (0, 0, 255), -1)

    # Finding xy coordinates of the corners
    for i in corners:
        x, y = i.ravel()
        if DEBUG:
            cv2.circle(roi_mat_copy, (x, y), 3, (0, 255, 0), -1)
        list_of_corners.append([x, y])

    if DEBUG:
        cv2.imshow("shi_Thomashi_corners", roi_mat_copy)
        cv2.waitKey(0)

    # Calculating the distance between the corners detected and
    # reference anchor point
    coord = [[]] * len(list_of_corners)
    dis = []
    i = 0
    for item in list_of_corners:
        x, y = item
        distance = np.linalg.norm(np.array((ref_x, ref_y)) - np.array((x, y)))
        dis.append(distance)
        coord[i].append([distance, x, y])
        i = i + 1
    ind = dis.index(min(dis))

    # Getting the nearest point to ref anchor point
    # which sits inside the threshold window as anchor point
    # If no candidate point satisfies the criteria, reference anchor
    # point is returned as the anchor point
    if (min(dis) <= min_distance):
        x, y = coord[0][ind][1], coord[0][ind][2]
        if DEBUG:
            roi_mat_copy2 = roi.copy()
            cv2.circle(roi_mat_copy2, (x, y), 3, (0, 255, 0), -1)
            cv2.imshow("one_corner", roi_mat_copy2)
            cv2.waitKey(0)
        return x, y
    else:
        if DEBUG:
            roi_mat_copy2 = roi.copy()
            cv2.circle(roi_mat_copy2, (ref_x, ref_y), 3, (0, 255, 0), 3)
            cv2.imshow("one_corner", roi_mat_copy2)
        return ref_x, ref_y


def get_runner_anchor_point(roi, roi_ref_point):
    """
    This function returns the anchor points on the runner arm
    roi : runner ROI where the anchor point is localized
    roi_ref_point : Reference point for localizing the anchor point
    return : runner_anchor_pt - Predicted anchor point
    """
    # Converting runner roi to grayscale
    gray_rroi = cv2.cvtColor(roi, cv2.COLOR_BGR2GRAY)

    # Reducing the ROI to reduce false-positives
    height, width = gray_rroi.shape
    new_width = int(1 * (width / 4))

    # Selecting small ROI to localize anchor points better
    roi = gray_rroi[0: height, new_width: width]

    # Applying histogram equalization
    hist = cv2.equalizeHist(roi)

    # Thresholding to get binary image
    _, thresh_rroi = cv2.threshold(hist, 145, 255, cv2.THRESH_BINARY)

    # Opening (Morphological Transformation) to remove
    # the small noises in between the contours
    opening_kernel = np.ones((3, 3), np.uint8)
    opening = cv2.morphologyEx(thresh_rroi, cv2.MORPH_OPEN,
                               opening_kernel, iterations=2)

    # Finding contours
    _, contours_rroi, _ = cv2.findContours(opening, cv2.RETR_EXTERNAL,
                                           cv2.CHAIN_APPROX_SIMPLE)

    # Sorting contours
    sorted_cnts_rroi = sorted(contours_rroi, key=cv2.contourArea)

    # Using approxpPolyDP to find corners
    epsilon = 0.02 * cv2.arcLength(sorted_cnts_rroi[-1], True)
    approx = cv2.approxPolyDP(sorted_cnts_rroi[-1], epsilon, False)

    if DEBUG:
        roi_clr = cv2.cvtColor(opening, cv2.COLOR_GRAY2BGR)
        cv2.drawContours(roi_clr, [approx], 0, (0, 0, 255), 2)
        cv2.imshow('final', roi_clr)
        cv2.waitKey(0)

    # First time declaration of anchor point
    runner_anchor_pt = approx[0]

    # Calculating distance between ref anchor point
    # and the points detected in the ROI
    dist_from_ref_anchor_pt = np.linalg.norm(np.array((roi_ref_point)) -
                                             np.array((runner_anchor_pt)))

    if DEBUG:
        print("""Distance between first runner anchor pt
              and ref anchor pt : """, dist_from_ref_anchor_pt)

    # Comparing distance of all the points with respect to ref_anchor_point to
    # filter out actual anchor point
    for i in approx:
        dst = np.linalg.norm(np.array((roi_ref_point)) - np.array((i)))
        if dist_from_ref_anchor_pt > int(dst):
            runner_anchor_pt = (i[0][0], i[0][1])
            dist_from_ref_anchor_pt = int(dst)

    # Adding back the width to get back the orginal x-axis of the runner ROI
    relative_runner_anchor_pt = (runner_anchor_pt[0] +
                                 new_width, runner_anchor_pt[1])

    return relative_runner_anchor_pt


def get_anchor_points(roi_mat, ref_anchor_point, mindis, source_xy):
    """
    This function returns the anchor points on the part and the runner arm
    roi_mat : List containing matrices of upperROI, lowerROI and runnerROI
    ref_anchor_point : List containing reference anchor points for
     upperROI, lowerROI and runnerROI
    mindis : List containing minimum distance parameter used in
     calculating anchor points of upperROI and lowerROI
    source_xy : List containing the x, y coordinates of the start of
     each ROIs, used for calculating relative xy coordinates of
     anchor points on source image
    return : Returns predicted anchor points on upperROI, lowerROI
     and runnerROI
    """
    upper_anchor_pt = get_part_anchor_points(roi_mat[0],
                                             ref_anchor_point[0],
                                             mindis[0])

    lower_anchor_pt = get_part_anchor_points(roi_mat[1],
                                             ref_anchor_point[1],
                                             mindis[1])

    runner_anchor_pt = get_runner_anchor_point(roi_mat[2], ref_anchor_point[2])

    upper_anchor_pt_in_src = ((source_xy[0][0] +
                              upper_anchor_pt[0]),
                              (source_xy[0][1] +
                               upper_anchor_pt[1]))
    if DEBUG:
        print("upper_anchor_pt_in_src : ", upper_anchor_pt_in_src)

    lower_anchor_pt_in_src = ((source_xy[1][0] +
                               lower_anchor_pt[0]),
                              (source_xy[1][1] +
                               lower_anchor_pt[1]))
    if DEBUG:
        print("lower_anchor_pt_in_src : ", lower_anchor_pt_in_src)

    runner_anchor_pt_in_src = ((source_xy[2][0] +
                                runner_anchor_pt[0]),
                               (source_xy[2][1] +
                                runner_anchor_pt[1]))
    if DEBUG:
        print("runner_anchor_pt_in_src : ", runner_anchor_pt_in_src)

    return upper_anchor_pt_in_src,\
        lower_anchor_pt_in_src, runner_anchor_pt_in_src


def get_anchor_metric(upper_anchor_pt, lower_anchor_pt, runner_anchor_pt):
    """
    Function to calculate the anchor metric
    upper_anchor_pt : upper part anchor point
    lower_anchor_pt : lower part anchor point
    runner_anchor_pt : runner part anchor point
    return : anchor metric
    """

    # Distance between anchor point2 (lowerROI) and anchor point3 (runnerROI)
    dist_l_r = np.linalg.norm(np.array((lower_anchor_pt[0],
                                        lower_anchor_pt[1])) -
                              np.array((runner_anchor_pt[0],
                                        runner_anchor_pt[1])))
    # Distance between anchor point1 (upperROI) and anchor point3 (runnerROI)
    dist_u_r = np.linalg.norm(np.array((upper_anchor_pt[0],
                                        upper_anchor_pt[1])) -
                              np.array((runner_anchor_pt[0],
                                        runner_anchor_pt[1])))
    # Distance between anchor point2 (lowerROI) and anchor point1 (upperROI)
    dist_l_u = np.linalg.norm(np.array((lower_anchor_pt[0],
                                        lower_anchor_pt[1])) -
                              np.array((upper_anchor_pt[0],
                                        upper_anchor_pt[1])))

    """
    Calculating angles needed for computing anchor metric using cosine rule
    If a, b and c are the lengths of the sides opposite the angles
     A, B and C in a triangle, then:
    angle A = Inv.Cosine((b*b + c*c - a*a) / (2*b*c))
    """
    # angle_u is angle at upperROI anchor point
    angle_u = math.acos(((dist_u_r * dist_u_r) +
                         (dist_l_u * dist_l_u) -
                         (dist_l_r * dist_l_r)) / (2 * dist_u_r * dist_l_u))

    # angle_l is angle at lowerROI anchor point
    angle_l = math.acos(((dist_l_r * dist_l_r) +
                         (dist_l_u * dist_l_u) -
                         (dist_u_r * dist_u_r)) / (2 * dist_l_r * dist_l_u))

    anchor_metric = angle_u / angle_l

    return anchor_metric


def calc_warpage_metric(roi, ref, img, thresh):
    """
    Function to detect warpage on the input image
    img : post clahe homographed image of the autopart including runner arm
    roiconfig : config file containing coordinates of the ROIs
     for localizing anchor points
    return : anchor metric and 3 anchor points
    """
    roi_mat = []
    ref_anchor_points = []
    mindis = []
    source_xy = []

    # Getting coordinates for upperROI
    upper_mat_x, upper_mat_y, upper_mat_x1, upper_mat_y1 = roi[0]

    # Cropping the img to upperROI
    upper_mat = img[upper_mat_y: upper_mat_y1, upper_mat_x: upper_mat_x1]
    if DEBUG:
        cv2.imshow('upper_mat', upper_mat)
        cv2.waitKey(0)
    roi_mat.append(upper_mat)

    # Getting coordinates for lowerROI
    lower_mat_x, lower_mat_y, lower_mat_x1, lower_mat_y1 = roi[1]

    # Cropping the img to lowerROI
    lower_mat = img[lower_mat_y: lower_mat_y1, lower_mat_x: lower_mat_x1]
    if DEBUG:
        cv2.imshow('lower_mat', lower_mat)
        cv2.waitKey(0)
    roi_mat.append(lower_mat)

    # Getting coordinates for runnerROI
    runner_mat_x, runner_mat_y, runner_mat_x1, runner_mat_y1 = roi[2]

    # Cropping the img to the runnerROI
    runner_mat = img[runner_mat_y: runner_mat_y1, runner_mat_x: runner_mat_x1]
    if DEBUG:
        cv2.imshow('runner_mat', runner_mat)
        cv2.waitKey(0)
    roi_mat.append(runner_mat)

    # Reference anchor point for upperROI on reference image
    upper_ref_pt = (thresh["upperRef"][0], thresh["upperRef"][1])
    ref_anchor_points.append(upper_ref_pt)

    # Reference anchor point for lowerROI on reference image
    lower_ref_pt = (thresh["lowerRef"][0], thresh["lowerRef"][1])
    ref_anchor_points.append(lower_ref_pt)

    # Reference anchor point for runnerROI on reference image
    runner_ref_pt = (thresh["runnerRef"][0], thresh["runnerRef"][1])
    ref_anchor_points.append(runner_ref_pt)

    # Threshold area to detect the anchor point in upperROI
    upper_mindis = thresh["minDis_upper"]
    mindis.append(upper_mindis)

    # Threshold area to detect the anchor point in lowerROI
    lower_mindis = thresh["minDis_lower"]
    mindis.append(lower_mindis)

    # x, y coordinates of the start of upperROI,
    # used for calculating relative x,y coordinates of
    # upper anchor point on source image
    u_source_xy = (upper_mat_x, upper_mat_y)
    source_xy.append(u_source_xy)

    # x, y coordinates of the start of lowerROI,
    # used for calculating relative x,y coordinates of
    # lower anchor point on source image
    l_source_xy = (lower_mat_x, lower_mat_y)
    source_xy.append(l_source_xy)

    # x, y coordinates of the start of runnerROI,
    # used for calculating relative x,y coordinates of
    # runner anchor point on source image
    r_source_xy = (runner_mat_x, runner_mat_y)
    source_xy.append(r_source_xy)

    # Getting anchor points
    upper_anchor_pt, lower_anchor_pt, runner_anchor_pt = get_anchor_points(
            roi_mat,
            ref_anchor_points,
            mindis,
            source_xy)
    # Getting anchor metric
    anchor_metric = get_anchor_metric(upper_anchor_pt,
                                      lower_anchor_pt,
                                      runner_anchor_pt)
    if DEBUG:
        cv2.rectangle(img, (778, 587), (778+88, 587+71), (0, 255, 0), 2)
        cv2.rectangle(img, (837, 809), (837+73, 809+53), (0, 255, 0), 2)
        cv2.rectangle(img, (1644, 410), (1644+199, 410+128), (0, 255, 0), 2)
        cv2.line(img, (upper_anchor_pt[0], upper_anchor_pt[1]),
                 (lower_anchor_pt[0], lower_anchor_pt[1]), (0, 255, 0), 2)
        cv2.line(img, (runner_anchor_pt[0], runner_anchor_pt[1]),
                 (lower_anchor_pt[0], lower_anchor_pt[1]), (0, 255, 0), 2)
        cv2.line(img, (runner_anchor_pt[0], runner_anchor_pt[1]),
                 (upper_anchor_pt[0], upper_anchor_pt[1]), (0, 255, 0), 2)
        disp_string = "Anchor metric = {}".format(anchor_metric)
        cv2.putText(img, disp_string, (1200, 100), cv2.FONT_HERSHEY_COMPLEX,
                    1, (0, 255, 255),  thickness=2, lineType=cv2.LINE_AA)
        cv2.imwrite("warpage_metric_triangulation.png", img)

    return anchor_metric
