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
ETCD_PREFIX = os.environ['ETCD_PREFIX']


def _execute_cmd(cmd):
    """Executes the shell cmd

    :param cmd: shell cmd
    :type cmd: str
    :return: process returncode
    :rtype: int
    """
    try:
        process = subprocess.run(cmd, stdout=subprocess.DEVNULL)
        return process.returncode
    except Exception as ex:
        print(ex)
        return -1


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
                                    print(ke)
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


def load_data_etcd(file, apps):
    """Parse given json file and add keys to etcd
    :param file: Full path of json file having etcd initial data
    :type file: String
    :param apps: dict for AppName:CertType
    :type apps: dict
    """
    with open(file, 'r') as f:
        config = json.load(f)
    print("=======Adding key/values to etcd========")
    for key, value in config.items():
        if key.split("/")[1] not in apps.keys() and key != '/GlobalEnv/':
            continue
        key = ETCD_PREFIX + key
        if isinstance(value, str):
            returncode = _execute_cmd(["./etcdctl", "put", key,
                                      bytes(value.encode())])
            if returncode != 0:
                print("Adding {} key failed".format(key))
                sys.exit(-1)
        elif isinstance(value, dict) and key == '/GlobalEnv/':
            # Adding DEV_MODE from env
            value['DEV_MODE'] = os.environ['DEV_MODE']
            returncode = _execute_cmd(["./etcdctl", "put", key,
                                      bytes(json.dumps(value,
                                       indent=4).encode())])
            if returncode != 0:
                print("Adding {} key failed".format(key))
                sys.exit(-1)
        elif isinstance(value, dict):
            # Adding ca cert, server key and cert in app config in PROD mode
            if not devMode:
                app_type = key[len(ETCD_PREFIX):].split('/')
                if app_type[2] == 'config':
                    if 'pem' in apps[app_type[1]] or \
                       'der' in apps[app_type[1]]:
                        server_cert_server_key = \
                            get_server_cert_key(app_type[1], apps[app_type[1]])
                        value.update(server_cert_server_key)
            returncode = _execute_cmd(["./etcdctl", "put", key,
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
        retruncode = _execute_cmd(["./etcdctl", "get", key])
        if returncode != 0:
            print("Reading {} key failed".format(key))
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


def get_server_cert_key(appname, certtype):
    """ parse appname and certtype, returns server cert and key dict
    :param appname: appname
    :type config: string
    :param certtype: certificate type
    :type apps: string
    :return: server cert key dict
    :rtype: dict
    """
    server_key_cert = {}
    cert_ext = None
    if 'pem' in certtype:
        cert_ext = ".pem"
    elif 'der' in certtype:
        cert_ext = ".der"
    cert_file = "Certificates/" + appname + "_Server/" + appname \
        + "_Server_server_certificate" + cert_ext
    key_file = "Certificates/" + appname + "_Server/" + appname \
        + "_Server_server_key" + cert_ext
    ca_certificate = "Certificates/ca/ca_certificate" + cert_ext

    if not os.path.isfile(cert_file):
        # For usage with certs mounted with helm created k8s secrets,
        # the directory structure will be flat
        cert_file = "Certificates/" + appname + "_Server_server_certificate" \
            + cert_ext
        key_file = "Certificates/" + appname + "_Server_server_key" + cert_ext
        ca_certificate = "Certificates/" + "ca_certificate" + cert_ext
        if not os.path.isfile(cert_file):
            print("Error: Server certificates not found for ", appname)
            return None

    if cert_ext == ".pem":
        with open(cert_file, 'r') as s_cert:
            server_cert = s_cert.read()
            server_key_cert["server_cert"] = server_cert
        with open(key_file, 'r') as s_key:
            server_key = s_key.read()
            server_key_cert["server_key"] = server_key
        with open(ca_certificate, 'r') as cert:
            ca_cert = cert.read()
            server_key_cert["ca_cert"] = ca_cert
    if cert_ext == ".der":
        with open(cert_file, 'rb') as s_cert:
            server_cert = s_cert.read()
            server_key_cert["server_cert"] = \
                base64.standard_b64encode(server_cert).decode("utf-8")
        with open(key_file, 'rb') as s_key:
            server_key = s_key.read()
            server_key_cert["server_key"] = \
                base64.standard_b64encode(server_key).decode("utf-8")
        with open(ca_certificate, 'rb') as cert:
            ca_cert = cert.read()
            server_key_cert["ca_cert"] = \
                base64.standard_b64encode(ca_cert).decode("utf-8")

    return server_key_cert


def etcd_health_check():
    returncode = _execute_cmd(["./etcd_health_check.sh"])
    if returncode != 0:
        print("etcd health check failed")

def clear_etcd_kv():
    returncode = _execute_cmd(["./etcdctl", "del", "--prefix", ETCD_PREFIX + "/"])
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

    etcd_health_check()

    apps = get_appname(str(sys.argv[1]))
    load_data_etcd("./config/eii_config.json", apps)
    for key, value in apps.items():
        try:
            if not devMode:
                if 'zmq' in value:
                    put_zmqkeys(key)
                create_etcd_users(key)
        except ValueError:
            pass
    if not devMode:
        enable_etcd_auth()
