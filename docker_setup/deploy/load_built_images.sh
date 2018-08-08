#!/bin/bash

mkdir -p $PWD/deploy/docker_images
cd $PWD/deploy/docker_images
curDir=`pwd`

echo "Loading all the docker images from $curDir folder..."

# loading all docker images of eta along with its dependencies
docker load -i influxdb-1.5.3.tar
docker load -i redis-4.0.10.tar
docker load -i mosquitto-1.4.12.tar

docker load -i data_agent-1.0.tar
docker load -i classifier-1.0.tar
docker load -i video_ingestion-1.0.tar