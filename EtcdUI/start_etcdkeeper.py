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
import cfgmgr.config_manager as cfg
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

    etcd_prefix = os.getenv('ETCD_PREFIX', "")
    etcd_prefix = etcd_prefix + '/'
    if os.getenv('ETCD_HOST') is not None:
        etcd_endpoint = os.getenv('ETCD_HOST', '127.0.0.1') + ':' + \
            os.getenv('ETCD_CLIENT_PORT', '2379')
    elif os.getenv('ETCD_ENDPOINT') is not None:
        etcd_endpoint = os.environ['ETCD_ENDPOINT']
    else:
        logger.error("ETCD_HOST/ETCD_ENDPOINT envs"
                     "not set or missing. Exiting!!!")
        sys.exit(-1)
    etcd_user = os.getenv("ETCD_USER", "root")

    if not devMode:
        ctx = cfg.ConfigMgr()
        config = ctx.get_app_config()
        server_cert = config["server_cert"]
        server_key = config["server_key"]

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
            with open("/tmp/nginx/eii_nginx_temp.conf", "w") as outfile:
                subprocess.run(["sed", sed_ip, "./eii_nginx_dev.conf"],
                               stdout=outfile, check=False)
            with open("/tmp/nginx/eii_nginx.conf", "w") as outfile:
                subprocess.run(["sed", sed_port,
                               "/tmp/nginx/eii_nginx_temp.conf"],
                               stdout=outfile, check=False)
        except subprocess.CalledProcessError as err:
            print("Subprocess error: {}, {}".format(err.returncode,
                  err.output))
    else:
        try:
            with open("/tmp/nginx/eii_nginx_temp.conf", "w") as outfile:
                subprocess.run(["sed", sed_ip, "./eii_nginx_prod.conf"],
                               stdout=outfile, check=False)
            with open("/tmp/nginx/eii_nginx.conf", "w") as outfile:
                subprocess.run(["sed", sed_port,
                               "/tmp/nginx/eii_nginx_temp.conf"],
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
            # Setting cert paths with option to override using env for K8s
            ca_cert = os.getenv("CONFIGMGR_CACERT", "/run/secrets/ca_etcd")
            key = os.getenv("CONFIGMGR_KEY", "/run/secrets/etcd_EtcdUI_key")
            cert = os.getenv("CONFIGMGR_CERT", "/run/secrets/etcd_EtcdUI_cert")
            subprocess.Popen(["./etcdkeeper/etcdkeeper",
                              "-h", "127.0.0.1",
                              "-p", "7070",
                              "-user", etcd_user,
                              "-sep", etcd_prefix,
                              "-usetls",
                              "-cacert", ca_cert,
                              "-key", key,
                              "-cert", cert,
                              "-auth"], start_new_session=True)

    except Exception as e:
        logger.exception("etcdkeeper start exception:{}".format(e))
    _execute_cmd("nginx")
