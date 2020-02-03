# Copyright (c) 2019 Intel Corporation.

# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

"""
etcd_client module is a etc wrapper to read any keys and watch on a directory
in etcd distributed key store
"""

import etcd3
import logging
import json
import os
from util.util import Util


class EtcdCli:

    def __init__(self, config):
        """ constructor which creates an EtcdCli instance, checks for
        the etcd service port availability
        :param config: config of type Dict with certFile, keyFile
        and trustFile"""

        self.logger = logging.getLogger(__name__)
        hostname = "localhost"

        # This change will be moved to an argument to the function in 2.3
        # This is done now for backward compatibility
        etcd_host = os.getenv("ETCD_HOST")
        if etcd_host is not None and etcd_host != "":
            hostname = etcd_host

        port = 2379

        if not Util.check_port_availability(hostname, port):
            raise Exception("etcd service port {} is not up!".format(port))

        try:
            if config["trustFile"] == "" and config["keyFile"] == "" \
               and config["certFile"] == "":
                self.etcd = etcd3.client(host=hostname, port=port)
            else:
                self.etcd = etcd3.client(host=hostname, port=port,
                                         ca_cert=config["trustFile"],
                                         cert_key=config["keyFile"],
                                         cert_cert=config["certFile"])
        except Exception as e:
            raise e
        self.callback = None
        self._setEnv()

    def _setEnv(self):
        """ _setEnv is a local function to set global env """
        value = self.etcd.get("/GlobalEnv/")
        jsonConfig = json.loads(value[0].decode('utf-8'))
        for key in jsonConfig.keys():
            os.environ[key] = jsonConfig[key]

    def GetConfig(self, key):
        """ GetConfig gets the value of a key from Etcd
        :param key: key to be queried on, form etcd
        :type: string
        :return: values returned from etcd based on key
        :rtype: string"""
        try:
            value = self.etcd.get(key)
        except Exception as e:
            raise e
        if value[0] is not None:
            return value[0].decode('utf-8')
        return value[0]

    def PutConfig(self, key, value):
        """ PutConfig to save a value to Etcd
        :param key: keyin etcd to set
        :type: string
        :param value: value to set key to
        :type: string
        """
        try:
            self.etcd.put(key, value)
        except Exception as e:
            raise e

    def onChangeCallback(self, event):
        key = event.events[0].key
        value = event.events[0].value
        self.callback(key.decode('utf-8'), value.decode('utf-8'))

    def RegisterDirWatch(self, key, callback):
        """ RegisterDirWatch registers to a callback and keeps a watch on the
        prefix of a specified key
        :param key: prefix of a key to keep a watch on, in etcd
        :type string
        :callback : callback function
        :return None"""

        self.callback = callback
        try:
            self.etcd.add_watch_prefix_callback(key, self.onChangeCallback)
        except Exception as e:
            raise e

    def RegisterKeyWatch(self, key, callback):
        """ RegisterKeyWatch registers to a callback and keeps a watch
        on a specified key
        :param key: key to keep a watch on, in etcd
        :type string
        :callback : callback function
        :return None"""

        self.callback = callback
        try:
            self.etcd.add_watch_callback(key, self.onChangeCallback)
        except Exception as e:
            raise e
