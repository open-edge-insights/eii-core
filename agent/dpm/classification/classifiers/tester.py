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

"""Simple classifier that does nothing. Meant for testing.
"""
import os
import logging
import cv2
from agent.db.defect import Defect


class Classifier:
    """Test classifier
    """
    def __init__(self, cascade_file, reject_levels, level_weights):
        """Constructor
        """
        self.log = logging.getLogger(__name__)
        assert os.path.exists(cascade_file), ('Cascade file does not exist: {}').format(
                cascade_file)
        self.face_cascade = cv2.CascadeClassifier(cascade_file)
        self.reject_levels = reject_levels
        self.level_weights = level_weights

    def classify(self, frame_num, img, user_data):
        """Classify the given image against the specified reference image.

        Parameters
        ----------
        frame_num : int
            Frame count since the start signal was received from the trigger
        img : NumPy Array
            Image to classify
        user_data : Object
            Extra data passed forward from the trigger

        Returns
        -------
            List of defects on the part in the frame, empty array if no defects
            exist on the part
        """
        self.log.debug('Received frame to classify')
        gray = cv2.cvtColor(img, cv2.COLOR_RGB2GRAY)
        faces = self.face_cascade.detectMultiScale(
                gray, self.reject_levels, self.level_weights) 

        if len(faces) == 0:
            self.log.debug('No faces found')
            return [] 

        defects = []
        for (x, y, w, h) in faces:
            # Top left and bottom right of the rectangle
            defects.append(Defect(0x00, (x, y), (x + w, y + h)))
        
        return defects

