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
"""EII Message Bus publisher example
"""

import os
import cfgmgr.config_manager as cfg
import time

def py_ex_func(key, json):
    print("Key is {}".format(key))
    print("json is {}".format(json))


try:
    os.environ["AppName"] = "VideoIngestion"

    # create ConfigMgr object
    ctx = cfg.ConfigMgr()
    
    # get AppCfg's obejct to get application's config('/<appname>/config')
    app_cfg = ctx.get_app_config()
    print('app config is : {}'.format((app_cfg.get_dict())))
    print('loop_video is : {}'.format((app_cfg["ingestor"]["loop_video"])))

    # get watch object
    watch_cfg = ctx.get_watch_obj()
    # Watching on GlobalEnv key
    watch_cfg.watch("/GlobalEnv/", py_ex_func)
    # Watching on VideoAnalytics prefix
    watch_cfg.watch_prefix("/VideoAnalytics", py_ex_func)

    # Watch the key "/<appname>/config" for any changes,
    # py_ex_func function will be called with updated value
    watch_cfg.watch_config(py_ex_func)

    # Watch the key "/<appname>/interface" for any changes,
    # py_ex_func function will be called with updated value
    watch_cfg.watch_interface(py_ex_func)
    print("Watching on app config & app interface for 60 seconds")
    time.sleep(60)

except KeyboardInterrupt:
    print('[INFO] Quitting...')
except Exception as e:
    print('Error during execution: {}'.format(e))
