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

import traceback as tb
import logging
import threading
import time
import cv2
import os
from . import IngestorError


try:
    import dahua_video_capture as dvc
    dahua_supported = True
except ImportError:
    dahua_supported = False


"""Video ingestor
"""


class VideoCapture:
    """Simple wrapper around external video capture objects.
    """
    def __init__(self, name, config, cap):
        """Constructor

        Parameters
        ----------
        name : str
            Name of the video capture
        config : dict or None
            Configuration of the camera, or None if it does not have one
        cap : Object
            Video capture object
        """
        self.config = config
        self.name = name
        self.cap = cap

    def __str__(self):
        return self.name

    def read(self):
        """read() method wrapper
        """
        return self.cap.read()

    def release(self):
        """release() method wrapper
        """
        return self.cap.release()


class Ingestor:
    """Ingestor object for ingesting video streams.

    The configuration object for this ingestor is as follows:

        {
            "poll_interval": <INTEGER>,
            "streams": {
                "<STREAM TYPE>": <DICT OF STREAM PARAMS>,
                ...
            }
        }

    The, "poll_interval", is how often the ingestor should loop over all of the
    camera streams and read the latest frame.

    The configuration for  stream is as follows:

        {
            "capture_streams": <LIST OF PARAMTERS FOR VideoCapture>
        }

    The opencv stream will open each of the capture streams as VideoCapture
    objects. What ever the first parameter of the cv2.VideoCapture object
    supports can be specified in the array.

    An error will be raised if Basler cameras are not supported, i.e.
    the needed library cannot be imported.
    """
    def __init__(self, config, stop_ev, on_data):
        """Constructor

        Parameters
        ----------
        config : dict
            Configuration object for the video ingestor
        stop_ev : Event
            Threading event telling the ingestor to stop
        on_data : function
            Callback to be called when data is available

        Exceptions
        ----------
        IngestorError
            If an error occurs while initializing the ingestor
        """
        self.log = logging.getLogger(__name__)
        self.stop_ev = stop_ev
        self.on_data = on_data
        self.poll_interval = config.get('poll_interval', 0.01)
        self.cameras = []
        self.cam_threads = []
        streams = config['streams']

        self.log.info('Loading OpenCV streams')
        self.cap_streams = streams['capture_streams']
        if isinstance(self.cap_streams, dict):
            for (cam_sn, camConfig) in self.cap_streams.items():

                thread_id = threading.Thread(target=self._run,
                                             args=(cam_sn, camConfig))
                self.cam_threads.append(thread_id)
        else:
            raise IngestorError('capture_streams is not a json object')

    def start(self):
        """Start the ingestor.
        """
        for thread in self.cam_threads:
            thread.start()

    def join(self):
        """Blocks until the ingestor has stopped running. Note that to signal
        the ingestor to stop you must set the stop event given to the
        constructor.
        """
        self.log.info('Stopping the video ingestor')
        for thread in self.cam_threads:
            thread.join()
        try:
            for camera in self.cameras:
                self.log.debug('Releasing camera %s', camera.name)
                camera.release()
        except AttributeError:
            pass  # If the video capture does not support release, that is okay

    def _run(self, cam_sn, camConfig):
        """Video stream ingestor run thread.
        """
        self.log.info('Capture thread started')
        camera = self._connect(cam_sn, camConfig)
        self.cameras.append(camera)
        while not self.stop_ev.is_set():
            try:
                ret, frame = camera.read()
                if not ret:
                    self.log.error(
                                'Failed to retrieve frame from camera %s',
                                camera)
                else:
                    self.on_data('video', (camera.name, frame,
                                 camera.config))
            except Exception as ex:
                self.log.error('Error while reading from camera: %s, \n%s',
                               camera, tb.format_exc())
                try:
                    self.log.info(
                            'Attempting to reconnect to camera %s', camera)
                    camera.release()
                    camera = self._connect(
                                camera.name, camera.config)
                    self.cameras = camera
                except Exception as ex:
                    self.log.error(
                            'Reconnect failed: \n%s', tb.format_exc())

            # Sleep for the poll interval.
            # Camera specific poll interval takes priority over the global
            # poll interval.
            if camera.config.get("poll_interval") is not None:
                time.sleep(camera.config.get("poll_interval"))
            elif self.poll_interval is not None:
                time.sleep(self.poll_interval)

    def _connect(self, cam_sn, config):
        """Private method to abstract connecting to a given camera type.

        Parameters
        ----------
        cam_sn : str or int
            Serial number of the camera, or integer for OpenCV
        config : None or dict
            Configuration for the camera, or None if no configuration is needed

        Exceptions
        ----------
        KeyError
            If the given configuration is missing a required value
        """
        cam = None
        if cam_sn is not None:
            self.log.info('Initializing OpenCV camera: %s', str(cam_sn))
            cam = cv2.VideoCapture(config["video_src"])
            cam_sn = '{}'.format(cam_sn)
        else:
            raise IngestorError('Unknown camera : {}'.format(cam_sn))

        return VideoCapture(cam_sn, config, cam)
