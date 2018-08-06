import sys
import argparse
import datetime
import numpy as np
import logging
import socket
import time
import json
import queue
import os
import threading as th
from ImageStore.py.imagestore import ImageStore
from agent.etr_utils.log import configure_logging, LOG_LEVELS
import numpy as np

# The kapacitor source code is needed for the below imports
from kapacitor.udf.agent import Agent, Handler, Server
from kapacitor.udf import udf_pb2


from agent.dpm.classification.classifier_manager import ClassifierManager
from agent.dpm.config import Configuration
from agent.db import DatabaseAdapter
from agent.dpm.storage import LocalStorage
server_addr = "/tmp/classifier"

logging.basicConfig(level=logging.DEBUG,
                    format='%(asctime)s : %(levelname)s : %(name)s : [%(filename)s] :' +
                    '%(funcName)s : in line : [%(lineno)d] : %(message)s')
logger = logging.getLogger()

TIME_MULTIPLIER_MICRO = 1000000

# Runs ML algo on stream of points and return result back to kapacitor.
class ConnHandler(Handler):
    def __init__(self, agent,config_file):
        self._agent = agent
        self.config_file = config_file
        self._cm = None
        self.data = []

    def info(self):
        response = udf_pb2.Response()
        response.info.wants = udf_pb2.STREAM
        response.info.provides = udf_pb2.STREAM
        return response

    def classifier_init(self):
        self.config = Configuration(self.config_file)
        self.db = DatabaseAdapter(self.config.machine_id, self.config.database)
        self.storage = LocalStorage(self.config.storage)
        self._cm = ClassifierManager(
                self.config.machine_id, self.config.classification,
                self.storage, self.db)
        self.classifier = self._cm.get_classifier("yumei")
        self.img_store = ImageStore()
        self.storage.start()

    def init(self, init_req):
        response = udf_pb2.Response()
        response.init.success = True
        #print("INIT CALLBACK of UDF", file=sys.stderr)
        return response

    def snapshot(self):
        response = udf_pb2.Response()
        response.snapshot.snapshot = b''
        return response

    def restore(self, restore_req):
        response = udf_pb2.Response()
        response.restore.success = False
        response.restore.error = 'not implemented'
        return response

    def begin_batch(self, begin_req):
        raise Exception("not supported")

    def point(self, point):
        #print("Recieved a point", file=sys.stderr)
        response = udf_pb2.Response()

        if(point.fieldsInt['Width'] != 0):
            img_handle = point.fieldsString["ImgHandle"]
            img_height = point.fieldsInt['Height']
            img_width = point.fieldsInt['Width']
            img_channels = point.fieldsInt['Channels']
            ret, frame = self.img_store.read(img_handle)
            if frame != None:
                # Convert the buffer into np array.
                Frame = np.frombuffer(frame, dtype=np.uint8)
                reshape_frame = np.reshape(Frame, (img_height,
                                                img_width, img_channels))
                # Call classification manager API with the tuple data
                user_data = point.fieldsInt['user_data']
                val = (1, user_data, img_handle,
                            ('camera-serial-number', reshape_frame))
                self.data.append(val)
                return


        # Sending in the list data for one part
        ret = self._cm._process_frames(self.classifier, self.data)

        for result in ret:
            # Process the Classifier Results into Response Structure
            for k, v in result.items():
                if isinstance(v, float):
                    response.point.fieldsDouble[k] = v
                elif isinstance(v, str):
                    response.point.fieldsString[k] = v
                elif isinstance(v, int):
                    response.point.fieldsInt[k] = v
                elif k == "defects":
                    for index,defect in enumerate(v, start=1):
                        d_k = k+'_'+str(index)
                        response.point.fieldsString[d_k] = json.dumps(defect)

            response.point.time = int(time.time()*TIME_MULTIPLIER_MICRO)
            self._agent.write_response(response, True)

        # Sending the end of part signal
        self.data = []
        response = udf_pb2.Response()
        response.point.fieldsString["end-of-part"] = "END_OF_PART"
        response.point.time = int(time.time()*TIME_MULTIPLIER_MICRO)
        self._agent.write_response(response, True)

    def end_batch(self, end_req):
        raise Exception("not supported")

class accepter(object):

    def __init__(self,config_file):
       self.config_file = config_file
       self._count = 0

    def accept(self, conn, addr):
        self._count += 1
        # Create an agent
        agent = Agent(conn, conn)
        # Create a handler and pass it an agent so it can write points
        h = ConnHandler(agent, self.config_file)
        h.classifier_init()
        # Set the handler on the agent
        agent.handler = h

        print("Starting kapacitor agent in socket mode", file=sys.stderr)
        agent.start()
        agent.wait()
        print("Ending kapacitor agent in socket mode", file=sys.stderr)

def parse_args():
    """Parse command line arguments
    """
    parser = argparse.ArgumentParser()

    parser.add_argument('--config', default='factory.json',
                        help='JSON configuration file')

    parser.add_argument('--log', choices=LOG_LEVELS.keys(), default='INFO',
                        help='Logging level (df: INFO)')

    parser.add_argument('--log-dir', dest='log_dir', default='logs',
                        help='Directory to for log files')

    return parser.parse_args()


if __name__ == '__main__':

    args = parse_args()

    currentDateTime = datetime.datetime.now()
    logFileName = 'dataAnalytics_' + str(currentDateTime) + '.log'

    if not os.path.exists(args.log_dir):
        os.mkdir(args.log_dir)

    configure_logging(args.log.upper(), logFileName , args.log_dir)

    server = Server(server_addr, accepter(args.config))
    logger.info("Started the server By Agent")

    server.serve()
