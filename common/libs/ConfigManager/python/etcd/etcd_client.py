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
from libs.common.py.util import Util


class EtcdCli:

    def __init__(self, config):
        """ constructor which creates an EtcdCli instance, checks for
        the etcd service port availability """

        self.logger = logging.getLogger(__name__)
        hostname = "localhost"
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
        value = self.etcd.get("/GlobalEnv")
        jsonConfig = json.loads(value[0].decode('utf-8'))
        for key in jsonConfig.keys():
            os.environ[key] = jsonConfig[key]

    def GetConfig(self, key):
        """ GetConfig gets the value of a key from Etcd """
        try:
            value = self.etcd.get(key)
        except Exception as e:
            raise e
        if value[0] is not None:
            return value[0].decode('utf-8')
        return value[0]

    def onChangeCallback(self, event):
        key = event.events[0].key
        value = event.events[0].value
        self.callback(key.decode('utf-8'), value.decode('utf-8'))

    def RegisterDirWatch(self, key, callback):
        """ RegisterDirWatch registers to a callback and keeps a watch on the
        prefix of a specified key """

        self.callback = callback
        try:
            self.etcd.add_watch_prefix_callback(key, self.onChangeCallback)
        except Exception as e:
            raise e

    def RegisterKeyWatch(self, key, callback):
        """ RegisterKeyWatch registers to a callback and keeps a watch
        on a specified key """

        self.callback = callback
        try:
            self.etcd.add_watch_callback(key, self.onChangeCallback)
        except Exception as e:
            raise e
