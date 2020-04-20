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

    cfgmgr_user_conf = Util.get_crypto_dict("etcduser")
    ETCD_CERT = cfgmgr_user_conf["certFile"]
    ETCD_KEY = cfgmgr_user_conf["keyFile"]
    CA_CERT = cfgmgr_user_conf["trustFile"]

    if not devMode:
        etcd_cert = ""
        etcd_key = ""
        ca_cert = ""
        try:
            etcd_cert = os.environ["ETCD_CERT"]
            etcd_key = os.environ["ETCD_KEY"]
            ca_cert = os.environ["CA_CERT"]
        except KeyError:
            pass

        if etcd_cert != "" and etcd_key != "":
            ETCD_CERT = etcd_cert
            ETCD_KEY = etcd_key

        if ca_cert != "":
            CA_CERT = ca_cert

    etcd_user = os.environ["ETCD_USER"]
    etcd_prefix = ""
    if etcd_user == "":
        logger.error("Please provide an Etcd User. Exiting!!!")
        sys.exit(-1)
    else:
        if etcd_user != "root":
            if devMode:
                get_prefix_command = "./etcdctl role get {} | \
                                        grep -A1 Read | grep prefix | \
                                        cut -d' ' -f4 | cut -d')' -f1".format(
                                        etcd_user)
            else:
                get_prefix_command = "./etcdctl --cacert {} --cert {} --key \
                                        {} role get {} | grep -A1 Read | grep \
                                        prefix | cut -d' ' -f4 | cut -d')' \
                                        -f1".format(CA_CERT,
                                                    ETCD_CERT,
                                                    ETCD_KEY,
                                                    etcd_user)

            etcd_prefix = _execute_cmd(get_prefix_command).decode(
                            'utf-8').rstrip()

    if not devMode:
        app_name = os.environ["AppName"]
        conf = Util.get_crypto_dict(app_name)
        cfg_mgr = ConfigManager()
        os.environ["ETCD_PREFIX"] = etcd_prefix
        config_client = cfg_mgr.get_config_client("etcd", conf)

        server_cert = config_client.GetConfig(etcd_prefix + "/" +
                                              app_name + "/server_cert")
        server_key = config_client.GetConfig(etcd_prefix + "/" +
                                             app_name + "/server_key")

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
                              "-sep", etcd_prefix + "/"],
                             start_new_session=True)

        else:
            subprocess.Popen(["./etcdkeeper/etcdkeeper",
                              "-h", "127.0.0.1",
                              "-p", "7070",
                              "-sep", etcd_prefix + "/",
                              "-user", etcd_user,
                              "-usetls",
                              "-cacert", CA_CERT,
                              "-key", ETCD_KEY,
                              "-cert", ETCD_CERT,
                              "-auth"], start_new_session=True)

    except Exception as e:
        logger.exception("etcdkeeper start exception:{}".format(e))
    _execute_cmd("nginx")
