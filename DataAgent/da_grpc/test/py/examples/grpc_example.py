# Python grpc client implementation

import logging
import argparse
import hashlib
import time
import sys
import os
path = os.path.abspath(__file__)
sys.path.append(os.path.join(os.path.dirname(path), "../../../client/py/"))
from client import GrpcClient

logging.basicConfig(level=logging.DEBUG,
                    format='%(asctime)s : %(levelname)s : %(name)s : [%(filename)s] :' +
                    '%(funcName)s : in line : [%(lineno)d] : %(message)s')
log = logging.getLogger("GRPC_TEST")


def parse_args():
    """Parse command line arguments
    """
    parser = argparse.ArgumentParser()
    parser.add_argument('--img_Handle', help='imgHandle key')
    parser.add_argument('--output_file', help='output image file')

    return parser.parse_args()

if __name__ == '__main__':

    args = parse_args()

    # If executing this script from other m/c, provide
    # the right hostname/ip addr of the system running
    # DataAgent module of ETA
    client = GrpcClient(hostname="localhost")

    # Testing GetBlob(imgHandle) gRPC call
    imgHandle = args.img_Handle
    outputFile = args.output_file

    log.info("Calling GetBlob(%s) gRPC interface...", imgHandle)
    start = time.time()
    outputBytes = client.GetBlob(imgHandle)
    end = time.time()
    timeTaken = end - start

    log.info("len(outputBytes): %s", len(outputBytes))

    log.info("Writing the binary data received into a file: %s", outputFile)
    with open(outputFile, "wb") as outfile:
        outfile.write(outputBytes)

    log.info("Time taken for GetBlob() call: %f secs", timeTaken)
