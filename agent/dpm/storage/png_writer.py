"""PNG Writer Module
"""
import cv2


class PngWriter:
    """Object for writing PNGs
    """
    def __init__(self, compression_level):
        """Constructor

        Parameters
        ----------
        compression_level : int
            Level of PNG compression, must be in the range [0, 9], the higher
            the value the higher the smaller the size of the file and the 
            longer the compression will take

        Exceptions
        ----------
        AssertionError
            If the compression level is not in the range [0, 9]
        """
        assert compression_level >= 0 and compression_level <= 9, ('Compression'
                ' level must be >= 0 and <=9')
        self.comp_params = [cv2.IMWRITE_PNG_COMPRESSION, compression_level]

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

