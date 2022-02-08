# Copyright (c) 2021 Intel Corporation.
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
"""Script to load docker images from docker_images folder
"""
import argparse
import os
import glob
import subprocess


"""load docker images from docker_images folder"""
def docker_load():
    cmdlist = []
    try:
        pwd = os.getcwd()
        docker_images = glob.glob(pwd+"/docker_images/*.tar")
        print(docker_images)
        for image in docker_images:
            print("docker load.....")
            cmdlist.append(["docker", "load", "-i", image])
        for cmd in cmdlist:
            print(cmd)
            subprocess.call(cmd)
    except Exception as err:
        print("Exception Occured ", str(err))


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description="docker load\
            ", formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser.add_argument('-dl',
                        '--docker_load',
                        action='store_true',
                        help='docker load services')
    arg = parser.parse_args()
    if arg.docker_load:
        docker_load()
