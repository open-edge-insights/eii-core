"""Video file ingestor.
"""
import threading
import time
import logging
import cv2
import os
from . import IngestorError


class Ingestor:
    """Ingestor for ingesting a video file and repeatedly uses the frames from 
    that video to send through the data pipeline.
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
        self.frames = []
        self.frame_count = 0
        self.th = None
        self.cap = None
        try:
            self.video_file = os.path.expanduser(config['video_file'])
            self.filename = os.path.basename(self.video_file)

            self.loop_video = config.get('loop_video', False)
            print('config:', config)
            
            self.poll_interval = config.get('poll_interval', None)
            if not os.path.exists(self.video_file):
                raise IngestorError(
                    'Video file does not exist: {}'.format(self.video_file))

            self.cap = cv2.VideoCapture(self.video_file)
            if not self.cap.isOpened():
                raise IngestorError(
                    'Failed to open video file: {}'.format(self.video_file))

        except KeyError as e:
            raise IngestorError(
                    'Video file configuration missing key: {}'.format(e))

    def start(self):
        """Start the ingestor.

        Exceptions
        ----------
        IngestorError
            If the ingestor has already been started
        """
        if self.th is not None:
            raise IngestorError('Already started')
        self.log.info('Starting ingestor thread')
        self.th = threading.Thread(target=self._run)
        self.th.start()

    def join(self):
        """Blocks until the ingestor has stopped running. Note that to signal
        the ingestor to stop you must set the stop event given to the
        constructor.
        """
        self.th.join()
        self.log.debug('Video file ingestor thread joined')
    
    def _run(self):
        """Video file ingestor run thread.
        """
        self.log.info('Video file ingestor running')
        
        while not self.stop_ev.is_set():
            ret, frame = self.cap.read()
            self.on_data('video_file', (self.filename, frame))
            
            if self.poll_interval is not None:
                time.sleep(self.poll_interval)

            if not ret:
                if self.loop_video:
                    self.log.debug('Video done playing, looping...')
                    self.cap.release()
                    self.cap = cv2.VideoCapture(self.video_file)
                    if not self.cap.isOpened():
                        raise IngestorError(
                            'Failed to open video file: {}'.format(
                                self.video_file))
                else:
                    self.log.debug('Video done playing, stopping...')
                    break
            
        self.log.info('Video file ingestor stopped')

