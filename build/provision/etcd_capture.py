# Copyright (c) 2020 Intel Corporation.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
"""Script to write back data from ETCD Cluster to JSON file.
"""
import subprocess
import json
import os
import argparse
from distutils.util import strtobool


def parse_args():
    """Parse command line arguments.
    """
    a_p = argparse.ArgumentParser()
    a_p.add_argument('--ca_etcd',
                     default='./Certificates/ca/ca_certificate.pem',
                     help='ca Certificate')
    a_p.add_argument('--etcd_root_cert',
                     default="./Certificates/root/root_client_certificate.pem",
                     help='root cert')
    a_p.add_argument('--etcd_root_key',
                     default="./Certificates/root/root_client_key.pem",
                     help='root key')
    a_p.add_argument('--etcd_endpoints',
                     default="127.0.0.1:2379",
                     help='etcd_host:port')
    return a_p.parse_args()


def _execute_cmd(cmd):
    cmd_output = subprocess.check_output(cmd)
    return cmd_output


def main():
    """ main function
    """

    dev_mode = bool(strtobool(os.environ['DEV_MODE']))
    args = parse_args()
    if dev_mode:
        cmd = _execute_cmd(["./etcd/etcdctl", "get",
                            "--endpoints", args.etcd_endpoints,
                            "--from-key", "''", "--keys-only"])
    else:
        cmd = _execute_cmd(["./etcd/etcdctl",
                            "--endpoints", args.etcd_endpoints,
                            "--cacert", args.ca_etcd,
                            "--cert", args.etcd_root_cert,
                            "--key", args.etcd_root_key, "get",
                            "--from-key", "''", "--keys-only"])
    keys = str(cmd, encoding='utf-8')
    key_list = keys.split()
    value_list = []
    matchers = ['Publickeys', 'private_key',
                'ca_cert', 'server_cert', 'server_key']
    matching = [s for s in key_list if any(xs in s for xs in matchers)]

    for key in matching:
        if key in key_list:
            key_list.remove(key)

    for key in key_list:
        if dev_mode:
            cmd = _execute_cmd(["./etcd/etcdctl", "get",
                                "--endpoints", args.etcd_endpoints,
                                "--print-value-only", key])
        else:
            cmd = _execute_cmd(["./etcd/etcdctl",
                                "--endpoints", args.etcd_endpoints,
                                "--cacert", args.ca_etcd,
                                "--cert", args.etcd_root_cert,
                                "--key", args.etcd_root_key, "get",
                                "--print-value-only", key])
        value = json.loads(cmd.decode('utf-8'))
        value_list.append(value)

    etcd_dict = dict(zip(key_list, value_list))

    with open('etcd_capture_data.json', 'w') as json_file:
        json.dump(etcd_dict, json_file, sort_keys=True, indent=4)


if __name__ == "__main__":
    main()
