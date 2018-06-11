# Python grpc client implementation

import grpc

import json
import DataAgent.da_grpc.protobuff.da_pb2 as da_pb2
import DataAgent.da_grpc.protobuff.da_pb2_grpc as da_pb2_grpc


class GrpcClient(object):

    @staticmethod
    def GetConfigInt(config):
        """
            This is a wrapper around gRPC python client implementation for
            GetConfigInt. It takes the config as the parameter (InfluxDBCfg,
            RedisCfg etc.,) returning the dictionary object for that
            particular config
        """
        channel = grpc.insecure_channel('localhost:50051')
        stub = da_pb2_grpc.daStub(channel)

        response = stub.GetConfigInt(da_pb2.ConfigIntReq(cfgType=config),
                                     timeout=1)
        # print("response:" +  response.jsonMsg)
        return json.loads(response.jsonMsg)
