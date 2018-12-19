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


def parse_args():
    parser = argparse.ArgumentParser(description="Tool Used for Generating\
                                     SSL Certificates")
    parser.add_argument('--dns', dest='dns',
                        help='Domain Names for the Server Certificates')
    parser.add_argument('--clean', action='store_true',
                        help='Clear All the Generated Certificates')
    parser.add_argument('--capath', dest='rootca_path',
                        help='RootCA certificate Path, if given,cert-tool\
                        will re-use the existing rootCA certificate')
    return parser.parse_args()


def parse_json():
    with open("config.json") as f:
        data = json.load(f)
    return data


def add_server_entries(data, server):
    server_list_candidates = data["server_list_candidates"]
    for cert in data["certificates"]:

        for component, cert_opts in cert.items():
            if component in server_list_candidates:
                if "server_alt_name" in cert_opts:
                    cert_opts.update(
                        {"server_alt_name": [cert_opts["server_alt_name"],
                                             server]})
                else:
                    return None
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
        generate_root_ca()
    copy_certificates_to_results_folder()
    for cert in opts["certificates"]:
        print("Generating Certificate for.......... " + str(cert) + "\n\n")
        for component, cert_opts in cert.items():
            if 'output_format' in cert_opts:
                outform = cert_opts['output_format']
            else:
                outform = None
            if "server_alt_name" in cert_opts:
                cert_core.generate_server_certificate_and_key_pair(component,
                                                                   cert_opts)
                cert_core.copy_leaf_certificate_and_key_pair("server",
                                                             component, outform)
            if "client_alt_name" in cert_opts:
                cert_core.generate_client_certificate_and_key_pair(component,
                                                                   cert_opts)
                cert_core.copy_leaf_certificate_and_key_pair("client",
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


def process_config(server=None, root_ca_needed=True):
    data = parse_json()
    if server:
        data = add_server_entries(data, server)
        if not data:
            print("Error:: sever_alt_name is missing,Cannot Add DNS,\
Hence Exiting. Please modify config.json and try again")
    if data:
        generate(data, root_ca_needed)


if __name__ == '__main__':
    args = parse_args()
    if args.clean is True:
        clean()
        exit(1)
    elif args.dns and args.rootca_path:
        root_ca_dir = args.rootca_path
        paths.root_ca_dir_name = root_ca_dir
        process_config(args.dns, False)   # root CA not needed
    elif args.dns and not args.rootca_path:
        process_config(args.dns)
    elif not args.dns and args.rootca_path:
        process_config(args.dns)
    else:
        process_config()
