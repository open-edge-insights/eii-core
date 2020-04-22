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
from eis.config_manager import ConfigManager
from distutils.util import strtobool
from eis.env_config import EnvConfig
from util.util import Util


def start_subscriber(topic_string):
    subscriber = None

    try:
        topics_list_sub = EnvConfig.get_topics_from_env("sub")

        app_name = os.environ["AppName"]
        conf = Util.get_crypto_dict(app_name)
        cfg_mgr = ConfigManager()
        config_client = cfg_mgr.get_config_client("etcd", conf)
        dev_mode = bool(strtobool(os.environ["DEV_MODE"]))

        topic = topics_list_sub[0]
        publisher, topic = topic.split("/")
        topic = topic.strip()

        msgbus_cfg = EnvConfig.get_messagebus_config(topic, "sub",
                                                      publisher,
                                                      config_client,
                                                      dev_mode)

        print('[INFO] Initializing message bus context')
        msgbus_sub = mb.MsgbusContext(msgbus_cfg)

        print(f'[INFO] Initializing subscriber for topic {topic}')
        subscriber = msgbus_sub.new_subscriber(topic)

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
