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

import grpc
import json
import logging as log
import sys
import os
path = os.path.abspath(__file__)
sys.path.append(os.path.join(os.path.dirname(path), "../../../protobuff/py/pb_internal/"))
import dainternal_pb2 as da_pb2
import dainternal_pb2_grpc as da_pb2_grpc


class GrpcInternalClient(object):

    def __init__(self, hostname="localhost", port="50052"):
        """
            GrpcInternalClient constructor.
            Keyword Arguments:
            hostname - refers to hostname/ip address of the m/c
                       where DataAgent module of ETA is running
                       (default: localhost)
            port     - refers to gRPC port (default: 50052)
        """
        self.hostname = hostname
        self.port = port
        if 'GRPC_SERVER' in os.environ:
            self.hostname = os.environ['GRPC_SERVER']
        addr = "{0}:{1}".format(self.hostname, self.port)
        log.debug("Establishing grpc channel to %s", addr)
        channel = grpc.insecure_channel(addr)
        self.stub = da_pb2_grpc.dainternalStub(channel)

    def GetConfigInt(self, config):
        """
            GetConfigInt is a wrapper around gRPC python client implementation
            for GetConfigInt gRPC interface.
            Arguments:
            config(string): InfluxDBCfg or RedisCfg
            Returns:
            The dictionary object corresponding to the config value
        """
        log.debug("Inside GetConfigInt() client wrapper...")
        response = self.stub.GetConfigInt(da_pb2.ConfigIntReq(cfgType=config),
                                     timeout=1000)
        log.debug("Sending the response to the caller...")
        return json.loads(response.jsonMsg)
