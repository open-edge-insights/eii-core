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
# SOFTWARE.

import yaml
import zmq
import zmq.auth
import sys
import os
import shutil
import subprocess
import json
import string
import random
import logging
from distutils.util import strtobool


def get_appname(file):
    """Parse given docker-compose file and returns dict for
        AppName:CertType from environment
    :param file: Full path of docker-compose file.
    :type file: String
    """
    dictApps = {}
    with open(file) as f:
        docs = yaml.load_all(f, Loader=yaml.FullLoader)
        for doc in docs:
            for key, value in doc.items():
                if key == "services":
                    for key, value in value.items():
                        for key, value in value.items():
                            if key == "environment":
                                try:
                                    dictApps.setdefault(value["AppName"],
                                                        value["CertType"])
                                except KeyError as ke:
                                    logging.info(ke)
                                    pass

    return dictApps


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
        logging.info("Re-generating ZMQ keys")
        public_key, secret_key = zmq.curve_keypair()
        str_public_key = public_key.decode()
        str_secret_key = secret_key.decode()
    try:
        subprocess.run(["./etcdctl", "put",
                        "/Publickeys/" + appname, public_key])
    except Exception:
        logging.error("Error putting Etcd public key for" + appname)
    try:
        subprocess.run(["./etcdctl", "put",
                        "/" + appname + "/private_key", secret_key])
    except Exception:
        logging.error("Error putting Etcd private key for" + appname)


def enable_etcd_auth():
    """Enable Auth for etcd and Create root user with root role
    """
    password = os.environ['ETCD_ROOT_PASSWORD']
    subprocess.run(["./etcd_enable_auth.sh", password])


def load_data_etcd(file):
    """Parse given json file and add keys to etcd
    :param file: Full path of json file having etcd initial data
    :type file: String
    """
    with open(file, 'r') as f:
        config = json.load(f)
    logging.info("=======Adding key/values to etcd========")
    for key, value in config.items():
        if isinstance(value, str):
            subprocess.run(["./etcdctl", "put", key, bytes(value.encode())])
        elif isinstance(value, dict) and key == '/GlobalEnv':
            # Adding DEV_MODE from env
            value['DEV_MODE'] = os.environ['DEV_MODE']
            subprocess.run(["./etcdctl", "put",
                           key,
                           bytes(json.dumps(value, indent=4).encode())])
        elif isinstance(value, dict):
            subprocess.run(["./etcdctl", "put",
                            key, bytes(json.dumps(value, indent=4).encode())])

    logging.info("=======Reading key/values to etcd========")
    for key in config.keys():
        value = subprocess.run(["./etcdctl", "get", key])
        logging.info(key, '->', value)


def create_etcd_users(appname):
    """create etcd user and role for given app. Allow Read only access
     only to appname, global and publickeys directory

    :param appname: App Name
    :type appname: String
    """
    subprocess.run(["./etcd_create_user.sh", appname])


def put_x509_certs(appname, certtype):
    """Put required X509 certs to ETCD
    """
    subprocess.run(["./put_x509_certs.sh", appname, certtype])


if __name__ == "__main__":
    devMode = bool(strtobool(os.environ['DEV_MODE']))
    if not devMode:
        os.environ["ETCDCTL_CACERT"] = "/run/secrets/ca_etcd"
        os.environ["ETCDCTL_CERT"] = "/run/secrets/etcd_root_cert"
        os.environ["ETCDCTL_KEY"] = "/run/secrets/etcd_root_key"

    apps = get_appname(str(sys.argv[1]))
    load_data_etcd("./config/etcd_pre_load.json")
    for key, value in apps.items():
        try:
            if not devMode:
                if 'zmq' in value:
                    put_zmqkeys(key)
                if 'pem' in value:
                    put_x509_certs(key, 'pem')
                if 'der' in value:
                    put_x509_certs(key, 'der')
                create_etcd_users(key)
        except ValueError:
            pass

    if not devMode:
        enable_etcd_auth()
