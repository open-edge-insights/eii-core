"""
Copyright (c) 2018 Intel Corporation.

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
"""

# Python grpc client implementation

import grpc

import json
import DataAgent.da_grpc.protobuff.da_pb2 as da_pb2
import DataAgent.da_grpc.protobuff.da_pb2_grpc as da_pb2_grpc
import os
import logging as log
from Util.proxy import ProxyHelper


class GrpcClient(object):

    def __init__(self, hostname="localhost", port="50051"):
        """
            GrpcClient constructor.
            Keyword Arguments:
            hostname - refers to hostname/ip address of the m/c
                       where DataAgent module of ETA is running
                       (default: localhost)
            port     - refers to gRPC port (default: 50051)
        """
        self.hostname = hostname
        self.port = port
        self.proxyHelper = ProxyHelper()


    def __daStub(self):
        """
            Private method to get the DaStub object
        """
        self.proxyHelper.unsetProxies()
        if 'GRPC_SERVER' in os.environ:
            self.hostname = os.environ['GRPC_SERVER']
        addr = "{0}:{1}".format(self.hostname, self.port)
        log.debug("Establishing grpc channel to %s", addr)
        channel = grpc.insecure_channel(addr)
        stub = da_pb2_grpc.daStub(channel)
        self.proxyHelper.resetProxies()
        return stub

    def GetBlob(self, imgHandle):
        """
            GetBlob is a wrapper around gRPC python client implementation
            for GetBlob gRPC interface.
            Arguments:
            imgHandle(string): key for ImageStore
            Returns:
            The consolidated byte array(value from ImageStore) associated with
            that imgHandle
        """
        log.debug("Inside GetBlob() client wrapper...")
        stub = self.__daStub()
        response = stub.GetBlob(da_pb2.BlobReq(imgHandle=imgHandle))
        outputBytes = b''
        response = stub.GetBlob(da_pb2.BlobReq(imgHandle=imgHandle))
        for resp in response:
            outputBytes += resp.chunk
        log.debug("Sending the response to the caller...")
        return outputBytes

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
        stub = self.__daStub()
        response = stub.GetConfigInt(da_pb2.ConfigIntReq(cfgType=config),
                                     timeout=1000)
        log.debug("Sending the response to the caller...")
        return json.loads(response.jsonMsg)
