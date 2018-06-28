#!/bin/bash

source ./init.sh

# Starting dependency containers
echo "1. Starting influxdb container..."
docker run -d --rm --net host --name influx_cont \
           -v $etaConfDir/influxdb.conf:/etc/influxdb/influxdb.conf:ro \
           -v $etaDataDir/influxdb:/var/lib/influxdb \
           influxdb:1.5.3
docker logs influx_cont &> $etaLogDir/influxdb.log

echo "2. Starting redis container..."
docker run -d --rm --net host  --name redis_cont \
           -v $etaConfDir/redis.conf:/usr/local/etc/redis/redis.conf:ro \
           redis:4.0.10 \
           redis-server /usr/local/etc/redis/redis.conf
docker logs redis_cont &> $etaLogDir/redis.log

echo "3. Starting nats container..."
docker run -d --rm --net host --name nats_cont nats:1.1.0 -p 4222
docker logs nats_cont &> $etaLogDir/nats_server.log

echo "4. Starting mosquitto container..."
docker run -d --rm --net host --name mosquitto_cont \
           -v $etaConfDir/mosquitto.conf:/mosquitto/mosquitto.conf:ro \
           -v $etaDataDir/mosquitto:/mosquitto/data \
           -v $etaLogDir:/mosquitto/log/mosquitto.log \
           eclipse-mosquitto:1.4.12

echo "5. Starting postgres container..."
docker run -d --rm --net host --name postgres_cont \
                -e POSTGRES_USER=ali -e POSTGRES_PASSWORD=intel123 -e POSTGRES_DB=ali \
                postgres:10.4

# re-capturing logs
# TODO: need to come up with a better way of streaming logs of process running inside container
docker logs influx_cont &> $etaLogDir/influxdb.log
docker logs redis_cont &> $etaLogDir/redis.log
docker logs nats_cont &> $etaLogDir/nats_server.log
docker logs postgres_cont &> $etaLogDir/postgres.log