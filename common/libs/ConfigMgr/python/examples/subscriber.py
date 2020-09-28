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
"""EIS Message Bus subscriber example
"""

import time
import json
import argparse
import os
import eis.msgbus as mb
import cfgmgr.config_manager as cfg

msgbus = None
subscriber = None

try:
    # For DEV_MODE true tests
    # os.environ["DEV_MODE"] = "TRUE"
    # os.environ["CONFIGMGR_CERT"] = ""
    # os.environ["CONFIGMGR_KEY"] = ""
    # os.environ["CONFIGMGR_CACERT"] = ""
    
    os.environ["DEV_MODE"] = "FALSE"
    # Set path to certs here
    os.environ["CONFIGMGR_CERT"] = ""
    os.environ["CONFIGMGR_KEY"] = ""
    os.environ["CONFIGMGR_CACERT"] = ""

    os.environ["AppName"] = "Visualizer"
    ctx = cfg.ConfigMgr()
    sub_ctx = ctx.get_subscriber_by_name("Cam2_Results")
    config = sub_ctx.get_msgbus_config()

    interface_value = sub_ctx.get_interface_value("Name")
    print('[INFO] Obtained interface_value is {}'.format(interface_value))

    topics = sub_ctx.get_topics()

    print('[INFO] Initializing message bus context')
    msgbus = mb.MsgbusContext(config)
    subscriber = msgbus.new_subscriber(topics[0])
    print('[INFO] Running...')
    while True:
        msg = subscriber.recv()
        print("recvd")
        meta_data, blob = msg
        if meta_data is not None:
            if True:
                print(f'[INFO] RECEIVED: meta data: {meta_data} \
                    for topic {msg.get_name()}')
        else:
            print('[INFO] Receive interrupted')

        if blob is not None:
            if True:
                print(f'[INFO] RECEIVED: blob: {msg.get_blob()} \
                    for topic {msg.get_name()}')
        else:
            print('[INFO] Receive interrupted')
except KeyboardInterrupt:
    print('[INFO] Quitting...')
finally:
    if subscriber is not None:
        subscriber.close()
