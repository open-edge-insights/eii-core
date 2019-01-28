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
import time
from Util.crypto.encrypt_decrypt import SymmetricEncryption
from Util.util import check_port_availability
from DataAgent.da_grpc.protobuff.py.pb_internal \
    import dainternal_pb2 as da_pb2
from DataAgent.da_grpc.protobuff.py.pb_internal \
    import dainternal_pb2_grpc as da_pb2_grpc


class GrpcInternalClient(object):

    def __init__(self, clientCert, clientKey,
                 caCert, hostname="localhost", port="50052"):
        """
            GrpcInternalClient constructor.
            Keyword Arguments:
            hostname   - refers to hostname/ip address of the m/c
                         where DataAgent module of IEI is running
                         (default: localhost)
            port       - refers to gRPC port (default: 50052)
            clientCert - client certificate
            clientKey  - client key
            caCert     - ca certificate used for signing client cert
        """
        self.hostname = hostname
        self.port = port
        if 'GRPC_SERVER' in os.environ:
            self.hostname = os.environ['GRPC_SERVER']
        addr = "{0}:{1}".format(self.hostname, self.port)
        log.debug("Establishing GRPC channel to %s", addr)

        try:
            key = os.environ["SHARED_KEY"]
            nonce = os.environ["SHARED_NONCE"]

            symEncrypt = SymmetricEncryption(key)
            ca_certs = symEncrypt.DecryptFile(caCert, nonce)
            client_key = symEncrypt.DecryptFile(clientKey, nonce)
            client_certs = symEncrypt.DecryptFile(clientCert, nonce)
            credentials = grpc.ssl_channel_credentials(
                root_certificates=ca_certs, private_key=client_key,
                certificate_chain=client_certs)

        except Exception as e:
            errMsg = "Exception Occured : {}".format(e)
            log.error(errMsg)
            raise Exception(errMsg)

        # check for grpc internal port availability
        if not check_port_availability(self.hostname, self.port):
            raise Exception("{}:{} port is not up!".format(self.hostname,
                            self.port))
        channel = grpc.secure_channel(addr, credentials)
        self.stub = da_pb2_grpc.dainternalStub(channel)

    def GetConfigInt(self, config):
        """
            GetConfigInt is a wrapper around gRPC python client implementation
            for GetConfigInt gRPC interface.
            Arguments:
            config(string): InfluxDBCfg or RedisCfg or PersistenceImageStore
                            or MinioCfg
            Returns:
            The dictionary object corresponding to the config value
        """
        log.debug("Inside GetConfigInt() client wrapper...")
        response = self.stub.GetConfigInt(da_pb2.ConfigIntReq(cfgType=config),
                                          timeout=1000)
        log.debug("Sending the response to the caller...")

        respDict = json.loads(response.jsonMsg)
        if config == "InfluxDBCfg" and 'INFLUX_SERVER' in os.environ:
            respDict["Host"] = os.environ['INFLUX_SERVER']
        elif ((config == "RedisCfg" or config == "MinioCfg") and
              'IMAGESTORE_SERVER' in os.environ):
            respDict["Host"] = os.environ['IMAGESTORE_SERVER']

        return respDict
