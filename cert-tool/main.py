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

import sys
import os
import shutil
import json
import cert_core
import paths


def parse_json():
    with open("config.json") as f:
        data = json.load(f)
    return data


def copy_certificates_to_results_folder(component=None, outform=None):
    os.makedirs(paths.relative_path("Certificates"), exist_ok=True)
    os.makedirs(paths.relative_path("Certificates", component), exist_ok=True)

    cert_core.copy_root_ca_certificate_and_key_pair()
    cert_core.copy_leaf_certificate_and_key_pair("server", component, outform)
    cert_core.copy_leaf_certificate_and_key_pair("client", component, outform)


def generate(opts):

    cert_core.generate_root_ca({"common_name": "rootca",
                                "client_alt_name": "rootca",
                                "server_alt_name": "rootca"})
    os.makedirs(paths.relative_path("Certificates", "ca"), exist_ok=True)
    for cert in opts["certificates"]:
        print("Generating Certificate for.......... " + str(cert) + "\n\n")
        for component, cert_opts in cert.items():
            if 'output_format' in cert_opts:
                outform = cert_opts['output_format']
            else:
                outform = None
            cert_core.generate_server_certificate_and_key_pair(cert_opts)
            cert_core.generate_client_certificate_and_key_pair(cert_opts)
            copy_certificates_to_results_folder(component, outform)


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
    cmd = ''
    if len(sys.argv) > 1:
        cmd = sys.argv[1]
    if cmd == "clean":
        clean()
    else:
        data = parse_json()
        generate(data)
