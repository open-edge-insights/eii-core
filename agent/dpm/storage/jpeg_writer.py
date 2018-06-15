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

