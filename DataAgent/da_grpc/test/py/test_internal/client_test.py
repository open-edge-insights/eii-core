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

# Python grpc client implementation

import logging
import argparse
import hashlib
import time
import sys
import os
from DataAgent.da_grpc.client.py.client_internal.client \
    import GrpcInternalClient

logging.basicConfig(level=logging.DEBUG,
                    format='%(asctime)s : %(levelname)s : \
                    %(name)s : [%(filename)s] :' +
                    '%(funcName)s : in line : [%(lineno)d] : %(message)s')
log = logging.getLogger("GRPC_TEST")

CLIENT_CERT = "/etc/ssl/grpc_int_ssl_secrets/grpc_internal_client_certificate.pem"
CLIENT_KEY = "/etc/ssl/grpc_int_ssl_secrets/grpc_internal_client_key.pem"
CA_CERT = "/etc/ssl/grpc_int_ssl_secrets/ca_certificate.pem"

if __name__ == '__main__':

    # If executing this script from other m/c, provide
    # the right hostname/ip addr of the system running
    # DataAgent module of ETA
    client = GrpcInternalClient(CLIENT_CERT, CLIENT_KEY, CA_CERT)

    # Testing GetConfigInt("InfluxDBCfg") gRPC call
    log.info("Getting InfluxDB config:")
    totalTime1 = 0.0
    iter = 50
    for index in range(iter):
        start = time.time()
        config = client.GetConfigInt("InfluxDBCfg")
        end = time.time()
        timeTaken = end - start
        totalTime1 += timeTaken
        log.info("index: %d, time: %f secs",
                 index, timeTaken)

    # Testing GetConfigInt("RedisCfg") gRPC call
    log.info("Getting Redis config:")
    totalTime2 = 0.0
    for index in range(iter):
        start = time.time()
        config = client.GetConfigInt("RedisCfg")
        end = time.time()
        timeTaken = end - start
        totalTime2 += timeTaken
        log.info("index: %d, time: %f secs, config: %s",
                 index, timeTaken, config)

    log.info("Average time taken for GetConfigInt(\"InfluxDBCfg\") %d calls: \
             %f secs", iter, totalTime1 / iter)
    log.info("Average time taken for GetConfigInt(\"RedisCfg\") %d calls: \
             %f secs", iter, totalTime2 / iter)
