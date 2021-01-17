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
import base64


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


def get_server_cert_key(appname, certtype, certificates_dir_path):
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
    cert_file = certificates_dir_path + appname + "_Server/" + appname \
        + "_Server_server_certificate" + cert_ext
    key_file = certificates_dir_path + appname + "_Server/" + appname \
        + "_Server_server_key" + cert_ext
    ca_certificate = certificates_dir_path + "ca/ca_certificate" + cert_ext
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
