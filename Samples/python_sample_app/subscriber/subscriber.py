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

import eis.msgbus as mb
import os
from libs.ConfigManager import ConfigManager
from distutils.util import strtobool
from util.msgbusutil import MsgBusUtil
from util.util import Util


def start_subscriber():
    subscriber = None

    try:
        topics_list_sub = MsgBusUtil.get_topics_from_env("sub")

        app_name = os.environ["AppName"]
        conf = Util.get_crypto_dict(app_name)
        cfg_mgr = ConfigManager()
        config_client = cfg_mgr.get_config_client("etcd", conf)

        for topic in topics_list_sub:
            publisher, topic = topic.split("/")
            topic = topic.strip()
            dev_mode = bool(strtobool(os.environ["DEV_MODE"]))

            msgbus_cfg = MsgBusUtil.get_messagebus_config(topic, "sub",
                                                          publisher,
                                                          config_client,
                                                          dev_mode)
            break  # since it is a single element

        print('[INFO] Initializing message bus context')
        msgbus_sub = mb.MsgbusContext(msgbus_cfg)

        print(f'[INFO] Initializing subscriber for topic publish_test')
        subscriber = msgbus_sub.new_subscriber("publish_test")

        print('[INFO] Running...')

        while True:
            msg, _ = subscriber.recv()
            if msg is not None:
                print(f'[INFO] RECEIVED: {msg}')
            else:
                print('[INFO] Receive interrupted')
    except KeyboardInterrupt:
        print('[INFO] Quitting...')
    finally:
        if subscriber is not None:
            subscriber.close()
