

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
import datetime
import json

from algos.dpm.config import Configuration
from DataHandler import DataHandler
from StreamSubLib.StreamSubLib import StreamSubLib
from Util.log import configure_logging, LOG_LEVELS
from distutils.util import strtobool
from Util.util import create_decrypted_pem_files, check_port_availability


CERTS_PATH = "/etc/ssl/grpc_int_ssl_secrets"
CA_CERT = CERTS_PATH + "/ca_certificate.pem"


class VideoAnalytics:
    """Subscribe to influxDB and register the callback"""

    def __init__(self, config, log, container_mode='no'):
        self.config = config
        self.log = log
        self.container_mode = container_mode
        self.devmode = bool(strtobool(os.environ['DEV_MODE']))

        self.log.info("=============== STARTING video_analytics \
            ===============")
        # load app configuration
        with open('config.json') as f:
            self.app_config = json.load(f)

        self.stream_sub_lib = StreamSubLib()

        if self.container_mode.lower() == 'yes':
            self.log.info("Running in container mode")
            # Wait for DA to be ready
            ret = check_port_availability(os.environ['DATA_AGENT_GRPC_SERVER'],
                                          os.environ['GRPC_INTERNAL_PORT'])
            if ret is False:
                log.error("DataAgent is not up. So Exiting...", exc_info=True)
                exit(-1)

            if not self.devmode:
                # Create cert
                src_files = [CA_CERT]
                files_to_decrypt = ["/etc/ssl/ca/ca_certificate.pem"]
                create_decrypted_pem_files(src_files, files_to_decrypt)
                self.stream_sub_lib.init()
            else:
                self.stream_sub_lib.init(dev_mode=True)
        else:
            self.log.info("Running in baremetal mode")
            try:
                if self.devmode:
                    self.stream_sub_lib.init(dev_mode=self.devmode)
                else:
                    self.stream_sub_lib.init(
                        cert_path=self.app_config['va_cert_path'],
                        key_path=self.app_config['va_key_path'])
            except Exception as e:
                self.log.error(e)
                os._exit(1)

            # self.stream_sub_lib.init(
            #  cert_path=self.app_config['va_cert_path'],
            #  key_path=self.app_config['va_key_path'])

        self.data_handler = DataHandler(self.config, self.log, self.devmode)

    def _get_streams(self):
        streams = []
        config_obj = Configuration(self.config)
        ingestors = config_obj.data_ingestion_manager['ingestors']
        if ingestors.get('video') and\
                ingestors['video'].get('streams') and\
                ingestors['video']['streams'].get('capture_streams'):
            # if config is factory_multi_cam.json
            capture_streams = ingestors.get('video').get('streams').get(
                'capture_streams')
            streams.extend(capture_streams.keys())
        else:
            # if config is factory_video_file.json
            streams.append(self.app_config['stream_measurement_name'])

        return streams

    def main(self):
        """Subscribes to influxDB streams based on config and register
           point data handler as a callback
        """

        streams = self._get_streams()
        self.log.info("Subscribing to streams: {0}".format(streams))
        for stream in streams:
            self.stream_sub_lib.Subscribe(
                stream, self.data_handler.handle_video_data)


def parse_args():
    """Parse command line arguments"""

    parser = argparse.ArgumentParser()
    parser.add_argument('--config', default='config.json',
                        help='JSON configuration file')

    parser.add_argument('--log', dest='log', choices=LOG_LEVELS.keys(),
                        default='DEBUG', help='Logging level (df: DEFAULT)')

    parser.add_argument('--log-name', dest='log_name',
                        default='videoAnalytics', help='Logfile name')

    parser.add_argument('--log-dir', dest='log_dir', default='logs',
                        help='Directory for log files')

    parser.add_argument('--container-mode', dest='container_mode',
                        default='no',
                        help='run in baremetal or container mode')

    return parser.parse_args()


if __name__ == "__main__":
    args = parse_args()

    if not os.path.exists(args.log_dir):
        os.mkdir(args.log_dir)

    current_datetime = str(datetime.datetime.now())
    list_datetime = current_datetime.split(" ")
    current_date_time = "_".join(list_datetime)
    log_file_name = args.log_name + '_' + current_date_time + '.log'

    log = configure_logging(args.log.upper(), log_file_name,
                            args.log_dir, __name__)

    app = VideoAnalytics(args.config, log, args.container_mode)
    app.main()
