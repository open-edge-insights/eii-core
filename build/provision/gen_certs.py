#!/usr/bin/env python3

"""
Copyright (c) 2018 Intel Corporation.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
"""

import os
import shutil
import json
import cert_core
import paths
import argparse
import subprocess
import yaml


def parse_args():
    parser = argparse.ArgumentParser(description="Tool Used for Generating\
                                     SSL Certificates")
    parser.add_argument('--clean', action='store_true',
                        help='Clear All the Generated Certificates')
    parser.add_argument('--f', dest='compose_file_path',
                        help='docker-compose.yml file path')
    parser.add_argument('--capath', dest='rootca_path',
                        help='RootCA certificate Path, if given,cert-tool\
                        will re-use the existing rootCA certificate')
    return parser.parse_args()


def parse_yml(filepath):
    with open("config/x509_cert_config.json") as f:
        data = json.load(f)
    with open(filepath) as f:
        docs = yaml.load_all(f, Loader=yaml.FullLoader)
        for doc in docs:
            for key, value in doc.items():
                if key == "services":
                    for key, value in value.items():
                        for key, value in value.items():
                            if key == "environment":
                                if 'AppName' in value.keys():
                                    existingCert = False
                                    for keyValue in data["certs"]:
                                        if value['AppName'] in keyValue.keys():
                                            existingCert = True

                                    if not existingCert:
                                        cert_name = value['AppName']
                                        cert_details = {'client_alt_name': ''}
                                        data["certs"].append({cert_name:
                                                             cert_details})
                                if 'CertType' in value.keys():
                                    if 'pem' in value["CertType"]:
                                        cert_name = value['AppName'] \
                                            + "_Server"
                                        cert_details = {'server_alt_name': ''}
                                        data["certs"].append({cert_name:
                                                             cert_details})
                                    if 'der' in value["CertType"]:
                                        cert_name = value['AppName'] \
                                            + "_Server"
                                        cert_details = {'server_alt_name': '',
                                                        'output_format': 'DER'}
                                        data["certs"].append({cert_name:
                                                             cert_details})
    return data


def parse_json():
    with open("config/x509_cert_config.json") as f:
        data = json.load(f)
    return data


def copy_certificates_to_results_folder():
    os.makedirs(paths.relative_path("Certificates"), exist_ok=True)
    cert_core.copy_root_ca_certificate_and_key_pair()


def generate_root_ca():
    cert_core.generate_root_ca({"common_name": "rootca",
                                "client_alt_name": "rootca",
                                "server_alt_name": "rootca"})
    os.makedirs(paths.relative_path("Certificates", "ca"), exist_ok=True)


def generate(opts, root_ca_needed=True):
    if root_ca_needed:
        print("Generating root CA certs...")
        generate_root_ca()
    copy_certificates_to_results_folder()
    for cert in opts["certs"]:
        print("Generating Certificate for.......... " + str(cert) + "\n\n")
        os.environ["SAN"] = \
            "IP:127.0.0.1,DNS:localhost,DNS:*,URI:urn:unconfigured:application"
        for component, cert_opts in cert.items():
            if 'output_format' in cert_opts:
                outform = cert_opts['output_format']
            else:
                outform = None
            if "server_alt_name" in cert_opts:
                if os.environ["SSL_SAN_IP"] != "":
                    os.environ["SAN"] = "IP:" + \
                                        os.environ["HOST_IP"] + "," + "IP:" + \
                                        os.environ["SSL_SAN_IP"] + "," + \
                                        os.environ["SAN"]
                else:
                    os.environ["SAN"] = "IP:" + \
                        os.environ["HOST_IP"] + "," + os.environ["SAN"]
                cert_core.generate_server_certificate_and_key_pair(component,
                                                                   cert_opts)
                cert_core.copy_leaf_cert_and_key_pair("server",
                                                      component, outform)
            if "client_alt_name" in cert_opts:
                cert_core.generate_client_cert_and_key_pair(component,
                                                            cert_opts)
                cert_core.copy_leaf_cert_and_key_pair("client",
                                                      component, outform)


def clean():
    for trees in [paths.root_ca_path(), paths.leaf_pair_path("server"),
                  paths.result_path(),
                  paths.leaf_pair_path("client")]:
        print("Removing {}".format(trees))
        try:
            shutil.rmtree(trees)
        except FileNotFoundError:
            pass


if __name__ == '__main__':
    try:
        args = parse_args()
        if args.clean is True:
            clean()
            exit(1)
        if not args.compose_file_path:
            data = parse_json()
        else:
            data = parse_yml(args.compose_file_path)
        if args.rootca_path:
            root_ca_dir = args.rootca_path
            paths.root_ca_dir_name = root_ca_dir
            generate(data, False)   # re use existing root CA
        else:
            generate(data, True)  # Generate new root CA
    except Exception as err:
        print("Exception Occured in certificates generation" + str(err))
