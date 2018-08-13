# Python grpc client implementation

import logging
import argparse
from DataAgent.da_grpc.client.client import GrpcClient
from ImageStore.py.imagestore import ImageStore
import hashlib

logging.basicConfig(level=logging.INFO)
log = logging.getLogger("GRPC_TEST")

def parse_args():
    """Parse command line arguments
    """
    parser = argparse.ArgumentParser()
    parser.add_argument('--input_file', help='input image file')
    parser.add_argument('--output_file', help='output image file')

    return parser.parse_args()

if __name__ == '__main__':

    args = parse_args()

    log.info("Getting InfluxDB config:")
    config = GrpcClient.GetConfigInt("InfluxDBCfg")
    log.info(config)
    log.info("Getting Redis config:")
    config = GrpcClient.GetConfigInt("RedisCfg")
    log.info(config)

    inputFile = args.input_file
    outputFile = args.output_file

    log.info("GetBlob call...")

    imgStore = ImageStore()
    imgStore.setStorageType("inmemory")

    inputBytes = None
    with open(inputFile, "rb") as f:
        inputBytes = f.read()

    log.info("len(inputBytes): %s", len(inputBytes))
    key = imgStore.store(inputBytes)

    log.info("Image Handle: %s", key)
    outputBytes = GrpcClient.GetBlob(key)
    log.info("len(outputBytes): %s", len(outputBytes))

    with open(outputFile, "wb")  as outfile:
        outfile.write(outputBytes)

    digests = []
    for filename in [inputFile, outputFile]:
        hasher = hashlib.md5()
        with open(filename, 'rb') as f:
            buf = f.read()
            hasher.update(buf)
            a = hasher.hexdigest()
            digests.append(a)
            log.info("Hash for filename: %s is %s", filename, a)

    if digests[0] == digests[1]:
        log.info("md5sum for the files match")
    else:
        log.info("md5sum for the files doesn't match")