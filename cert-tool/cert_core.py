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
import stat
import tempfile

from subprocess import run
import paths


def add_multiple_dns_entries(peer):
    multiple_domain = ''
    if isinstance(peer, list):
        for index, name in enumerate(peer, start=1):
            multiple_domain += "DNS." + str(index+1) + "=" + name
            multiple_domain += "\n"
    else:
        multiple_domain = "DNS.2=" + peer
        multiple_domain += "\n"
    return multiple_domain


def get_openssl_cnf_path(opts):
    if isinstance(opts["common_name"], list):
        cn = opts["common_name"][0]
    else:
        cn = opts["common_name"]

    if "client_alt_name" in opts:
        cli_domains = add_multiple_dns_entries(opts["client_alt_name"])
    else:
        cli_domains = ""
    if "server_alt_name" in opts:
        server_domains = add_multiple_dns_entries(opts["server_alt_name"])
    else:
        server_domains = ""

    cnf_path = paths.openssl_cnf_path()
    tmp_cnf_path = None
    with tempfile.NamedTemporaryFile(mode='w', delete=False) as outfile:
        with open(cnf_path, 'r') as infile:
            in_cnf = infile.read()
            out_cnf0 = in_cnf.replace('@COMMON_NAME@', cn)
            out_cnf1 = out_cnf0.replace('@MULTIPLE_CLI_DOMAINS@', cli_domains)
            out_cnf2 = out_cnf1.replace('@MULTIPLE_SERVER_DOMAINS@',
                                        server_domains)
            outfile.write(out_cnf2)
            tmp_cnf_path = outfile.name
    return tmp_cnf_path


def create_target_folder(target):
    os.makedirs(paths.relative_path("Certificates", target), exist_ok=True)


def copy_root_ca_certificate_and_key_pair(target="ca"):
    create_target_folder(target)
    paths.copy_tuple_path(
        (paths.root_ca_dir_name, "cacert.pem"),
        (paths.result_dir_name+"/ca", "ca_certificate.pem"))
    paths.copy_tuple_path(
        (paths.root_ca_dir_name, "cacert.der"),
        (paths.result_dir_name+"/ca", "ca_certificate.der"))
    paths.copy_tuple_path(
        (paths.root_ca_dir_name, "cakey.pem"),
        (paths.result_dir_name+"/ca", "ca_key.pem"))


def copy_leaf_certificate_and_key_pair(source, target, outform=None):
    create_target_folder(target)
    if outform:
        paths.copy_tuple_path((source, "cert.der"),
                              (paths.result_dir_name+"/"+target,
                               "{}_certificate.der".format(target+"_"+source)))
        paths.copy_tuple_path((source, "key.der"),
                              (paths.result_dir_name+"/"+target,
                               "{}_key.der".format(target+"_"+source)))
    else:
        paths.copy_tuple_path((source, "cert.pem"),
                              (paths.result_dir_name+"/"+target,
                               "{}_certificate.pem".format(target+"_"+source)))
        paths.copy_tuple_path((source, "key.pem"),
                              (paths.result_dir_name+"/"+target,
                               "{}_key.pem".format(target+"_"+source)))


def openssl_req(opts, *args, **kwargs):
    cnf_path = get_openssl_cnf_path(opts)
    print("=>\t[openssl_req]")
    xs = ["openssl", "req", "-config", cnf_path] + list(args)
    run(xs, **kwargs)


def openssl_x509(*args, **kwargs):
    print("=>\t[openssl_x509]")
    xs = ["openssl", "x509"] + list(args)
    run(xs, **kwargs)


def openssl_genrsa(*args, **kwargs):
    print("=>\t[openssl_genrsa]")
    xs = ["openssl", "genrsa"] + list(args)
    run(xs, **kwargs)


def openssl_ecparam(*args, **kwargs):
    print("=>\t[openssl_ecparam]")
    xs = ["openssl", "ecparam"] + list(args)
    run(xs, **kwargs)


def openssl_rsa(*args, **kwargs):
    print("=>\t[openssl_rsa]")
    xs = ["openssl", "rsa"] + list(args)
    run(xs, **kwargs)


def openssl_ca(opts, *args, **kwargs):
    cnf_path = get_openssl_cnf_path(opts)
    print("=>\t[openssl_ca]")
    xs = ["openssl", "ca", "-config", cnf_path] + list(args)
    run(xs, **kwargs)


def prepare_ca_directory(dir_name):
    os.makedirs(paths.relative_path(dir_name), exist_ok=True)
    os.makedirs(paths.relative_path(dir_name, "certs"), exist_ok=True)
    os.makedirs(paths.relative_path(dir_name, "private"), exist_ok=True)

    os.chmod(paths.relative_path(dir_name, "private"),
             stat.S_IRUSR | stat.S_IWUSR | stat.S_IXUSR)

    serial = open(paths.relative_path(dir_name, "serial"), "w")
    serial.write("01")
    serial.close

    index_txt = open(paths.relative_path(dir_name, "index.txt"), "w")
    index_txt.close
    index_txt = open(paths.relative_path(dir_name, "index.txt.attr"), "w")
    index_txt.close


def generate_root_ca(opts):
    prepare_ca_directory(paths.root_ca_path())

    openssl_req(opts,
                "-x509",
                "-days",    str(3650),
                "-newkey",  "rsa:{}".format(4096),
                "-keyout",  paths.root_ca_key_path(),
                "-out",     paths.root_ca_cert_path(),
                "-outform", "PEM",
                "-subj",    "/CN=ETACertToolSelfSignedtRootCA/L=$$$$/",
                "-nodes")
    openssl_x509("-in",      paths.root_ca_cert_path(),
                 "-out",     paths.root_ca_certificate_cer_path(),
                 "-outform", "DER")


def generate_server_certificate_and_key_pair(key, opts):
    generate_certificate_and_key_pair(key, "server", opts)


def generate_client_certificate_and_key_pair(key, opts):
    generate_certificate_and_key_pair(key, "client", opts)


def generate_certificate_and_key_pair(key, peer, opts,
                                      pa_cert_path=paths.root_ca_cert_path(),
                                      pa_key_path=paths.root_ca_key_path(),
                                      pa_certs_path=paths.root_ca_certs_path()
                                      ):

    os.makedirs(paths.relative_path(peer), exist_ok=True)
    if 'output_format' in opts:
        privkey_path = paths.leaf_key_path_der(peer)
    else:
        privkey_path = paths.leaf_key_path(peer)

    cert_path = paths.leaf_certificate_path(peer)
    req_pem_path = paths.relative_path(peer, "req.pem")
    CN = opts[peer+"_alt_name"]
    opts["common_name"] = key + "_" + (CN[0] if isinstance(CN, list) else CN)

    openssl_req(opts,
                "-new",
                "-newkey", "rsa:{}".format(4096),
                "-keyout",  privkey_path,
                "-out",     req_pem_path,
                "-days",    str(3650),
                "-outform", "PEM",
                "-subj", "/CN={}/O={}/L=$$$/".format(opts["common_name"],
                                                     peer),
                "-nodes")
    openssl_ca(opts,
               "-days",    str(3650),
               "-cert",    pa_cert_path,
               "-keyfile", pa_key_path,
               "-in",      req_pem_path,
               "-out",     cert_path,
               "-outdir",  pa_certs_path,
               "-notext",
               "-batch",
               "-extensions", "{}_extensions".format(peer))
    if 'output_format' in opts:
        print("Generating the DER file......")
        openssl_x509("-in",      cert_path,
                     "-out",     paths.leaf_certificate_der_path(peer),
                     "-outform", "DER")
        openssl_rsa("-in",  privkey_path,
                    "-out", paths.leaf_key_der_path(peer),
                    "-inform", "PEM",
                    "-outform", "DER")
