import sys
import json
import socket
import logging
# The kapacitor source code is needed for the below imports
from kapacitor.udf.agent import Agent, Handler
from kapacitor.udf import udf_pb2

logging.basicConfig(level=logging.DEBUG,
                    format='%(asctime)s %(levelname)s:%(name)s: %(message)s')
logger = logging.getLogger()


# Mirrors all points it receives back to Kapacitor
class ConnHandler(Handler):
    def __init__(self, agent):
        self._agent = agent

    def info(self):
        response = udf_pb2.Response()
        response.info.wants = udf_pb2.STREAM
        response.info.provides = udf_pb2.STREAM
        return response

    def init(self, init_req):
        self.sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
        server_address = '/tmp/classifier'
        try:
            self.sock.connect(server_address)
        except socket.error:
            print >>sys.stderr, "Socket connect failed"

        response = udf_pb2.Response()
        response.init.success = True
        return response

    def snapshot(self):
        response = udf_pb2.Response()
        response.snapshot.snapshot = ''
        return response

    def restore(self, restore_req):
        response = udf_pb2.Response()
        response.restore.success = False
        response.restore.error = 'not implemented'
        return response

    def begin_batch(self, begin_req):
        raise Exception("not supported")

    def point(self, point):
        fields = {}
        fields.update(dict(point.fieldsInt))
        fields.update(dict(point.fieldsDouble))
        fields.update(dict(point.fieldsString))
        # print >>sys.stderr, "Fields: ", json.dumps(fields)
        self.sock.send(json.dumps(fields))
        response = udf_pb2.Response()
        response.point.CopyFrom(point)
        self._agent.write_response(response, True)

    def end_batch(self, end_req):
        raise Exception("not supported")


if __name__ == '__main__':
    # Create an agent
    agent = Agent()

    # Create a handler and pass it an agent so it can write points
    h = ConnHandler(agent)

    # Set the handler on the agent
    agent.handler = h

    print >> sys.stderr, "Starting agent for ConnHandler"
    agent.start()
    agent.wait()
    print >> sys.stderr, "Agent finished"
