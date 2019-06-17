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

import logging
import json


class ConfigError(Exception):
    """Exception raised when there is a configuration error
    """
    pass


class Configuration:
    """Factory agent configuration object
    """
    def __init__(self, configfile):
        """Constructor

        Parameters
        ----------
            configfile - Configuration file to read

        Exceptions
        ----------
        OSError
            If the configuration file does not exist
        KeyError
            If the configuration is missing keys.
        """
        self.log = logging.getLogger(__name__)
        self.log.info('Loading configuration file %s', configfile)
        with open(configfile, 'r') as f:
            config = json.load(f)

        # ID of the gateway
        self.machine_id = config['machine_id']
        self.trigger_threads = config.get('trigger_threads', None)
        self.queue_size = config.get('queue_size', None)
        self.log_file_size = config.get('log_file_size', None)

        # Reading classifier configuration
        self.classification = config['classification']
        self.data_ingestion_manager = config['data_ingestion_manager']
        self.triggers = config.get('triggers', None)
        self.rsync_service = config.get('rsync_service', None)
