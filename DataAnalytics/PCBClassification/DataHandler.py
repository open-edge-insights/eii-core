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

import numpy as np
import time
import json

from ImageStore.client.py.client import GrpcImageStoreClient
from DataAgent.da_grpc.client.py.client_internal.client \
    import GrpcInternalClient
from Util.util import write_certs
import algos.dpm.classification.classifiers
from algos.dpm.classification.classifier_manager import ClassifierManager
from algos.dpm.config import Configuration
from DataIngestionLib.DataIngestionLib import DataIngestionLib, DataPoint


ROOTCA_CERT = '/etc/ssl/ca/ca_certificate.pem'
IM_CLIENT_CERT = '/etc/ssl/imagestore/imagestore_client_certificate.pem'
IM_CLIENT_KEY = '/etc/ssl/imagestore/imagestore_client_key.pem'

GRPC_CERTS_PATH = "/etc/ssl/grpc_int_ssl_secrets"
CLIENT_CERT = GRPC_CERTS_PATH + "/grpc_internal_client_certificate.pem"
CLIENT_KEY = GRPC_CERTS_PATH + "/grpc_internal_client_key.pem"
CA_CERT = GRPC_CERTS_PATH + "/ca_certificate.pem"


class DataHandler:
    """Handles the Point Data and runs classifier algos on stream of
       points and save the classified result back to influxDB.
    """

    def __init__(self, config_file, logger):
        self.config_file = config_file
        self.logger = logger

    def init(self):
        """Initialize the required object to call the classifier algos
         and save result to influx
        """
        self.config = Configuration(self.config_file)
        self._cm = ClassifierManager(
            self.config.machine_id, self.config.classification, self.logger)
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

        self.di = DataIngestionLib(self.logger)

    def handle_point_data(self, data_point_json):
        """Process the data point stream and persist the classified
           result back to influxdb
        This method reference from point method DataAnalytics/Classifier.py
        Main differences:
         1. it is called as callback by influx db subscription instead
            of kapacitor
         2. Received the data in json string format instead
            of Kapacitor managed object
         3. persist the classified data back to influxDB instead of giving
            back to kapacitor
         4. Form the classified data response in DataPoint object of
            DataIngestionLib/DataPoint.py instead of protobuf

         @:arg data_point_json: Point data in json string format
         :returns True if successfully classified and
                  saved the result otherwise None
        """

        self.logger.info("Received a data point: {0}".format(data_point_json))
        # convert json string to dict
        point = json.loads(data_point_json)

        # Create the data point object
        data_point = DataPoint(self.img_store, self.logger)
        data_point.set_measurement_name('classifier_results')

        img_handles = point["ImgHandle"]
        img_handles = img_handles.split(",")
        img_handle = img_handles[0]  # First one is in-mem

        if img_handle is None:
            self.logger.error('Input point doesnt have image')
            return

        img_height = point['Height']
        img_width = point['Width']
        img_channels = point['Channels']
        user_data = point['user_data']
        data = []
        # Reject the frame with user_data -1
        if user_data == -1:
            return

        try:
            frame = self.img_store.Read(img_handle)
        except Exception:
            self.logger.exception('Frame read failed : %s', img_handle)
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
            self.logger.info("Frame read unsuccessful.")

        if len(data) == 0:
            return

        # Sending in the list data for one part
        ret = self._cm._process_frames(self.classifier, data)

        for result in ret:
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

                # Send only the persistent handle to export
                if len(img_handles) > 1:
                    data_point.data_point['fields']['ImgHandle'] =\
                        img_handles[1]
                else:
                    # Persistent handle not available.
                    self.logger.info('Persistent handle not found. Sending \
                                     inmem handle to export')
                    data_point.data_point['fields']['ImgHandle'] =\
                        img_handles[0]

            data_point.add_fields('timestamp', time.time())

            # saving the data point into influxdb 'classifier_results'
            save_result = self.di.save_data_point(data_point)
            if save_result:
                self.logger.info("Successfully saved the point: {0}".format(
                    data_point.data_point))
                return True
            else:
                self.logger.error("Failed to saved the point: {0}".format(
                    data_point.data_point))
