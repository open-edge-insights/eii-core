"""
Copyright (c) 2019 Intel Corporation.

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

import os
import numpy as np
import time
import json
import threading
import queue
from concurrent.futures import ThreadPoolExecutor
import cv2

from ImageStore.client.py.client import GrpcImageStoreClient
from DataAgent.da_grpc.client.py.client_internal.client \
    import GrpcInternalClient
from Util.util import write_certs
import algos.dpm.classification.classifiers
from algos.dpm.classification.classifier_manager import ClassifierManager
from algos.dpm.config import Configuration
from DataIngestionLib.DataIngestionLib import DataIngestionLib, DataPoint
from distutils.util import strtobool

ROOTCA_CERT = '/etc/ssl/ca/ca_certificate.pem'
IM_CLIENT_CERT = '/etc/ssl/imagestore/imagestore_client_certificate.pem'
IM_CLIENT_KEY = '/etc/ssl/imagestore/imagestore_client_key.pem'

GRPC_CERTS_PATH = "/etc/ssl/grpc_int_ssl_secrets"
CLIENT_CERT = GRPC_CERTS_PATH + "/grpc_internal_client_certificate.pem"
CLIENT_KEY = GRPC_CERTS_PATH + "/grpc_internal_client_key.pem"
CA_CERT = GRPC_CERTS_PATH + "/ca_certificate.pem"
MAX_BUFFERS = 10


class DataHandler:
    """Handles the Point Data and runs classifier algos on stream of
       points and save the classified result back to influxDB.
    """

    def __init__(self, config_file, logger):
        self.config = Configuration(config_file)
        self.logger = logger
        self.frame_classify_ex = \
            ThreadPoolExecutor(
                max_workers=self.config.classification['max_workers'])
        self.frame_submitter_thread = threading.Thread(
            target=self.submit_frame)
        self.frame_queue = queue.Queue(maxsize=MAX_BUFFERS)
        self.frame_submitter_ev = threading.Event()

        self._cm = ClassifierManager(
            self.config.machine_id, self.config.classification, self.logger)
        classifier_name = next(iter(
            self.config.classification['classifiers']))
        self.classifier = self._cm.get_classifier(classifier_name)

        # Use getConfigInt to read cert and create file
        client = GrpcInternalClient(CLIENT_CERT, CLIENT_KEY, CA_CERT)
        self.resp = client.GetConfigInt("ImgStoreClientCert")

        # Write File
        file_list = [IM_CLIENT_CERT, IM_CLIENT_KEY]
        write_certs(file_list, self.resp)
        self.img_store = GrpcImageStoreClient(IM_CLIENT_CERT, IM_CLIENT_KEY,
                                              ROOTCA_CERT)
        self.di = DataIngestionLib(self.logger)
        self.frame_submitter_thread.start()

        self.profiling = bool(strtobool(os.environ['PROFILING']))
        self.logger.debug('profiling is {0}'.format(self.profiling))

    def submit_frame(self):
        while not self.frame_submitter_ev.is_set():
            pending = self.frame_classify_ex._work_queue.qsize()
            if pending < self.config.classification['max_workers']:
                frame, point = self.frame_queue.get()
                self.frame_classify_ex.submit(
                    self.process_frame, frame, point)
            else:
                self.logger.debug("Waiting for thread from pool...")
                time.sleep(0.01)

    def handle_video_data(self, data_point_json):
        """Process the data point stream and persist the classified
           result back to influxdb
           It just fetch the frame from image store and submit the frame to
           queue which is monitor by a thread to process it

         @:arg data_point_json: Point data in json string format
         :returns True if successfully classified and
                  saved the result otherwise None
        """

        self.logger.debug("Received a data point: {0}".format(data_point_json))

        if self.profiling is True:
            ts_va_entry = float(time.time()*1000)

        # convert json string to dict
        point = json.loads(data_point_json)

        # Create the data point object
        stream_name = point.get('Measurement')
        img_handles = point["ImgHandle"]
        img_handles = img_handles.split(",")
        img_handle = img_handles[0]  # First one is in-mem

        if img_handle is None:
            self.logger.error('[{0}]: Input point does not have image'.format(
                stream_name))
            return

        user_data = point['user_data']
        # Reject the frame with user_data -1
        if user_data == -1:
            return

        if self.profiling is True:
            point["ts_va_img_read_entry"] = float(time.time()*1000)

        try:
            frame = self.img_store.Read(img_handle)
        except Exception:
            self.logger.exception('[{0}]: Frame read failed : {1}'.format(
                stream_name, img_handle))
            return

        if self.profiling is True:
            point["ts_va_img_read_exit"] = float(time.time()*1000)

        if self.profiling is True:
            point["ts_va_entry"] = ts_va_entry

        if frame:
            self.frame_queue.put((frame, point))
        else:
            self.logger.info("[{0}]: Frame read unsuccessful.".format(
                stream_name))

    def process_frame(self, frame, point):
        """Classify the frame and save the classifier result to influxDB"""

        stream_name = point.get('Measurement')
        self.logger.debug("[{0}]: Processing frame...".format(stream_name))

        img_height = point['Height']
        img_width = point['Width']
        img_channels = point['Channels']
        if 'encoding' in point:
            encoding = point['encoding']
        else:
            encoding = None

        # Convert the buffer into np array.
        Frame = np.frombuffer(frame, dtype=np.uint8)
        if encoding is not None:
            reshape_frame = np.reshape(Frame, (Frame.shape))
            reshape_frame = cv2.imdecode(reshape_frame, 1)
        else:
            reshape_frame = np.reshape(Frame, (int(img_height),
                                               int(img_width),
                                               int(img_channels)))

        img_handles = point["ImgHandle"]
        img_handles = img_handles.split(",")
        img_handle = img_handles[0]  # First one is in-mem
        user_data = point['user_data']
        data = []

        # Call classification manager API with the tuple data
        val = (1, user_data, img_handle,
               ('camera-serial-number', reshape_frame))
        data.append(val)

        if len(data) == 0:
            return

        # Create the data point object
        data_point = DataPoint(self.img_store, self.logger)
        data_point.set_measurement_name(stream_name + '_results')

        if self.profiling is True:
            ts_va_analy_entry = float(time.time()*1000)

        # Sending in the list data for one part
        ret = self._cm._process_frames(self.classifier, data)

        if self.profiling is True:
            ts_va_analy_exit = float(time.time()*1000)

        for result in ret:

            if self.profiling is True:
                data_point.add_fields('ts_va_analy_entry',
                                      ts_va_analy_entry)
                data_point.add_fields('ts_va_analy_exit',
                                      ts_va_analy_exit)
                data_point.add_fields('ts_va_img_read_entry',
                                      point['ts_va_img_read_entry'])
                data_point.add_fields('ts_va_img_read_exit',
                                      point['ts_va_img_read_exit'])
                data_point.add_fields('ts_va_entry',
                                      point['ts_va_entry'])
                data_point.add_fields('ts_vi_fr_store_entry',
                                      point['ts_vi_fr_store_entry'])
                data_point.add_fields('ts_vi_fr_store_exit',
                                      point['ts_vi_fr_store_exit'])
                data_point.add_fields('ts_vi_influx_entry',
                                      point['ts_vi_influx_entry'])

            # Process the Classifier Results into DataPoint
            for k, v in result.items():
                if isinstance(v, str):
                    data_point.add_fields(k, v)
                elif isinstance(v, int) or isinstance(v, float):
                    data_point.add_fields(k, float(v))
                elif k == "defects":
                    data_point.add_fields(k, json.dumps(v))

                data_point.add_fields('Height', float(img_height))
                data_point.add_fields('Width', float(img_width))
                data_point.add_fields('Channels', float(img_channels))
                if encoding is not None:
                    data_point.add_fields('encoding', encoding)

                # Send only the persistent handle to export
                if len(img_handles) > 1:
                    data_point.data_point['fields']['ImgHandle'] =\
                        img_handles[1]
                else:
                    # Persistent handle not available.
                    self.logger.info('[{0}]: Persistent handle not found.'
                                     ' Sending inmem handle to'
                                     ' export'.format(stream_name))
                    data_point.data_point['fields']['ImgHandle'] =\
                        img_handles[0]

            data_point.add_fields('timestamp', time.time())

            # saving the data point into influxdb
            self.di.save_data_point(data_point)
