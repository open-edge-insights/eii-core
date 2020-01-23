#!/usr/bin/python3
# Copyright (c) 2019 Intel Corporation.

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

import yaml
import subprocess
import argparse
import os


def parse_args():
    parser = argparse.ArgumentParser(description="Tool Used for stopping\
                                     and removing existing EIS containers")
    parser.add_argument('--f', dest='compose_file_path',
                        help='docker-compose.yml file path')
    return parser.parse_args()



def stop_and_remove_eis_containers(composefile):
    """Parse given docker-compose file, stops and removed any running containers.
    :param file: Full path of docker-compose file.
    :type file: String
    """
    with open(composefile, 'r') as ymlfile:
        config = yaml.load(ymlfile, Loader=yaml.FullLoader)
        for service in config['services'].keys():
            try:
                print("Stopping Service : ", service)
                subprocess.run("docker rm -f " + service, shell=True, stderr=open(os.devnull, 'wb'))
            except:
                print("Error stopping EIS. Please stop EIS manually \
                         and try provision again.")
                


if __name__ == "__main__":
    args = parse_args()
    stop_and_remove_eis_containers(args.compose_file_path)
