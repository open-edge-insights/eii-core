#!/bin/bash

# docker stop cmd stops and removes the container as "--rm" switch was used
# to run the container
docker stop influx_cont \
            redis_cont \
            nats_cont \
            mosquitto_cont \
            postgres_cont 