"""
Copyright (c) 2018 Intel Corporation.

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to
do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE,ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.
"""

import datetime
import json
import time
import sys
import os
import requests
import argparse
import logging
from threading import Condition, Thread
from algos.etr_utils.log import configure_logging, LOG_LEVELS
from ImageStore.client.py.client import GrpcImageStoreClient
from DataBusAbstraction.py.DataBus import databus

logging.basicConfig(level=logging.DEBUG,
                    format='%(asctime)s : %(levelname)s : \
                    %(name)s : [%(filename)s] :' +
                    '%(funcName)s : in line : [%(lineno)d] : %(message)s')

logging.getLogger("opcua").setLevel(logging.WARNING)
logger = logging.getLogger(__name__)

filePath = os.path.abspath(os.path.dirname(__file__))
ROOTCA_CERT = filePath + '/../cert-tool/Certificates/ca/ca_certificate.pem'
IM_CLIENT_CERT = filePath + '/../cert-tool/Certificates/imagestore/' + \
    'imagestore_client_certificate.pem'
IM_CLIENT_KEY = filePath + '/../cert-tool/Certificates/imagestore/' + \
    'imagestore_client_key.pem'


class EtaDataSync:

    def ConvertClassfierDatatoVisualHmiData(self, classifier_results):
        """
            This Method Converts classifier results into
            VIsual HMI Json Format
        """
        meta_data_dict = {}
        meta_data_hmi = {}
        meta_data_list = []
        try:
            meta_data_dict["defects"] =\
                json.loads(classifier_results["defects"])
            meta_data_dict["idx"] = classifier_results["idx"]
            meta_data_dict["timestamp"] =\
                float(classifier_results["timestamp"])
            meta_data_dict["machine_id"] = classifier_results["machine_id"]
            meta_data_dict["part_id"] = classifier_results['part_id']
            meta_data_dict["image_id"] = classifier_results["image_id"]
            meta_data_dict["cam_sn"] = classifier_results["cam_sn"]
            meta_data_list.append(meta_data_dict)
            meta_data_hmi["part_id"] = classifier_results['part_id']
            meta_data_hmi["meta-data"] = meta_data_list

        except Exception as e:
            print("Exception Occured in Text Conversion Module : " + str(e))
            raise Exception

        return meta_data_hmi

    def post_metadata(self, host, port, data):
        """
            This Method post data to Visual HMI server.
        """
        posturi = 'http://{0}:{1}/rest/v1/part'.format(host, port)
        logger.info("HMI Server URI : %s", posturi)
        try:
            response = requests.post(posturi, json=data)
            if response.status_code != 200:
                logger.error("HMI Failure Response Code : %s",
                             response.status_code)
            else:
                logger.info("HMI Success Response Code : %s",
                            response.status_code)
        except Exception as e:
            logger.error("Exception Occured: %s in Posting MetaData : %s",
                         str(e), data)
            raise

    def getBlob(self, key):
        """
            GetBlob Method to get the data from GRPC server
            based on imagehandle returns the blob
        """
        try:
            outputBytes = self.client.Read(key)
            return outputBytes
        except Exception as e:
            print("Exception: ", e)
            logger.error("Exception: %s", str(e))
            raise

    def writeImageToDisk(self, keyname, filename):
        """
            This will write Retrieved Image from Geblob
            to Visual HMI File System
        """
        try:
            blob = self.getBlob(keyname)
            logger.info("Blob Successfully received for key: %s", keyname)
            imgName = self.config["hmi_image_folder"] + filename + ".png"
            with open(imgName, "wb") as fp:
                fp.write(blob)
            logger.info("Image Conversion & Written Success as : %s", imgName)
        except Exception as e:
            logger.error("Exception: %s", str(e))
            raise

    def databus_callback(self, topic, msg):
        """
            Callback method called whenever message to the
            subscribed topic comes via Databus
        """
        try:
            logger.info("Message: %s", msg)
            databus_data = {}
            metalist = []
            classifier_results = json.loads(msg)
            classifier_visualhmi_dict =\
                self.ConvertClassfierDatatoVisualHmiData(
                    classifier_results)
            self.writeImageToDisk(classifier_results["ImgHandle"],
                                    classifier_results["image_id"])
            databus_data['part_id'] = classifier_visualhmi_dict['part_id']
            metalist.append(classifier_visualhmi_dict['meta-data'][0])
            databus_data['meta-data'] = metalist

            if self.args.savelocal == "yes":
                logger.info("No Post Request Made to HMI as You are\
                Running on Local Mode")
            else:
                logger.info("Started Sending Payload to HMI Server")
                self.post_metadata(self.config["hmi_host"],
                                    self.config["hmi_port"],
                                    databus_data)

        except Exception as e:
            logger.error("Exception: %s", str(e))
            raise

    def main(self, args):
        self.args = args
        with open(self.args.config, 'r') as f:
            self.config = json.load(f)

        # Client handle for image store
        self.client = GrpcImageStoreClient(IM_CLIENT_CERT, IM_CLIENT_KEY,
                                        ROOTCA_CERT,
                                        hostname=self.config['databus_host'])

        # Client handle for data bus abstraction
        filePath = os.path.abspath(os.path.dirname(__file__))
        certFile = filePath + "/../cert-tool/Certificates/opcua/" + \
            "opcua_client_certificate.der"
        privateFile = filePath + "/../cert-tool/Certificates/opcua/" + \
            "opcua_client_key.der"
        trustFile = filePath + "/../cert-tool/Certificates/ca/" + \
            "ca_certificate.der"

        print("Filepath: {}".format(filePath))
        contextConfig = {"endpoint": "opcua://" +
                         self.config['databus_host'] + ":" +
                         self.config['databus_port'],
                         "direction": "SUB",
                         "name": "streammanager",
                         "certFile": certFile,
                         "privateFile": privateFile,
                         "trustFile": trustFile}
        try:
            etadbus = databus()
            etadbus.ContextCreate(contextConfig)
            topicConfig = {"name": "classifier_results", "type": "string"}
            etadbus.Subscribe(topicConfig, "START", self.databus_callback)
        except Exception as e:
            logger.error("Exception: %s", str(e))
            raise
        return etadbus


def parse_arg():
    """
        Parsing the Commandline Arguments
    """
    ap = argparse.ArgumentParser()
    ap.add_argument("-c", "--config", default="config.json",
                    help="Please give the config file localtion")
    ap.add_argument("-local", "--savelocal", default="no",
                    help="This is to skip posting metaData\
        to HMI Server & Writing Images Locally")

    ap.add_argument('-log', choices=LOG_LEVELS.keys(), default='INFO',
                    help='Logging level (df: INFO)')

    ap.add_argument('-log-dir', dest='log_dir', default='logs',
                    help='Directory to for log files')

    return ap.parse_args()


if __name__ == "__main__":
    args = parse_arg()

    currentDateTime = str(datetime.datetime.now())
    listDateTime = currentDateTime.split(" ")
    currentDateTime = "_".join(listDateTime)
    logFileName = 'visual_hmi_eta_data_sync_' + currentDateTime + '.log'

    if not os.path.exists(args.log_dir):
        os.mkdir(args.log_dir)

    configure_logging(args.log.upper(), logFileName, args.log_dir)

    condition = Condition()
    try:
        etaDataSync = EtaDataSync().main(args)
    except Exception as e:
        logger.error("Exception: %s", str(e))
        if "refused" in e.message:
            logger.error("Retrying to establish connection with opcua \
                            server...")
            time.sleep(10)
    except KeyboardInterrupt:
        logger.error("Recevied Ctrl+C & VisualHMIClient App is shutting \
                        down now !!!")
        try:
            etaDataSync.ContextDestroy()
        except Exception as e:
            logger.error("Exiting..")
        condition.notifyAll()

    with condition:
        condition.wait()
        os._exit(1)
