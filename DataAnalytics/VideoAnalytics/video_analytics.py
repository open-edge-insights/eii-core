# Copyright (c) 2019 Intel Corporation.

# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

import os
import datetime
import queue
import signal
import argparse
import json
import etcd3
import logging
import zmq
import threading
from subscriber import Subscriber
from publisher import Publisher
from distutils.util import strtobool
from libs.base_classifier import load_classifier
from libs.ConfigManager.etcd.py.etcd_client import EtcdCli
from libs.log import configure_logging, LOG_LEVELS

# Etcd paths
CLASSIFIER_KEY_PATH = "/classifier"
CLASSIFIER_NAME_PATH = CLASSIFIER_KEY_PATH + "/classifier_name"


class VideoAnalytics:
    """Get the video frames from messagebus, classify and add the results
    back to the messagebus"""

    def __init__(self):
        """Constructor

        Returns
        -------
            VideoAnalytics object
        """
        self.log = logging.getLogger(__name__)
        self.profiling = bool(strtobool(os.environ['PROFILING']))
        self.dev_mode = bool(strtobool(os.environ["DEV_MODE"]))
        self.app_name = os.environ["APP_NAME"]
        conf = {
            "endpoint": "localhost:2379",
            "certFile": "",
            "keyFile": "",
            "trustFile": ""
        }

        self.etcd_cli = EtcdCli(conf)
        self.classifier_name = self.etcd_cli.GetConfig("/{0}{1}".format(
            self.app_name, CLASSIFIER_NAME_PATH))
        self.classifier_config = self.etcd_cli.GetConfig("/{0}{1}/{2}".format(
            self.app_name, CLASSIFIER_KEY_PATH, self.classifier_name))

        self.log.debug('classifier_name: {}, classifier_config: {}'.format(
            self.classifier_name, self.classifier_config))

        self.classifier_config = json.loads(self.classifier_config)

        self.etcd_cli.RegisterDirWatch("/{0}/".format(self.app_name)
            , self.onChangeConfigCB)

    def start(self):
        """ Start the Video Analytics.
        """
        self.log.info('=======Starting {}======='.format(self.app_name))
        self.classifier_input_queue = \
            queue.Queue(maxsize=self.classifier_config["input_queue_size"])

        self.classifier_output_queue = \
            queue.Queue(maxsize=self.classifier_config["output_queue_size"])

        self.publisher = Publisher(self.classifier_output_queue)
        self.publisher.start()

        self.classifier = load_classifier(self.classifier_name,
                                     self.classifier_config,
                                     self.classifier_input_queue,
                                     self.classifier_output_queue)
        self.classifier.start()

        self.subscriber = Subscriber(self.classifier_input_queue)
        self.subscriber.start()

    def stop(self):
        """ Stop the Video Analytics."""
        self.log.info('=======Stopping {}======='.format(self.app_name))
        self.subscriber.stop()
        self.classifier.stop()
        self.publisher.stop()

    def onChangeConfigCB(self, key, value):
        """
        Callback method to be called by etcd

        Parameters:
        ----------
        key: str
            etcd key
        value: str
            etcd value
        """
        # TODO: To be added:
        # 1. Add logic to control restart of filter/ingestor or publisher
        #    alone based on the config change instead of restarting all threads.
        # 2. Fix the issue of thread pool not shutting down properly
        # 3. Socket close in publisher isn't working, so the publish after
        #    config change isn't working (see address bind issue but publish is 
        #    happening and subscriber doesn't receive stuck)
        try:
            if "_classifier" in value:
                classifier_config = self.etcd_cli.GetConfig("/{0}{1}/{2}".format(
                    self.app_name, CLASSIFIER_KEY_PATH, value))
                self.classifier_name = value
                self.classifier_config = json.loads(classifier_config)
            elif "_classifier" in key:
                self.classifier_name = key
                self.classifier_config = json.loads(value)
        except:
            pass
        
        self.stop()
        self.start()


def parse_args():
    """Parse command line arguments
    """
    parser = argparse.ArgumentParser()

    parser.add_argument('--log', choices=LOG_LEVELS.keys(), default='INFO',
                        help='Logging level (df: INFO)')
    parser.add_argument('--log-dir', dest='log_dir', default='logs',
                        help='Directory to for log files')

    return parser.parse_args()


def main():    
    """Main method
    """
    # Parse command line arguments
    args = parse_args()

    currentDateTime = str(datetime.datetime.now())
    listDateTime = currentDateTime.split(" ")
    currentDateTime = "_".join(listDateTime)
    logFileName = 'videoanalytics_' + currentDateTime + '.log'

    # Creating log directory if it does not exist
    if not os.path.exists(args.log_dir):
        os.mkdir(args.log_dir)

    log = configure_logging(args.log.upper(), logFileName, args.log_dir,
                            __name__)

    va = VideoAnalytics()

    def handle_signal(signum, frame):
        log.info('Video Ingestion program killed...')
        va.stop()

    signal.signal(signal.SIGTERM, handle_signal)

    try:
        va.start()
    except Exception as ex:
        log.exception('Exception: {}'.format(ex))
        va.stop()


if __name__ == '__main__':
    main()


