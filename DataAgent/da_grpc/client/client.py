# Python grpc client implementation

import grpc

import json
import DataAgent.da_grpc.protobuff.da_pb2 as da_pb2
import DataAgent.da_grpc.protobuff.da_pb2_grpc as da_pb2_grpc
import os

class GrpcClient(object):

    @staticmethod
    def GetConfigInt(config):
        """
            This is a wrapper around gRPC python client implementation for
            GetConfigInt. It takes the config as the parameter (InfluxDBCfg,
            RedisCfg etc.,) returning the dictionary object for that
            particular config
        """
        # Unsetting proxy to address StatusCode.Unavailable/putty sessions gRPC issue
        http_proxy = None
        https_proxy = None
        if 'http_proxy' in os.environ:
            http_proxy = os.environ['http_proxy']
            del os.environ['http_proxy']

        if 'https_proxy' in os.environ:
            https_proxy = os.environ['https_proxy']
            del os.environ['https_proxy']

        channel = grpc.insecure_channel('localhost:50051')
        stub = da_pb2_grpc.daStub(channel)

        response = stub.GetConfigInt(da_pb2.ConfigIntReq(cfgType=config),
                                     timeout=1000)

        # Resetting proxy variables
        if http_proxy is not None:
            os.environ['http_proxy'] = http_proxy

        if https_proxy is not None:
            os.environ['https_proxy'] = https_proxy

        return json.loads(response.jsonMsg)