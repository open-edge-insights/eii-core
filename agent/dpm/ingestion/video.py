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

"""Video ingestor
"""
import traceback as tb
import logging
import threading
import time
import cv2
import os
from . import IngestorError

try:
    import basler_video_capture as bvc
    bvc.initialize()
    basler_supported = True
except ImportError:
    basler_supported = False

try:
    import dahua_video_capture as dvc
    dahua_supported = True
except ImportError:
    dahua_supported = False


class VideoCapture:
    """Simple wrapper around external video capture objects. 
    """
    def __init__(self, cam_type, name, config, cap):
        """Constructor

        Parameters
        ----------
        cam_type : str
            Type of the camera
        name : str
            Name of the video capture
        config : dict or None
            Configuration of the camera, or None if it does not have one
        cap : Object
            Video capture object
        """
        self.cam_type = cam_type
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

    Currently the two supported streams are opencv, and basler. The configuration
    for the opencv stream is as follows:

        {
            "capture_streams": <LIST OF PARAMTERS FOR VideoCapture>
        }

    The opencv stream will open each of the capture streams as VideoCapture 
    objects. What ever the first parameter of the cv2.VideoCapture object
    supports can be specified in the array.

    The basler configuration is as follows:

        {
            "<CAMERA SERIAL NUMBER>": {
                "gain": <INT: (Optional) Gain to setting for the camera>,
                "exposure": <INT: (Optional) Exposure to setting for the camera>
            }
        }
    
    Each key in the configuration object for Basler is the serial number of a
    camera for the ingestor to ingest data from. The value is another dictionary
    object with two optional keys, gain and exposure. If these keys are not
    specified, then the camera will use whatever the default value is set to
    on the camera for the gain and exposure.

    An error will be raised if Basler cameras are not supported, i.e. the needed
    library cannot be imported.
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
        self.poll_interval = config['poll_interval']
        self.cameras = []
        streams = config['streams']

        if 'basler' in streams:
            if not basler_supported:
                raise IngestorError(
                        'Basler cameras are not supported, missing lib')
            self.log.info('Loading basler camera streams')
            cameras = bvc.enumerate_devices()

            self.log.debug('Enumerated Basler cameras: %s', cameras)

            for (sn, conf) in streams['basler'].items():
                if sn not in cameras:
                    raise IngestorError('Cannot find camera: {}'.format(sn))
                self.cameras.append(self._connect('basler', sn, conf))

        if 'opencv' in streams:
            self.log.info('Loading OpenCV streams')
            cap_streams = streams['opencv']['capture_streams']
            for stream in cap_streams:
                self.cameras.append(self._connect('opencv', stream, None))

        if 'dahua' in streams:
            if not dahua_supported:
                raise IngestorError(
                        'Dahua cameras are not supported, missing lib')
            self.log.info('Loading Dahua streams')
            num_cameras = streams['dahua']['number_of_cameras']
            found = dvc.enumerate_devices()

            if len(found) < num_cameras:
                raise IngestorError('Found too few dahua cameras')

            self.log.debug('Enumerated dahua cameras: %s', found)

            # Connect to the first N cameras, where N is the num_cameras
            for dev in found[:num_cameras]:
                self.log.debug('Opening dahua device: %s', dev)
                self.cameras.append(self._connect('dahua', dev, None))

        if not self.cameras:
            raise IngestorError('No capture streams for the video ingestor')

        self.th = threading.Thread(target=self._run)

    def start(self):
        """Start the ingestor.
        """
        self.th.start()

    def join(self):
        """Blocks until the ingestor has stopped running. Note that to signal
        the ingestor to stop you must set the stop event given to the
        constructor.
        """
        self.log.info('Stopping the video ingestor')
        self.th.join()
        try:
            for camera in self.cameras:
                self.log.debug('Releasing camera %s', camera.name)
                camera.release()
        except AttributeError:
            pass  # If the video capture does not support release, that is okay

    def _run(self):
        """Video stream ingestor run thread.
        """
        self.log.info('Capture thread started')
        while not self.stop_ev.is_set():
            for (i, camera) in enumerate(self.cameras):
                try:
                    ret, frame = camera.read()
                    if not ret:
                        self.log.error(
                                'Failed to retrieve frame from camera %s', 
                                camera)
                    else:
                        self.on_data('video', (camera.name, frame,))
                except:
                    self.log.error('Error while reading from camera: %s, \n%s', 
                        camera, tb.format_exc())
                    try:
                        self.log.info(
                                'Attempting to reconnect to camera %s', camera)
                        camera.release()
                        camera = self._connect(
                                camera.cam_type, camera.name, camera.config)
                        self.cameras[i] = camera
                    except:
                        self.log.error(
                                'Reconnect failed: \n%s', tb.format_exc())

            time.sleep(self.poll_interval)

    def _connect(self, cam_type, cam_sn, config):
        """Private method to abstract connecting to a given camera type.

        Parameters
        ----------
        cam_type : str
            Type of the camera (basler, dahua, or opencv)
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
        if cam_type == 'basler':
            self.log.info('Initializing basler camera %s', cam_sn)
            try:
                cam = bvc.BaslerVideoCapture(cam_sn)
            except:
                for i in range(5):
                    try:
                        cam = bvc.BaslerVideoCapture(cam_sn)
                        break
                    except:
                        self.log.error("Camera not responding, retrying %d", i+1)
                if cam is None:
                    os._exit(1)
            if 'gain' in config:
                self.log.info('Setting gain to %d on camera %s', 
                        config['gain'], cam_sn)
                cam.set_gain(config['gain'])
            if 'exposure' in config:
                self.log.info('Setting exposure to %d on camera %s', 
                        config['exposure'], cam_sn)
                cam.set_exposure(config['exposure'])
        elif cam_type == 'opencv':
            self.log.info('Initializing OpenCV camera: %s', str(cam_sn))
            cam = cv2.VideoCapture(cam_sn)
            cam_sn = 'opencv: {}'.format(cam_sn)
        elif cam_type == 'dahua':
            self.log.info('Initializing Dahua camera: %s', cam_sn)
            cam = dvc.DahuaVideoCapture(dev)
        else:
            raise IngestorError('Unknown camera type: {}'.format(cam_type))

        return VideoCapture(cam_type, cam_sn, config, cam)

