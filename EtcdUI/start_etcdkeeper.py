#!/usr/bin/python3
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
# SOFTWARE.#!/usr/bin/python3

import os
import subprocess
from distutils.util import strtobool
import logging

if __name__ == "__main__":
    logger = logging.getLogger(__name__)
    try:
        devMode = bool(strtobool(os.environ['DEV_MODE']))
    except KeyError:
        devMode = 1
    try:
        if devMode:
            subprocess.run(["./etcdkeeper/etcdkeeper"])
        else:
            subprocess.run(["etcdkeeper/etcdkeeper",
                            "-h", "127.0.0.1",
                            "-usetls",
                            "-cacert", "/run/secrets/ca_etcd",
                            "-key", "/run/secrets/etcd_root_key",
                            "-cert", "/run/secrets/etcd_root_cert",
                            "-auth"])
    except Exception as e:
        logger.exception("etcdkeeper start exception:{}".format(e))
