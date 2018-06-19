import sys
import numpy as np
import logging
import socket
import os
import json
import queue
import threading as th
from ImageStore.py.imagestore import ImageStore

from agent.dpm.classification.classifier_manager import ClassifierManager
from agent.dpm.config import Configuration
from agent.db import DatabaseAdapter
from agent.dpm.storage import LocalStorage

server_address = '/tmp/classifier'
logger = logging.getLogger(__name__)
logging.basicConfig(level=logging.DEBUG,
                    format='%(asctime)s %(levelname)s:%(name)s: %(message)s')


class classifier_udf ():
    def __init__(self, config_file):
        self.config_file = config_file
        self._cm = None
        self.queue = queue.Queue()
        self.th = th.Thread(target=self.process_point)

    def init(self):
        self.config = Configuration(self.config_file)
        self.db = DatabaseAdapter(self.config.machine_id, self.config.database)
        self.storage = LocalStorage(self.config.storage)
        self._cm = ClassifierManager(
                self.config.machine_id, self.config.classification,
                self.storage, self.db)
        self.classifier = self._cm.get_classifier("yumei")
        self.img_store = ImageStore()
        self.th.start()
        self.storage.start()

    def add_point(self, point):
        self.queue.put(point)

    def process_point(self):
        logger.info('Proess point thread')
        while(True):
            data = self.queue.get(block=True)
            point = json.loads(data)

            # logger.info('Process point :', point)

            frame_handle = point["ImgHandle"]
            img_height = point['Height']
            img_width = point['Width']
            img_channels = point['Channels']
            img_handles = frame_handle.split(',')
            for idx in range(len(img_handles)):
                ret, frame = self.img_store.read(img_handles[idx])
                if ret is True:
                    # Convert the buffer into np array.
                    Frame = np.frombuffer(frame, dtype=np.uint8)
                    reshape_frame = np.reshape(Frame, (img_height,
                                               img_width, img_channels))
                    # Call classification manager API with the tuple data
                    user_data = [-1, 0]
                    data = [(1, user_data,
                             ('camera-serial-number', reshape_frame))]
                    self._cm._process_frames(self.classifier, data)
                else:
                    # TO Do : Log Error
                    pass


if __name__ == '__main__':

    if (len(sys.argv) < 2):
        logger.error("Usage : python3 classifier.py factory.json")
        sys.exit(1)

    config_file = sys.argv[1]
    udf = classifier_udf(config_file)
    # Make sure the socket does not already exist
    try:
        os.unlink(server_address)
    except OSError:
        if os.path.exists(server_address):
            raise

    sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
    sock.bind(server_address)
    sock.listen(1)

    # Wait for a connection
    logger.info('Waiting for a connection')
    connection, client_address = sock.accept()
    udf.init()

    try:
        logger.info('Connection received')

        while True:
            data = connection.recv(1024)
            data = data.decode('utf8')
            points = data.split("}")
            for point in points:
                if point != '':
                    udf.add_point(point + '}')

    finally:
        connection.close()
