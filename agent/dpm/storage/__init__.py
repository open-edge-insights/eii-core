"""Local storage management module.
"""
import os
import logging
import inspect
import queue
import threading as th
import queue 

from .png_writer import PngWriter
from .jpeg_writer import JpegWriter
from agent.etr_utils import abspath


class LocalStorageError(Exception):
    """Exception thrown by the LocalStorage object.
    """
    pass


def new_writer(config):
    """Initialize a new image writer object. This is only for use inside of
    the storage module.

    Parameters
    ----------
    config : Dict
        Dictionary configuration object for the writer

    Exceptions
    ----------
    LocalStorageError
        If the writer type is invalid, or if the configuration values are
        incorrect/missing
    """
    try:
        writer_type = config['type']
        writer_config = config['config']

        if writer_type == 'png':
            class_ = PngWriter
        elif writer_type == 'jpeg':
            class_ = JpegWriter
        else:
            raise LocalStorageError('Unknown writer type: {}'.format(writer_type))

        arg_names = inspect.getargspec(class_.__init__).args[1:]
        if len(arg_names) > 0:
            args = [writer_config[arg] for arg in arg_names]
        else:
            args = []
        return class_(*args)
    except KeyError as e:
        raise LocalStorageError('Image format missing config value: {}'.format(e))
    except AssertionError as e:
        raise LocalStorageError(e)


class LocalStorage:
    """Handles the storage of image data locally on the gateway.

    The configuration object for the LocalStorage class must have the following
    values:
        - `folder`       : Folder in which to save the images
        - `image_format` : Dictionary object specified below

    The `image_format` object must have the following values:
        - `type`   : Format type, must be either png or jpeg
        - `config` : Configuration for the format type writer (see below)

    PNG
    ---
    For the PNG writer, you must specify the following configuration value:
        - `compression_level` : Int between 0 and 9, the higher the more
           compressed the image will be, but also the longer the compression
           will take

    JPEG
    ----
    For the JPEG writer, you must specify the following configuration value:
        - `quality` : Quality of the JPEG file to write between 0 and 100. The
          higher the value the higher the quality and also the larger the file.
    """
    def __init__(self, config):
        """Constructor

        Parameters
        ----------
        config : Dict
            Dictionary configuration object

        Exceptions
        ----------
        LocalStorageError
            If the save folder does not exist, if the image format is unknown,
            or if the folder cannot be accessed by the program
        """
        self.log = logging.getLogger(__name__)
        self.th = th.Thread(target=self._run)
        self.queue = queue.Queue() 
        self.stop_ev = th.Event()

        try:
            self.folder = abspath(config['folder'])
            image_format_config = config['image_format']
            self.writer = new_writer(image_format_config)
            self.image_format = image_format_config['type']
            self.file_ext = '.' + self.image_format
        except KeyError as e:
            raise LocalStorageError(
                    'Local storage configuration missing value: {}'.format(e))

        if not os.path.exists(self.folder):
            raise LocalStorageError(
                    'Save folder "{}" does not exist'.format(self.folder))

        if not os.access(self.folder, os.W_OK):
            raise LocalStorageError(
                    'Agent does not have write access to the save folder: {}'\
                    .format(self.folder))

        if self.image_format not in ('jpeg', 'png'):
            raise LocalStorageError(
                    'Unknown image format: {}'.format(self.image_format))

    def start(self):
        """Start the storage thread
        """
        self.th.start()

    def stop(self):
        """Stop the thread used for saving the images.
        """
        self.stop_ev.set()
        self.th.join()
    
    def get_format(self):
        """Get the local storage's image format type.
        """
        return self.image_format

    def get_image_path(self, image):
        """Get the full path to an image from the database image object.

        Parameters
        ----------
        image : agent.db.image.Image
            Database image object

        Returns
        -------
        String
            String of the fill path to the image, does not guarentee it exists
        """
        return os.path.join(self.folder, image.filename)

    def write(self, filename, image):
        """Writes the given image to the local storage with the given filename.
        Note that the filename must end with the provided image format file
        extention.

        Parameters
        ----------
        filename : str
            Filename for the saved image
        image : NumPy Array
            NumPy array of the raw image data to save

        Exceptions
        ----------
        LocalStorageError
            If the file extension for the filename is incorrect, or if writing
            the file fails, or if the file already exists.
        """
        fn, ext = os.path.splitext(filename)
        
        if ext != self.file_ext:
            raise LocalStorageError(
                    'Incorrect file extension "{0}", should be "{1}"'.format(
                        ext, self.file_ext))

        full_name = os.path.join(self.folder, filename)
        
        if os.path.exists(full_name):
            raise LocalStorageError('File "{}" already exists'.format(full_name))
        
        self.queue.put((full_name, image))

    def _run(self):
        """Private run method.
        """
        try:
            while not self.stop_ev.is_set():
                try:
                    filename, frame = self.queue.get(timeout=2)
                    self.log.info('Saving file (qsize %d): %s', 
                            self.queue.qsize(), filename)
                    self.writer.write(filename, frame)
                except queue.Empty:
                    pass
        except Exception:
            self.log.error('Error in storage:\n%s', tb.format_exc())

