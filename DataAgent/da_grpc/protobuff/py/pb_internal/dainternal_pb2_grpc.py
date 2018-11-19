# Generated by the gRPC Python protocol compiler plugin. DO NOT EDIT!
import grpc

import dainternal_pb2 as dainternal__pb2


class dainternalStub(object):
  """The Data Agent service definition.
  """

  def __init__(self, channel):
    """Constructor.

    Args:
      channel: A grpc.Channel.
    """
    self.GetConfigInt = channel.unary_unary(
        '/DataAgentInternal.dainternal/GetConfigInt',
        request_serializer=dainternal__pb2.ConfigIntReq.SerializeToString,
        response_deserializer=dainternal__pb2.ConfigIntResp.FromString,
        )


class dainternalServicer(object):
  """The Data Agent service definition.
  """

  def GetConfigInt(self, request, context):
    """**********Internal Interfaces***************
    GetConfigInt internal interface
    """
    context.set_code(grpc.StatusCode.UNIMPLEMENTED)
    context.set_details('Method not implemented!')
    raise NotImplementedError('Method not implemented!')


def add_dainternalServicer_to_server(servicer, server):
  rpc_method_handlers = {
      'GetConfigInt': grpc.unary_unary_rpc_method_handler(
          servicer.GetConfigInt,
          request_deserializer=dainternal__pb2.ConfigIntReq.FromString,
          response_serializer=dainternal__pb2.ConfigIntResp.SerializeToString,
      ),
  }
  generic_handler = grpc.method_handlers_generic_handler(
      'DataAgentInternal.dainternal', rpc_method_handlers)
  server.add_generic_rpc_handlers((generic_handler,))