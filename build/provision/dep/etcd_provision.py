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

import sys
import os
import shutil
import subprocess
import json
import string
import random
import yaml
import zmq
import base64
import zmq.auth
from distutils.util import strtobool
from util import _execute_cmd, get_appname, get_server_cert_key
from etcd_config_update import load_data_etcd
ETCD_PREFIX = os.environ['ETCD_PREFIX']


def put_zmqkeys(appname):
    """Generate public/private key for given app and put it in etcd
    :param appname: App Name
    :type file: String
    """
    secret_key = ''
    public_key = ''
    public_key, secret_key = zmq.curve_keypair()
    str_public_key = public_key.decode()
    str_secret_key = secret_key.decode()
    while str_public_key[0] is "-" or str_secret_key[0] is "-":
        print("Re-generating ZMQ keys")
        public_key, secret_key = zmq.curve_keypair()
        str_public_key = public_key.decode()
        str_secret_key = secret_key.decode()
    returncode = _execute_cmd(["./etcdctl", "put",
                              ETCD_PREFIX + "/Publickeys/" + appname,
                              public_key])
    if returncode != 0:
        print("Error putting Etcd public key for" + appname)
        sys.exit(-1)
    returncode = _execute_cmd(["./etcdctl", "put",
                              ETCD_PREFIX + "/" + appname +
                              "/private_key", secret_key])
    if returncode != 0:
        print("Error putting Etcd private key for" + appname)
        sys.exit(-1)


def enable_etcd_auth():
    """Enable Auth for etcd and Create root user with root role
    """
    password = os.environ['ETCD_ROOT_PASSWORD']
    returncode = _execute_cmd(["./etcd_enable_auth.sh", password])
    if returncode != 0:
        print("Error enabling Auth for etcd with root role")
        sys.exit(-1)


def create_etcd_users(appname):
    """create etcd user and role for given app. Allow Read only access
     only to appname, global and publickeys directory

    :param appname: App Name
    :type appname: String
    """
    returncode = _execute_cmd(["./etcd_create_user.sh", appname])
    if returncode != 0:
        print("Failed to create etcd user for {}".format(appname))


def etcd_health_check():
    returncode = _execute_cmd(["./etcd_health_check.sh"])
    if returncode != 0:
        print("etcd health check failed")


def clear_etcd_kv():
    returncode = _execute_cmd(["./etcdctl", "del", "--prefix",
                               ETCD_PREFIX + "/"])
    if returncode != 0:
        print("Clearing Prefix {} key failed".format(ETCD_PREFIX))


if __name__ == "__main__":
    devMode = bool(strtobool(os.environ['DEV_MODE']))
    if not os.environ['ETCD_HOST']:
        os.environ['ETCD_HOST'] = 'localhost'
    if not os.environ['ETCD_CLIENT_PORT']:
        os.environ['ETCD_CLIENT_PORT'] = '2379'

    os.environ['ETCDCTL_ENDPOINTS'] = os.getenv('ETCD_HOST') \
        + ':' + os.getenv('ETCD_CLIENT_PORT')
    if not devMode:
        os.environ["ETCD_CERT_FILE"] = \
            os.environ.get("ETCD_CERT_FILE", "/run/secrets/etcd_server_cert")
        os.environ["ETCD_KEY_FILE"] = \
            os.environ.get("ETCD_KEY_FILE", "/run/secrets/etcd_server_key")
        os.environ["ETCD_TRUSTED_CA_FILE"] = \
            os.environ.get("ETCD_TRUSTED_CA_FILE", "/run/secrets/ca_etcd")
        os.environ["ETCDCTL_CACERT"] = \
            os.environ.get("ETCD_TRUSTED_CA_FILE", "/run/secrets/ca_etcd")
        os.environ["ETCDCTL_CERT"] = \
            os.environ.get("ETCD_ROOT_CERT", "/run/secrets/etcd_root_cert")
        os.environ["ETCDCTL_KEY"] = \
            os.environ.get("ETCD_ROOT_KEY", "/run/secrets/etcd_root_key")

    if os.environ['provision_mode'] != "csl":
        etcd_health_check()

    if os.environ['provision_mode'] == "csl":
        clear_etcd_kv()

    apps = get_appname(str(sys.argv[1]))
    load_data_etcd("./config/eis_config.json", apps,
                   "./etcdctl", "Certificates/", devMode)
    for key, value in apps.items():
        try:
            if not devMode:
                if 'zmq' in value:
                    put_zmqkeys(key)
                if os.environ['provision_mode'] != "csl":
                    create_etcd_users(key)
        except ValueError:
            pass
    if not devMode and os.environ['provision_mode'] != "csl":
        enable_etcd_auth()
