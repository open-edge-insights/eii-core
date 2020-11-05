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
import json
import argparse
import os
import eis.msgbus as mb
import cfgmgr.config_manager as cfg

msgbus = None
publisher = None

try:
    os.environ["AppName"] = "VideoIngestion"
    ctx = cfg.ConfigMgr()
    dev_mode = ctx.is_dev_mode()
    if (dev_mode):
        print("Running in DEV mode")
    else:
        print("Running in PROD mode")
    app_name = ctx.get_app_name()
    print('[INFO] App name {}'.format(app_name))
    print('[INFO] Total number of publishers in interface is {}'.format(ctx.get_num_publishers()))

    pub_ctx = ctx.get_publisher_by_name("default")
    #pub_ctx = ctx.get_publisher_by_index(0)
    config = pub_ctx.get_msgbus_config()
    print('[INFO] Obtained config is {}'.format(config))

    interface_value = pub_ctx.get_interface_value("Name")
    print('[INFO] Obtained interface_value is {}'.format(interface_value))

    print('[INFO] Obtained endpoint is {}'.format(pub_ctx.get_endpoint()))
    print('[INFO] Obtained allowed clients is {}'.format(pub_ctx.get_allowed_clients()))
    topics = pub_ctx.get_topics()
    print('[INFO] Obtained topics is {}'.format(topics))
    new_topics = ["camera2_stream_results", "camera3_stream_results", "camera4_stream_results"]
    print('[INFO] Topics set {}'.format(pub_ctx.set_topics(new_topics)))
    print('[INFO] Obtained new topics is {}'.format(pub_ctx.get_topics()))
    

    print('[INFO] Initializing message bus context')
    msgbus = mb.MsgbusContext(config)
    publisher = msgbus.new_publisher(topics[0])

    print('[INFO] Running...')
    while True:
        blob = b'\x22' * 3
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
        time.sleep(3)
except KeyboardInterrupt:
    print('[INFO] Quitting...')
finally:
    if publisher is not None:
        publisher.close()
