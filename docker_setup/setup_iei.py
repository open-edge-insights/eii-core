#!/usr/bin/python3

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


import argparse
import subprocess
import os
SYSTEMD_PATH = "/etc/systemd/system"
INSTALL_PATH = "/opt/intel/iei/"


def parse_args():

    parser = argparse.ArgumentParser()

    parser.add_argument('-s', '--setup_iei_service', help='setup iei service',
                        action="store_true")

    parser.add_argument('-u', '--uninstall_iei', help='uninstall iei pkg',
                        action="store_true")

    args = parser.parse_args()
    return args


def create_install_dir():
    print("Creating {0} before install if it doesn't exists...".format(
        INSTALL_PATH))
    output = subprocess.run(["mkdir", "-p", INSTALL_PATH])
    if output.returncode != 0:
        print("Unable to create install path" + INSTALL_PATH)
        exit(-1)


def enable_systemd_service():
    # Copy the systemd service file to /etc/systemd/system
    print("Copying the service file to systemd path...")
    output = subprocess.run(["cp", "iei.service", SYSTEMD_PATH])
    if output.returncode != 0:
        print("Unable to copy the systemd service file")
        exit(-1)

    # This is to keep systemctl DB sane after adding IEI service.
    print("systemctl daemon reload in progress...")
    output = subprocess.run(["systemctl", "daemon-reload"])
    if output.returncode != 0:
        print("Failed to execute systemctl daemon-reload")
        exit(-1)

    print("systemctl start of IEI service in progress...")
    output = subprocess.run(["systemctl", "start", "iei"])
    if output.returncode != 0:
        print("Unable to start IEI systemd service")
        exit(-1)

    print("systemctl enable of IEI service in progress...")
    output = subprocess.run(["systemctl", "enable", "iei"])
    if output.returncode != 0:
        print("Unable to add IEI systemd service as part of boot")
        exit(-1)


def uninstall_iei():
    print("***********Un-installing IEI***********")
    uninstall_list = [INSTALL_PATH,
                      "/etc/systemd/system/iei.service"]
    print("systemctl stop of IEI service in progress...")
    output = subprocess.run(["systemctl", "stop", "iei"])
    if output.returncode != 0:
        print("Unable to stop IEI systemd service file")

    cwd = os.getcwd()
    os.chdir(INSTALL_PATH)
    for i in uninstall_list:
        print("Removing "+i+" ...")
        if i == INSTALL_PATH:
            keepers = ['secret_store', 'tpm_secret']
            for filename in os.listdir('.'):
                if filename not in keepers:
                    print('Removing %s' % (filename,))
                    output = subprocess.run(["rm", "-rf", filename])
            if output.returncode != 0:
                print("Unable to remove" + i)
        else:
            output = subprocess.run(["rm", "-rf", i])
            if output.returncode != 0:
                print("Unable to remove" + i)
    os.chdir(cwd)

    # This is to keep systemctl DB sane after deleting IEI service.
    print("systemctl daemon reload in progress...")
    output = subprocess.run(["systemctl", "daemon-reload"])
    if output.returncode != 0:
        print("Unable to do systemd daemon-reload")

    print("***********Finished Un-installing IEI***********")


if __name__ == '__main__':
    args = parse_args()

    if args.setup_iei_service:
        create_install_dir()
        print("*****Setting up IEI Service*******")
        enable_systemd_service()
        print("***********Installation Finished***********")

    if args.uninstall_iei:
        if os.path.exists(INSTALL_PATH):
            uninstall_iei()
