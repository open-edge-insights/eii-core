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
"""EIS Message Bus publisher example
"""

import time
import eis.msgbus as mb
import os
import json
from distutils.util import strtobool
from util.msgbusutil import MsgBusUtil
from eis.config_manager import ConfigManager
from util.util import Util


def start_publisher(topic_string):
    publisher = None

    try:
        topics_list_pub = MsgBusUtil.get_topics_from_env("pub")

        app_name = os.environ["AppName"]
        clients = os.environ["Clients"].split(',')
        conf = Util.get_crypto_dict(app_name)
        cfg_mgr = ConfigManager()
        config_client = cfg_mgr.get_config_client("etcd", conf)
        dev_mode = bool(strtobool(os.environ["DEV_MODE"]))

        topic = topics_list_pub[0]
        topic = topic.strip()

        msgbus_cfg = MsgBusUtil.get_messagebus_config(topic, "pub",
                                                      clients,
                                                      config_client,
                                                      dev_mode)

        app_config = config_client.GetConfig("/" + app_name + "/config")
        app_config_dict = json.loads(app_config)

        print('[INFO] Initializing message bus context')
        msgbus_pub = mb.MsgbusContext(msgbus_cfg)

        print(f'[INFO] Initializing publisher for topic \'{topic_string}\'')
        publisher = msgbus_pub.new_publisher(topic_string)

        print('[INFO] Running...')
        while True:
            blob = b'\x22' * 10
            meta = {
                'integer': 123,
                'floating': 55.5,
                'string': 'test',
                'boolean': True,
                'empty': None,
                'obj': {'test': {'test2': 'hello'}, 'test3': 'world'},
                'arr': ['test', 123]
            }

            publisher.publish((meta, blob,))
            time.sleep(int(app_config_dict["loop_interval"]))
    except KeyboardInterrupt:
        print('[INFO] Quitting...')
    finally:
        if publisher is not None:
            publisher.close()
