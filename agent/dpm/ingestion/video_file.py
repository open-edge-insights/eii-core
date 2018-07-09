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

        cap = None
        try:
            video_file = os.path.expanduser(config['video_file'])
            self.filename = os.path.basename(video_file)
            self.loop_video = config.get('loop_video', False)
            self.poll_interval = config.get('poll_interval', None)
            if not os.path.exists(video_file):
                raise IngestorError(
                        'Video file does not exist: {}'.format(video_file))

            cap = cv2.VideoCapture(video_file)
            if not cap.isOpened():
                raise IngestorError(
                        'Failed to open video file: {}'.format(video_file))

            self.log.info('Reading frames from video file %s', video_file)
            while True:
                ret, frame = cap.read()

                if not ret:
                    break

                self.frames.append(frame)
                self.frame_count += 1
        except KeyError as e:
            raise IngestorError(
                    'Video file configuration missing key: {}'.format(e))
        finally:
            if cap is not None:
                cap.release()

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
        current_frame = 0

        while not self.stop_ev.is_set():
            self.on_data(
                    'video_file', (self.filename, self.frames[current_frame]))
            current_frame += 1

            if self.poll_interval is not None:
                time.sleep(self.poll_interval)

            if current_frame == self.frame_count:
                if self.loop_video:
                    self.log.debug('Video done playing, looping...')
                    current_frame = 0
                else:
                    self.log.debug('Video done playing, stopping...')
                    break

        self.log.info('Video file ingestor stopped')

