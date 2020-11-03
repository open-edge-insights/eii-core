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
import json
import argparse
import os
import eis.msgbus as mb
import cfgmgr.config_manager as cfg

msgbus = None
service = None

try:
    os.environ["AppName"] = "VideoAnalytics"

    ctx = cfg.ConfigMgr()
    print('[INFO] Total number of clients in interface is {}'.format(ctx.get_num_clients()))
    client_ctx = ctx.get_client_by_name("default")
    config = client_ctx.get_msgbus_config()
    print('[INFO] Obtained config is {}'.format(config))
    print('[INFO] Obtained endpoint is {}'.format(client_ctx.get_endpoint()))

    interface_value = client_ctx.get_interface_value("Name")
    print('[INFO] Obtained interface_value is {}'.format(interface_value))

    print('[INFO] Initializing message bus context')
    msgbus = mb.MsgbusContext(config)
    service = msgbus.get_service(interface_value)

    # Request used for the example
    request = {'int': 42, 'float': 55.5, 'str': 'Hello, World!', 'bool': True}

    print('[INFO] Running...')
    while True:
        print(f'[INFO] Sending request {request}')
        service.request(request)
        print('[INFO] Waiting for response')
        response = service.recv()
        print(f'[INFO] Received response: {response.get_meta_data()}')
        time.sleep(1)
except KeyboardInterrupt:
    print('[INFO] Quitting...')
finally:
    if service is not None:
        service.close()
