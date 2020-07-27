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
    cmd_output = subprocess.check_output(cmd)
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
        etcd_endpoint = os.getenv('ETCD_HOST', '127.0.0.1') + ':'+ os.getenv('ETCD_CLIENT_PORT','2379')
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
            get_prefix_command1 = subprocess.run(["./etcdctl", "role", "get",
                                  etcd_user], stdout=subprocess.PIPE,
                                  check=False)
            get_prefix_command2 = subprocess.run(["grep", "-A1", "Read"],
                                  input=get_prefix_command1.stdout,
                                  stdout=subprocess.PIPE, check=False)
            get_prefix_command3 = subprocess.run(["grep", "prefix"],
                                  input=get_prefix_command2.stdout,
                                  stdout=subprocess.PIPE, check=False)
            get_prefix_command4 = subprocess.run(["cut", "-d", "' '", "-f4"],
                                  input=get_prefix_command3.stdout,
                                  stdout=subprocess.PIPE, check=False)
            get_prefix_command = subprocess.run(["cut", "-d", ')', "-f1"],
                                 input=get_prefix_command4.stdout,
                                 stdout=subprocess.PIPE, check=False)
        else:
            etcd_pwd = os.environ["ETCD_PASSWORD"]
            get_prefix_command1 = subprocess.run(["./etcdctl", "--user",
                                  etcd_user, "--password", etcd_pwd,
                                  "--cacert", conf["trustFile"], "--cert",
                                  conf["certFile"], "--key", conf["keyFile"],
                                  "--endpoints", etcd_endpoint, "role",
                                  "get", etcd_user], stdout=subprocess.PIPE,
                                  check=False)
            get_prefix_command2 = subprocess.run(["grep", "-A1", "Read"],
                                  input=get_prefix_command1.stdout,
                                  stdout=subprocess.PIPE, check=False)
            get_prefix_command3 = subprocess.run(["grep", "prefix"],
                                  input=get_prefix_command2.stdout,
                                  stdout=subprocess.PIPE, check=False)
            get_prefix_command4 = subprocess.run(["cut", "-d", "' '", "-f4"],
                                  input=get_prefix_command3.stdout,
                                  stdout=subprocess.PIPE, check=False)
            get_prefix_command = subprocess.run(["cut", "-d", ')', "-f1"],
                                 input=get_prefix_command4.stdout,
                                 stdout=subprocess.PIPE, check=False)

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
    cmd1 = subprocess.run(["hostname", "-I"], stdout=subprocess.PIPE,
                          check=False)
    cmd2 = subprocess.run(["awk", '{print $1}'], input=cmd1.stdout,
                          stdout=subprocess.PIPE, check=False)
    ip = cmd2.stdout.decode('utf-8').rstrip("\n")
    port = os.environ["NGINX_PORT"]

    sed_ip = 's/NGINX_HOST/{}/'.format(ip)

    sed_port = 's/NGINX_PORT/{}/'.format(port)

    if devMode:
        try:
            with open("/tmp/nginx/eis_nginx_temp.conf", "w") as outfile:
                subprocess.run(["sed", sed_ip, "./eis_nginx_dev.conf"],
                               stdout=outfile, check=False)
            with open("/tmp/nginx/eis_nginx.conf", "w") as outfile:
                subprocess.run(["sed", sed_port,
                               "/tmp/nginx/eis_nginx_temp.conf"],
                               stdout=outfile, check=False)
        except subprocess.CalledProcessError as err:
            print("Subprocess error: {}, {}".format(err.returncode,
                  err.output))
    else:
        try:
            with open("/tmp/nginx/eis_nginx_temp.conf", "w") as outfile:
                subprocess.run(["sed", sed_ip, "./eis_nginx_prod.conf"],
                               stdout=outfile, check=False)
            with open("/tmp/nginx/eis_nginx.conf", "w") as outfile:
                subprocess.run(["sed", sed_port,
                               "/tmp/nginx/eis_nginx_temp.conf"],
                               stdout=outfile, check=False)
        except subprocess.CalledProcessError as err:
            print("Subprocess error: {}, {}".format(err.returncode,
                  err.output))

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
