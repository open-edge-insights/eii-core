#!/bin/bash

# docker stop cmd stops the container
docker stop influx_cont \
            redis_cont \
            nats_cont \
            mosquitto_cont \
            postgres_cont 