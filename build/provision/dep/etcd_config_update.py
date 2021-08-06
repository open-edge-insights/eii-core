#!/usr/bin/python3
# Copyright (c) 2021 Intel Corporation.

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
import base64
from distutils.util import strtobool
from util import _execute_cmd, get_appname, get_server_cert_key


def source_env(file):
    """Method to source an env file

    :param file: Path of env file
    :type file: str
    """
    try:
        with open(file, 'r') as env_path:
            for line in env_path.readlines():
                # Checking if line has = in env
                if "=" in line:
                    # Emulating sourcing an env
                    key, value = line.strip().split("=")
                    os.environ[key] = value
    except Exception as err:
        print("Exception occured {}".format(err))
        return


def load_data_etcd(file, apps, etcdctl_path, certificates_dir_path, dev_mode):
    """Parse given json file and add keys to etcd
    :param file: Full path of json file having etcd initial data
    :type file: String
    :param apps: dict for AppName:CertType
    :type apps: dict
    """
    with open(file, 'r') as f:
        config = json.load(f)
    ETCD_PREFIX = os.environ['ETCD_PREFIX']
    print("=======Adding key/values to etcd========")
    for key, value in config.items():
        if key.split("/")[1] not in apps.keys() and key != '/GlobalEnv/':
            continue
        key = ETCD_PREFIX + key
        if isinstance(value, str):
            print(value)
            returncode = _execute_cmd([etcdctl_path, "put", key,
                                      bytes(value.encode())])
            if returncode != 0:
                print("Adding {} key failed".format(key))
                sys.exit(-1)
        elif isinstance(value, dict) and key == '/GlobalEnv/':
            # Adding DEV_MODE from env
            value['DEV_MODE'] = os.environ['DEV_MODE']
            returncode = _execute_cmd([etcdctl_path, "put", key,
                                      bytes(json.dumps(value,
                                       indent=4).encode())])
            if returncode != 0:
                print("Adding {} key failed".format(key))
                sys.exit(-1)
        elif isinstance(value, dict):
            # Adding ca cert, server key and cert in app config in PROD mode
            if not dev_mode:
                app_type = key[len(ETCD_PREFIX):].split('/')
                if app_type[2] == 'config':
                    if 'pem' in apps[app_type[1]] or \
                       'der' in apps[app_type[1]]:
                        server_cert_server_key = \
                            get_server_cert_key(app_type[1],
                                                apps[app_type[1]],
                                                certificates_dir_path)
                        value.update(server_cert_server_key)
            returncode = _execute_cmd([etcdctl_path, "put", key,
                                      bytes(json.dumps(value,
                                       indent=4).encode())])
            if returncode != 0:
                print("Adding {} key failed".format(key))
                sys.exit(-1)
        print("Added {} key successfully".format(key))

    print("=======Reading key/values from etcd========")
    for key in config.keys():
        if key.split("/")[1] not in apps.keys() and key != '/GlobalEnv/':
            continue
        key = ETCD_PREFIX + key
        retruncode = _execute_cmd([etcdctl_path, "get", key])
        if returncode != 0:
            print("Reading {} key failed".format(key))
            sys.exit(-1)


if __name__ == "__main__":
    source_env("../../.env")
    dev_mode = bool(strtobool(os.environ['DEV_MODE']))
    if not os.environ['ETCD_HOST']:
        os.environ['ETCD_HOST'] = 'localhost'
    if not os.environ['ETCD_CLIENT_PORT']:
        os.environ['ETCD_CLIENT_PORT'] = '2379'

    os.environ['ETCDCTL_ENDPOINTS'] = os.getenv('ETCD_HOST') \
        + ':' + os.getenv('ETCD_CLIENT_PORT')
    if not dev_mode:
        if os.path.isdir('../Certificates') is False:
            print('Certificate directory is missing')
            sys.exit(-1)

        os.environ["ETCDCTL_CACERT"] = \
            os.environ.get("ETCD_TRUSTED_CA_FILE",
                           "../Certificates/ca/ca_certificate.pem")
        os.environ["ETCDCTL_CERT"] = \
            os.environ.get("ETCD_ROOT_CERT",
                           "../Certificates/root/root_client_certificate.pem")
        os.environ["ETCDCTL_KEY"] = \
            os.environ.get("ETCD_ROOT_KEY",
                           "../Certificates/root/root_client_key.pem")
    apps = get_appname("../../docker-compose.yml")
    load_data_etcd("../config/eii_config.json", apps,
                   "../etcd/etcdctl", "../Certificates/", dev_mode)
