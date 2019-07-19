import cv2
import os
import argparse
import json
import etcd3
from distutils.util import strtobool
from libs.ConfigManager.etcd.py.etcd_client import EtcdCli


def main():
    """main method"""
    args = parse_args()
    etcd = etcd3.client(host="localhost", port=2379,
                        ca_cert="/run/secrets/etcd.ca.cert",
                        cert_key="/run/secrets/etcd.client.key",
                        cert_cert="/run/secrets/etcd.client.cert")

    etcd.put("VideoIngestion1/Subscribers", "client0,client1")
    for i in range(4):
        with open(args.public_keys_dir +
                  "client"+str(i) +
                  ".key", "rb") as client_key:
            client_key_encoded = client_key.read()
            etcd.put('PublicKeys/client'+str(i), client_key_encoded)


def parse_args():

    parser = argparse.ArgumentParser()

    parser.add_argument('--public-key', dest='public_keys_dir',
                        default="",
                        help='Public keys directory')

    return parser.parse_args()


if __name__ == "__main__":
    main()
