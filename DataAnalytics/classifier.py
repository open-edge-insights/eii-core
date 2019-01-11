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

import argparse
import numpy as np
import time
import json
import os
from ImageStore.client.py.client import GrpcImageStoreClient
from DataAgent.da_grpc.client.py.client_internal.client \
    import GrpcInternalClient
from Util.util import write_certs

from Util.log import configure_logging, LOG_LEVELS

# The kapacitor source code is needed for the below imports
from kapacitor.udf.agent import Agent, Handler, Server
from kapacitor.udf import udf_pb2
from algos.dpm.classification.classifier_manager import ClassifierManager
from algos.dpm.config import Configuration


server_addr = "/tmp/classifier"

TIME_MULTIPLIER_NANO = 1000000000

ROOTCA_CERT = '/etc/ssl/grpc_int_ssl_secrets/ca_certificate.pem'
IM_CLIENT_CERT = '/etc/ssl/imagestore/imagestore_client_certificate.pem'
IM_CLIENT_KEY = '/etc/ssl/imagestore/imagestore_client_key.pem'

GRPC_CERTS_PATH = "/etc/ssl/grpc_int_ssl_secrets"
CLIENT_CERT = GRPC_CERTS_PATH + "/grpc_internal_client_certificate.pem"
CLIENT_KEY = GRPC_CERTS_PATH + "/grpc_internal_client_key.pem"
CA_CERT = GRPC_CERTS_PATH + "/ca_certificate.pem"


# Runs ML algo on stream of points and return result back to kapacitor.
class ConnHandler(Handler):
    def __init__(self, agent, config_file, logger):
        self._agent = agent
        self.config_file = config_file
        self._cm = None
        self.logger = logger

    def info(self):
        response = udf_pb2.Response()
        response.info.wants = udf_pb2.STREAM
        response.info.provides = udf_pb2.STREAM
        return response

    def classifier_init(self):
        self.config = Configuration(self.config_file)
        self._cm = ClassifierManager(
                self.config.machine_id, self.config.classification)
        classifier_name = next(iter(self.config.classification['classifiers']))
        self.classifier = self._cm.get_classifier(classifier_name)

        # Use getConfigInt to read cert and create file
        client = GrpcInternalClient(CLIENT_CERT, CLIENT_KEY, CA_CERT)
        self.resp = client.GetConfigInt("ImgStoreClientCert")

        # Write File
        file_list = [IM_CLIENT_CERT, IM_CLIENT_KEY]
        write_certs(file_list, self.resp)
        self.img_store = GrpcImageStoreClient(IM_CLIENT_CERT, IM_CLIENT_KEY,
                                              ROOTCA_CERT)

    def init(self, init_req):
        self.classifier_init()
        response = udf_pb2.Response()
        response.init.success = True
        # self.logger.info("INIT CALLBACK of UDF")
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
        # self.logger.info("Recieved a point")
        response = udf_pb2.Response()


        img_handles = point.fieldsString["ImgHandle"]
        img_handles = img_handles.split(",")
        img_handle = img_handles[0] # First one is in-mem

        if img_handle is None:
            self.logger.error('Input point doesnt have image')
            return

        img_height = point.fieldsInt['Height']
        img_width = point.fieldsInt['Width']
        img_channels = point.fieldsInt['Channels']
        user_data = point.fieldsInt['user_data']
        data = []
        # Reject the frame with user_data -1
        if user_data == -1:
            return

        try:
            frame = self.img_store.Read(img_handle)
        except Exception:
            self.logger.error('Frame read failed : %s', img_handle)
            return

        if frame is not None:
            # Convert the buffer into np array.
            Frame = np.frombuffer(frame, dtype=np.uint8)
            reshape_frame = np.reshape(Frame, (img_height,
                                       img_width, img_channels))

            # Call classification manager API with the tuple data
            val = (1, user_data, img_handle,
                   ('camera-serial-number', reshape_frame))
            data.append(val)
        else:
            logger.info("Frame read unsuccessful.")

        if len(data) == 0:
            return

        # Sending in the list data for one part
        ret = self._cm._process_frames(self.classifier, data)

        for result in ret:
            # Process the Classifier Results into Response Structure
            for k, v in result.items():
                if isinstance(v, str):
                    response.point.fieldsString[k] = v
                elif isinstance(v, int) or isinstance(v, float):
                    response.point.fieldsDouble[k] = v
                elif k == "defects":
                    response.point.fieldsString[k] = json.dumps(v)

                response.point.fieldsDouble['Height'] = img_height
                response.point.fieldsDouble['Width'] = img_width
                response.point.fieldsDouble['Channels'] = img_channels
                # Send only the persistent handle to export
                if len(img_handles) > 1:
                    response.point.fieldsString["ImgHandle"] = img_handles[1]
                else:
                    # Persistent handle not available.
                    self.logger.info('Persistent handle not found. Sending \
                                     inmem handle to export')
                    response.point.fieldsString["ImgHandle"] = img_handles[0]

            response.point.time = int(time.time()*TIME_MULTIPLIER_NANO)
            self._agent.write_response(response, True)

    def end_batch(self, end_req):
        raise Exception("not supported")


class accepter(object):

    def __init__(self, config_file, logger):
        self.config_file = config_file
        self._count = 0
        self.logger = logger

    def accept(self, conn, addr):
        self._count += 1
        # Create an agent
        agent = Agent(conn, conn)
        # Create a handler and pass it an agent so it can write points
        h = ConnHandler(agent, self.config_file, self.logger)
        # Set the handler on the agent
        agent.handler = h

        self.logger.info("Starting kapacitor agent in socket mode")
        agent.start()
        agent.wait()
        self.logger.info("Classifier UDF stopped.")


def parse_args():
    """Parse command line arguments
    """
    parser = argparse.ArgumentParser()

    parser.add_argument('--config', default='factory.json',
                        help='JSON configuration file')

    parser.add_argument('--log', choices=LOG_LEVELS.keys(), default='DEBUG',
                        help='Logging level (df: DEFAULT)')

    parser.add_argument('--log-name', help='Logfile name')

    parser.add_argument('--log-dir', dest='log_dir', default='logs',
                        help='Directory to for log files')

    return parser.parse_args()


if __name__ == '__main__':

    args = parse_args()

    if not os.path.exists(args.log_dir):
        os.mkdir(args.log_dir)

    logger = configure_logging(args.log.upper(), args.log_name,
                               args.log_dir, __name__)

    if os.path.exists(server_addr):
        logger.info("Deleting %s", server_addr)
        os.remove(server_addr)

    server = Server(server_addr, accepter(args.config, logger))
    logger.info("Started the server By Agent")

    server.serve()
