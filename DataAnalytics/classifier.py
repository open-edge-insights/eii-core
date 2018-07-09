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

    def init(self, results_queue):
        self.config = Configuration(self.config_file)
        self.db = DatabaseAdapter(self.config.machine_id, self.config.database)
        self.storage = LocalStorage(self.config.storage)
        self._cm = ClassifierManager(
                self.config.machine_id, self.config.classification,
                self.storage, self.db)
        self.classifier = self._cm.get_classifier("yumei")
        self.img_store = ImageStore()
        self.results_queue = results_queue
        self.th.start()
        self.storage.start()

    def add_point(self, point):
        self.queue.put(point)

    def process_point(self):
        logger.info('Process point thread')
        while(True):
            data = self.queue.get(block=True)
            point = json.loads(data)
            img_handle = point["ImgHandle"]
            img_height = point['Height']
            img_width = point['Width']
            img_channels = point['Channels']
            ret, frame = self.img_store.read(img_handle)
            if ret is True:
                # Convert the buffer into np array.
                Frame = np.frombuffer(frame, dtype=np.uint8)
                reshape_frame = np.reshape(Frame, (img_height,
                                                   img_width, img_channels))
                # Call classification manager API with the tuple data
                user_data = point["user_data"]
                data = [(1, user_data,
                         ('camera-serial-number', reshape_frame))]
                ret = self._cm._process_frames(self.classifier, data)
                ret['ImgHandle'] = img_handle
                self.results_queue.put(json.dumps(ret))
            else:
                # TO Do : Log Error
                pass


class ResultsHandler():
    def __init__(self, connection):
        self.conn = connection
        self.th_results = th.Thread(target=self.process_classified_results)
        self.classified_results = queue.Queue()
        self.th_results.start()

    def get_results_queue(self):
        return self.classified_results

    def process_classified_results(self):
        logger.info('Process Classified Results Thread...')
        while(True):
            data = self.classified_results.get(block=True)
            result = (data.encode('utf-8'))
            self.conn.send(result)


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
    results_handler = ResultsHandler(connection)
    udf.init(results_handler.get_results_queue())

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
