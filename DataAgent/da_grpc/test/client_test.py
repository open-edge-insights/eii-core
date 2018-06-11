# Python grpc client implementation

import logging
from DataAgent.da_grpc.client.client import GrpcClient

logging.basicConfig(level=logging.INFO)
log = logging.getLogger("GRPC_TEST")

if __name__ == '__main__':

    for i in range(20):
        log.info("Iter#: {0}".format(i+1))
        log.info("Getting InfluxDB config:")
        config = GrpcClient.GetConfigInt("InfluxDBCfg")
        log.info(config)
        log.info("Getting Redis config:")
        config = GrpcClient.GetConfigInt("RedisCfg")
        log.info(config)
