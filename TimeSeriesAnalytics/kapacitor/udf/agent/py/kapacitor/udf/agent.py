# Kapacitor UDF Agent implementation in Python
#
# Requires protobuf v3
#   pip install protobuf==3.0.0b2
from __future__ import absolute_import
import sys
#import udf_pb2
from . import udf_pb2
from threading import Lock, Thread
#from Queue import Queue
try:
    from queue import Queue
except ImportError:
    from Queue import Queue

# Check for python3
PY3K = sys.version_info >= (3, 0)
ALIGN_TO_EIGHT_MULTIPLE = 8

import io
import traceback
import socket
import os

import logging
logger = logging.getLogger()


# The Agent calls the appropriate methods on the Handler as requests are read off STDIN.
#
# Throwing an exception will cause the Agent to stop and an ErrorResponse to be sent.
# Some *Response objects (like SnapshotResponse) allow for returning their own error within the object itself.
# These types of errors will not stop the Agent and Kapacitor will deal with them appropriately.
#
# The Handler is called from a single thread, meaning methods will not be called concurrently.
#
# To write Points/Batches back to the Agent/Kapacitor use the Agent.write_response method, which is thread safe.
class Handler(object):
    def info(self):
        pass
    def init(self, init_req):
        pass
    def snapshot(self):
        pass
    def restore(self, restore_req):
        pass
    def begin_batch(self, begin_req):
        pass
    def point(self):
        pass
    def end_batch(self, end_req):
        pass


# Python implementation of a Kapacitor UDF agent.
# This agent is responsible for reading and writing
# messages over STDIN and STDOUT.
#
# The Agent requires a Handler object in order to fulfill requests.
class Agent(object):
    def __init__(self, _in=sys.stdin, out=sys.stdout, handler=None):
        self._in = _in
        self._out = out
        # check for in and out buffers against stdin/stdout
        # in python3
        if PY3K:
            print("Python 3 setting will be enabled ", file=sys.stderr)
            if _in == sys.stdin:
                self._in = _in.buffer
            else:
                self._in = _in

            if out == sys.stdout:
                self._out = out.buffer
            else:
                self._out = out
        self._thread = None
        self.handler = handler
        self._write_lock = Lock()

    # Start the agent.
    # This method returns immediately
    def start(self):
        self._thread = Thread(target=self._read_loop)
        self._thread.start()

    # Wait for the Agent to terminate.
    # The Agent will terminate if STDIN is closed or an error occurs
    def wait(self):
        self._thread.join()
        self._in.close()
        self._out.close()

    # Write a response to STDOUT.
    # This method is thread safe.
    def write_response(self, response, flush=False):
        if response is None:
            raise Exception("cannot write None response")

        # Serialize message
        self._write_lock.acquire()
        try:
            data = response.SerializeToString()
            # Write message len
            encodeUvarint(self._out, len(data))
            # Write message
            self._out.write(data)
            if flush:
                self._out.flush()
        finally:
            self._write_lock.release()

    # Read requests off stdin
    def _read_loop(self):
        request = udf_pb2.Request()
        while True:
            msg = 'unknown'
            try:
                size = decodeUvarint32(self._in)
                data = self._in.read(size)

                request.ParseFromString(data)

                # use parsed message
                msg = request.WhichOneof("message")
                if msg == "info":
                    response = self.handler.info()
                    self.write_response(response, flush=True)
                elif msg == "init":
                    response = self.handler.init(request.init)
                    self.write_response(response, flush=True)
                elif msg == "keepalive":
                    response = udf_pb2.Response()
                    response.keepalive.time = request.keepalive.time
                    self.write_response(response, flush=True)
                elif msg == "snapshot":
                    response = self.handler.snapshot()
                    self.write_response(response, flush=True)
                elif msg == "restore":
                    response = self.handler.restore(request.restore)
                    self.write_response(response, flush=True)
                elif msg == "begin":
                    self.handler.begin_batch(request.begin)
                elif msg == "point":
                    self.handler.point(request.point)
                elif msg == "end":
                    self.handler.end_batch(request.end)
                else:
                    logger.error("received unhandled request %s", msg)
            except EOF:
                break
            except Exception as e:
                traceback.print_exc()
                error = "error processing request of type %s: %s" % (msg, e)
                logger.error(error)
                response = udf_pb2.Response()
                response.error.error = error
                self.write_response(response)
                break

# Indicates the end of a file/stream has been reached.
class EOF(Exception):
    pass

# Varint encode decode values
mask32uint = (1 << 32) - 1
byteSize = 8
shiftSize = byteSize - 1
varintMoreMask = 2**shiftSize
varintMask = varintMoreMask - 1


# Encode an unsigned varint
def encodeUvarint(writer, value):
    bits = value & varintMask
    value >>= shiftSize
    while value:
        size = varintMoreMask|bits
        # extracting bytes required to fit the value represented by size
        num_bytes = ((size.bit_length() + (ALIGN_TO_EIGHT_MULTIPLE -1)) &
                (-ALIGN_TO_EIGHT_MULTIPLE)) // 8

        #writer.write(bytes([varintMoreMask|bits]))
        writer.write((varintMoreMask|bits).to_bytes(num_bytes, sys.byteorder))
        bits = value & varintMask
        value >>= shiftSize
    num_bytes = ((bits.bit_length() + (ALIGN_TO_EIGHT_MULTIPLE -1)) &
            (-(ALIGN_TO_EIGHT_MULTIPLE))) // 8
    return writer.write((bits).to_bytes(num_bytes, sys.byteorder))

# Decode an unsigned varint, max of 32 bits
def decodeUvarint32(reader):
    result = 0
    shift = 0
    while True:
        byte = reader.read(1)
        if len(byte) == 0:
            raise EOF
        b = ord(byte)
        result |= ((b & varintMask) << shift)
        if not (b & varintMoreMask):
            result &= mask32uint
            return result
        shift += shiftSize
        if shift >= 32:
            raise Exception("too many bytes when decoding varint, larger than 32bit uint")

class Server(object):
    def __init__(self, socket_path, accepter):
        self._socket_path = socket_path
        self._listener = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM )
        self._listener.bind(socket_path)
        self._accepter = accepter

    def serve(self):
        self._listener.listen(5)
        try:
            while True:
                conn, addr = self._listener.accept()
                if PY3K:
                    conn = conn.makefile('rwb')
                else:
                    conn = conn.makefile()
                thread = Thread(target=self._accepter.accept, args=(conn,addr))
                thread.start()
        except:
            self.stop()

    def stop(self):
        self._listener.close()
        try:
            os.remove(self._socket_path)
        except:
            pass

