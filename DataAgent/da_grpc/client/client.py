# Python grpc client implementation

import grpc

import json
import DataAgent.da_grpc.protobuff.da_pb2 as da_pb2
import DataAgent.da_grpc.protobuff.da_pb2_grpc as da_pb2_grpc
import os
import socket
from Util.proxy import ProxyHelper


class GrpcClient(object):

    @staticmethod
    def GetBlob(imgHandle):
        """
               GetBlob is a wrapper around gRPC python client implementation for GetBlob interface.
            It takes the imgHandle string(key from ImageStore) of the image frame ingested in the ImageStore
            and returns the consolidated byte array(value from ImageStore) from ImageStore associated with
            that imgHandle
        """
        proxyHelper = ProxyHelper()
        proxyHelper.unsetProxies()
        hostname = "ia_data_agent"
        try:
            #hostname ia_data_agent resolution will fail for containers
            #running in net host namespace
            socket.gethostbyname(hostname)
        except socket.gaierror:
            hostname = "localhost"
        channel = grpc.insecure_channel("{0}:50051".format(hostname))
        stub = da_pb2_grpc.daStub(channel)
        # response = stub.GetBlob(imgHandle)
        response = stub.GetBlob(da_pb2.BlobReq(imgHandle=imgHandle))

        proxyHelper.resetProxies()
        return response


    @staticmethod
    def GetConfigInt(config):
        """
            This is a wrapper around gRPC python client implementation for
            GetConfigInt. It takes the config as the parameter (InfluxDBCfg,
            RedisCfg etc.,) returning the dictionary object for that
            particular config
        """
        proxyHelper = ProxyHelper()
        proxyHelper.unsetProxies()
       
        hostname = "ia_data_agent"
        try:
            #hostname ia_data_agent resolution will fail for containers
            #running in net host namespace
            socket.gethostbyname(hostname)
        except socket.gaierror:
            hostname = "localhost"
        channel = grpc.insecure_channel("{0}:50051".format(hostname))
        stub = da_pb2_grpc.daStub(channel)
        response = stub.GetConfigInt(da_pb2.ConfigIntReq(cfgType=config),
                                     timeout=1000)

        proxyHelper.resetProxies()
        return json.loads(response.jsonMsg)
