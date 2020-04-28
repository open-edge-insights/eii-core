#!/usr/bin/python3
# Copyright (c) 2020 Intel Corporation.

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

import os
import sys
import subprocess
from distutils.util import strtobool
import logging
import subprocess
from eis.config_manager import ConfigManager
from util.util import Util
import shutil
import threading


def _execute_cmd(cmd):
    cmd_output = subprocess.check_output(cmd, shell=True)
    return cmd_output


if __name__ == "__main__":
    logger = logging.getLogger(__name__)
    try:
        devMode = bool(strtobool(os.environ['DEV_MODE']))
    except KeyError:
        devMode = 1

    etcd_prefix = os.environ["ETCD_PREFIX"]
    etcd_prefix = etcd_prefix + '/'
    if os.getenv('ETCD_HOST') is not None:
        etcd_endpoint = os.getenv('ETCD_HOST', '127.0.0.1') + ':2379'
    elif os.getenv('ETCD_ENDPOINT') is not None:
        etcd_endpoint = os.environ['ETCD_ENDPOINT']
    else:
        logger.error("ETCD_HOST/ETCD_ENDPOINT envs not set or missing. Exiting!!!")
        sys.exit(-1)
    etcd_user = os.getenv("ETCD_USER", "root")
    app_name = os.environ["AppName"]
    conf = Util.get_crypto_dict(app_name)

    if etcd_user != "root":
        if devMode:
            get_prefix_command = "./etcdctl role get {} | \
                                    grep -A1 Read | grep prefix | \
                                    cut -d' ' -f4 | cut -d')' -f1".format(
                                    etcd_user)
        else:
            etcd_pwd = os.environ["ETCD_PASSWORD"]
            get_prefix_command = "./etcdctl --user {} --password {} --cacert {} --cert {} --key \
                                    {} --endpoints {} role get {} | grep -A1 Read | grep \
                                    prefix | cut -d' ' -f4 | cut -d')' \
                                    -f1".format(etcd_user,
                                                etcd_pwd,
                                                conf["trustFile"],
                                                conf["certFile"],
                                                conf["keyFile"],
                                                etcd_endpoint,
                                                etcd_user)

        etcd_prefix = _execute_cmd(get_prefix_command).decode(
                        'utf-8').rstrip()

    if not devMode:
        cfg_mgr = ConfigManager()
        config_client = cfg_mgr.get_config_client("etcd", conf)
        server_cert = config_client.GetConfig("/" + app_name + "/server_cert")
        server_key = config_client.GetConfig("/" + app_name + "/server_key")

        with open('/tmp/nginx/server_cert.pem', 'w') as f:
            f.write(server_cert)

        with open('/tmp/nginx/server_key.pem', 'w') as f:
            f.write(server_key)

    cmd = _execute_cmd("hostname -I | awk '{print $1}'")
    ip = str(cmd, encoding='utf-8').strip()

    port = os.environ["NGINX_PORT"]

    if devMode:
        cmd1 = "sed  's/NGINX_HOST/{}/' \
                ./eis_nginx_dev.conf > \
                /tmp/nginx/eis_nginx_temp.conf".format(ip)
        cmd2 = "sed  's/NGINX_PORT/{}/' \
                /tmp/nginx/eis_nginx_temp.conf \
                > /tmp/nginx/eis_nginx.conf".format(port)
        _execute_cmd(cmd1)
        _execute_cmd(cmd2)
    else:
        cmd1 = "sed  's/NGINX_HOST/{}/' \
                ./eis_nginx_prod.conf > \
                /tmp/nginx/eis_nginx_temp.conf".format(ip)
        cmd2 = "sed  's/NGINX_PORT/{}/' \
                /tmp/nginx/eis_nginx_temp.conf \
                > /tmp/nginx/eis_nginx.conf".format(port)
        _execute_cmd(cmd1)
        _execute_cmd(cmd2)

    try:
        if devMode:
            subprocess.Popen(["./etcdkeeper/etcdkeeper",
                              "-p", "7070",
                              "-sep", etcd_prefix],
                             start_new_session=True)

        else:
            subprocess.Popen(["./etcdkeeper/etcdkeeper",
                              "-p", "7070",
                              "-user", etcd_user,
                              "-sep", etcd_prefix,
                              "-usetls",
                              "-cacert", conf["trustFile"],
                              "-key", conf["keyFile"],
                              "-cert", conf["certFile"],
                              "-auth"], start_new_session=True)

    except Exception as e:
        logger.exception("etcdkeeper start exception:{}".format(e))
    _execute_cmd("nginx")
