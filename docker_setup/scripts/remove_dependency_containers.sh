#!/bin/bash

source ./stop_dependency_containers.sh

echo "Removing dependency containers..."
# docker rm cmd removes the container
docker rm influx_cont \
            redis_cont \
            nats_cont \
            mosquitto_cont \
            postgres_cont 