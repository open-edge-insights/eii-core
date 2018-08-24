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

"""JPEG Writer Module
"""
import cv2


class JpegWriter:
    """Object for writing JPEGs 
    """
    def __init__(self, quality):
        """Constructor

        Parameters
        ----------
        quality : int
            JPEG image quality, must be in the range [0, 100], the higher the
            number the higher quality and larger the file

        Exceptions
        ----------
        AssertionError
            If the quality is not in the range [0, 100]
        """
        assert quality >= 0 and quality <= 100, ('Quality'
                ' level must be >= 0 and <= 100')
        self.comp_params = [cv2.IMWRITE_JPEG_QUALITY, quality]

    def write(self, filename, img):
        """Write the given image to the given file. Note that it is assumed 
        that all needed error checking is done prior to calling this method.

        Parameters
        ----------
        filename : str
            Full path to the file in which to write the image
        image : NumPy array
            NumPy array image to write
        """
        cv2.imwrite(filename, img, self.comp_params)

