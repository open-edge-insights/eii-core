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
from os import path
import shutil
import socket


root = os.getcwd()
root_ca_dir_name = "rootca"
result_dir_name = "Certificates"

#
# General
#


def relative_path(*paths):
    return path.join(root, *paths)


def copy_tuple_path(from_tuple, to_tuple):
    shutil.copy(relative_path(*from_tuple), relative_path(*to_tuple))


def openssl_cnf_path():
    return relative_path("openssl.cnf")


def container_name():
    return socket.gethostname()


#
# Root CA
#


def root_ca_path():
    return path.join(root, root_ca_dir_name)


def root_ca_certs_path():
    return path.join(root, root_ca_dir_name, "certs")


def root_ca_cert_path():
    return path.join(root, root_ca_dir_name, "cacert.pem")


def root_ca_key_path():
    return path.join(root, root_ca_dir_name,  "cakey.pem")


def root_ca_certificate_cer_path():
    return path.join(root, root_ca_dir_name, "cacert.der")

#
# Leaf (peer) certificates and keys
#


def leaf_pair_path(peer):
    return path.join(root, peer)


def leaf_certificate_path(peer):
    return relative_path(peer, "cert.pem")


def leaf_certificate_der_path(peer):
    return relative_path(peer, "cert.der")


def leaf_key_der_path(peer):
    return relative_path(peer, "key.der")


def leaf_key_path(peer):
    return relative_path(peer, "key.pem")


def leaf_key_path_der(peer):
    return relative_path(peer, "key.key")


#
# Results directory
#


def result_path():
    return path.join(root, result_dir_name)


def result_root_ca_certificate_path():
    return path.join(root, result_dir_name, "ca_certificate.pem")


def result_leaf_certificate_path(peer):
    return path.join(result_path(), "{}_certificate.pem".format(peer))


def result_leaf_key_path(peer):
    return path.join(result_path(), "{}_key.pem".format(peer))
