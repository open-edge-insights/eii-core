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

import socket
import logging as log
import os
import base64


def check_port_availability(hostname, port):
    """
        Verifies port availability on hostname for accepting connection
        Arguments:
        hostname(str) - hostname of the machine
        port(str)     - port
    """
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    log.debug("Attempting to connect to {}:{}".format(hostname, port))
    numRetries = 5
    retryCount = 0
    portUp = False
    while(retryCount < numRetries):
        if(sock.connect_ex((hostname, int(port)))):
            portUp = True
            break
        retryCount += 1
        log.debug("{} port is not up on {}".format(port, hostname))
    return portUp

def write_certs(file_list, file_data):
    file_name = ""
    for file_path in file_list:
        try:
            with open(file_path, 'wb+') as fd:
                file_name = os.path.basename(file_path)
                fd.write(base64.b64decode(file_data[file_name]))
        except Exception as e:
                log.debug("Failed creating file {}, {} ".format(file_name, e))
