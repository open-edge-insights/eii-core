# Copyright (c) 2020 Intel Corporation.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
"""EIS Message Bus echo service client Python example.
"""

import time
import eis.msgbus as mb
import os
import json
from distutils.util import strtobool
from eis.env_config import EnvConfig
from eis.config_manager import ConfigManager
from util.util import Util


def start_client(server_name):
    msgbus = None
    service = None

    try:
        print('[INFO] Initializing message bus context')

        app_name = os.environ["AppName"]
        conf = Util.get_crypto_dict(app_name)
        cfg_mgr = ConfigManager()
        config_client = cfg_mgr.get_config_client("etcd", conf)
        dev_mode = bool(strtobool(os.environ["DEV_MODE"]))

        msgbus_cfg = EnvConfig.get_messagebus_config(server_name, "client",
                                                      server_name,
                                                      config_client, dev_mode)
        msgbus = mb.MsgbusContext(msgbus_cfg)

        app_config = config_client.GetConfig("/" + app_name + "/config")
        app_config_dict = json.loads(app_config)

        print(f'[INFO] Initializing service for {server_name}')
        service = msgbus.get_service(server_name)

        # Request used for the example
        request = {'int': 42, 'float': 55.5,
                   'str': 'Hello, World!', 'bool': True}

        print('[INFO] Running...')
        while True:
            print(f'[INFO] Sending request {request}')
            service.request(request)
            print('[INFO] Waiting for response')
            response, _ = service.recv()
            print(f'[INFO] Received response: {response}')
            time.sleep(int(app_config_dict["loop_interval"]))
    except KeyboardInterrupt:
        print('[INFO] Quitting...')
    finally:
        if service is not None:
            service.close()
