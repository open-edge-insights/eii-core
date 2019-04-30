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

import argparse
import os
import sys
import json
import logging as log

from distutils.util import strtobool
# IMPORT the library to read from IES
from StreamSubLib.StreamSubLib import StreamSubLib
from ImageStore.internalClient.py.client import GrpcImageStoreInternalClient


class sampleApp:
    """ A sample app using IES framework libraries
    to perform custom analytics operation. """

    def __init__(self, devMode):
        self.devMode = devMode

        try:
            self.strmSubscrbr = StreamSubLib()
            # Pass the mode
            self.strmSubscrbr.init(dev_mode=devMode)
            self.img_store = GrpcImageStoreInternalClient(dev_mode=devMode)
        except Exception as e:
            log.error(e)
            sys.exit(1)

        self.strmSubscrbr.Subscribe("stream1", self.sampleCb)

    def sampleCb(self, pointData):
        """ It process the point recieved from influx DB subscription"""

        # convert json string to dict
        influxPoint = json.loads(pointData)

        # Extract the relevant information for reading the frame.
        stream_name = influxPoint.get('Measurement')
        img_handles = influxPoint["ImgHandle"]
        img_handles = img_handles.split(",")
        img_handle = img_handles[0]  # First one is in-mem

        try:
            frame = self.img_store.Read(img_handle)
            # Let's see the frame.
            self.analyzeFrame(frame, influxPoint)

        except Exception:
            log.exception('[{0}]: Failed to process frame : {1}'.format(
                stream_name, img_handle))
            return

    def analyzeFrame(self, frame, metaData):
        """ This function can be filled with analytics logic"""
        log.info("The Meta-data from StreamSubLib() is {0}".format(metaData))
        log.info("The FRAME IS {}".format(frame))


def parse_args():
    """Parse command line arguments"""

    parser = argparse.ArgumentParser()

    parser.add_argument('--dev-mode', dest='dev_mode',
                        default='false',
                        help='run in secured or non-secured mode')
    return parser.parse_args()


# This is a sample program which shows how to use the framework
# provided library function and plug a python based video analytics
# logic. It works in secured as well as non-secured mode.
if __name__ == "__main__":
    args = parse_args()

    dev_mode = bool(strtobool(args.dev_mode))
    app = sampleApp(dev_mode)
