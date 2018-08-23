"""
Copyright (c) 2018 Intel Corporation.

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
"""

import re
import json
import time
import sys
import os
import requests
import cv2
import numpy as np
import argparse

path = os.path.abspath(__file__)
sys.path.append(os.path.join(os.path.dirname(path), "../"))
from DataBus import databus
from DataAgent.da_grpc.client.client import GrpcClient


class EtaDataSync:
    def ConvertClassfiertoJson(self, classifier_results):
        """
            This Method Converts classifier results into
            VIsual HMI Json Format
	
	"""
	metatext_dict = {}
        meta_data_hmi = {}
        meta_data_list = []
        meta_data_dict = {}
        try:
            print("Classifier Data Conversion Started.")
	    raw_text = classifier_results.replace("classifier_results ","")
            raw_defects_text = re.findall('(defects=.*]")', raw_text)
            meta_text = raw_text.replace(raw_defects_text[0] + ',','')
            defects_list = raw_defects_text[0].split("=")[1]
            defects_list_conversion =  json.loads(defects_list.decode('string-escape').strip('"'))

	    for x in meta_text.split(','):
                metatext_dict[x.split('=')[0]] = x.split('=')[1].replace('"',"")

            meta_data_dict["defects"] = defects_list_conversion 
            meta_data_dict["idx"] = metatext_dict["idx"]
            meta_data_dict["timestamp"] = float(metatext_dict["timestamp"].split(" ")[0])
            meta_data_dict["machine_id"] = metatext_dict["machine_id"]
            meta_data_dict["part_id"] = metatext_dict['part_id']
            meta_data_dict["image_id"] = metatext_dict["image_id"]
            meta_data_dict["cam_sn"] = metatext_dict["cam_sn"]
            meta_data_list.append(meta_data_dict)
            meta_data_hmi["part_id"] = metatext_dict['part_id']
            meta_data_hmi["meta-data"] = meta_data_list


            self.meta_data_hmi = meta_data_hmi
            self.meta_data_dict = meta_data_dict
            self.metatext_dict = metatext_dict
            final_json = json.dumps(meta_data_hmi,indent=4, separators=(',', ': '))
            print("Classifier Data Conversion Done Successfully")
 
        except Exception as e:
            print("Exception Occured in Text Conversion Module : "+ str(e))
            raise Exception

        return final_json

    def post_metadata(self, host, port, data):
	"""
            This Method post data to Visual HMI server.

        """
	posturi = 'http://{0}:{1}/rest/v1/part'.format(host,port)
	print("HMI Server URI : ", posturi)
        try:
            response = requests.post(posturi, json=data)
            if response.status_code != 200:
                print("HMI Failure Response Code : ", response.status_code)
                self.final_json = {}
                raise Exception
            else:
                print("HMI Success Response Code : ", response.status_code)
            self.final_json = {}
        except Exception as e:
            print("Exception Occured in Posting MetaData : ", str(e), data)
            self.final_json = {}
            self.metalist = []
            raise Exception

    def getBlob(self, key):
	"""
            GetBlob Method to get the data from GRPC server
            based on imagehandle returns the blob

        """
	try:
            # provide the hostname/ip addr of the m/c
            # where DataAgent module of ETA solution is running
	    grpc_host = self.config['databus_host']
	    client = GrpcClient(hostname=grpc_host)
            outputBytes = client.GetBlob(key)
            return outputBytes
        except Exception as e:
            print("Exception Occured in GetBlob Module : ", str(e))
            raise Exception

    def writeImageToDisk(self, keyname, filename):
	"""
            This will write Retrieved Image from Geblob
            to Visual HMI File System

        """
	try:
            blob = self.getBlob(keyname)
            print("Blob Successfully received for " + keyname)
            Frame = np.frombuffer(blob, dtype=np.uint8)
            reshape_frame = np.reshape(Frame, (1200,1920,3))
            cv2.imwrite(self.config["hmi_image_folder"]+filename+".png",reshape_frame,[cv2.IMWRITE_PNG_COMPRESSION,3]) 
            print("Image Conversion & Written Success as " + filename)

        except Exception as e:
            print("Exception Occured in WriteImagetoDisk Module : ", str(e))
            raise Exception


    def databus_callback(self, topic, msg):
	"""
            This is a Callback Method happens, Whenever
            Message gets Subscribed from Databus

        """
	try:
	    if 'end-of-part="END_OF_PART"' in msg:
                if self.args.savelocal is not False:
                    print("No Post Request Made to HMI as You are Running on Local Mode")
                else:
                    print("Started Sending Payload to HMI Server")
                    self.post_metadata(self.config["hmi_host"], self.config["hmi_port"], self.final_json)
                    self.final_json = {}
                    self.metalist = []
            else:
                converted_json = json.loads(self.ConvertClassfiertoJson(msg))
                self.writeImageToDisk(self.metatext_dict["ImgHandle"], self.meta_data_dict["image_id"])
                self.final_json['part_id'] = converted_json['part_id']
                self.metalist.append(converted_json['meta-data'][0])
                self.final_json['meta-data'] = self.metalist

        except Exception as e:
            print("Exceptin Occured in Call Back Module : ", str(e))
            raise Exception

    def parse_arg(self):
	"""
            Parsing the Commandline Arguments

        """
	
	ap = argparse.ArgumentParser()
        ap.add_argument("-c","--config", default="config.json",help="Please give the config file localtion")
        ap.add_argument("-local","--savelocal", default=False,help="This is to skip posting metaData to HMI Server & Writing Images Locally")
	return ap.parse_args()

    def main(self):
        self.final_json = {}
        self.metalist = []
        self.args = self.parse_arg()
        with open(self.args.config, 'r') as f:
            self.config = json.load(f)
        
        if self.args.savelocal is not False:
            self.config["hmi_image_folder"] = self.args.savelocal
        else:
            self.config["hmi_image_folder"] = self.config["hmi_image_folder"] + "/"
        
        contextConfig = { "endpoint": "opcua://" + self.config['databus_host'] + ":" + self.config['databus_port'], 
                          "direction": "SUB",
                          "name": "streammanager"
                        }
        try:
            etadbus = databus()
            etadbus.ContextCreate(contextConfig)
            topicConfig = {"name": "classifier_results", "type": "string"}
            etadbus.Subscribe(topicConfig, "START", self.databus_callback)
        except Exception as e:
            print("came to exception ", str(e))
            raise Exception


if __name__ == "__main__":

    etaDataSync = EtaDataSync()
    etaDataSync.main()
    while True:
        time.sleep(1)

