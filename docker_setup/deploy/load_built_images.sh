#!/bin/bash

source ./setenv.sh
mkdir -p $PWD/deploy/docker_images
cd $PWD/deploy/docker_images
curDir=`pwd`

echo "Loading all the docker images from $curDir folder..."

# loading all docker images of eta along with its dependencies
docker load -i log_rotate-$LOG_ROTATE_VERSION.tar && echo "Saved log_rotate docker image.."
docker load -i influxdb-$INFLUXDB_VERSION.tar && echo "Loaded influxdb docker image.."
docker load -i redis-$REDIS_VERSION.tar && echo "Loaded redis docker image.."

docker load -i gobase-$ETA_VERSION.tar && echo "Loaded gobase docker image.."
docker load -i pybase-$ETA_VERSION.tar && echo "Loaded pybase docker image.."
docker load -i gopybase-$ETA_VERSION.tar && echo "Loaded gopybase docker image.."
docker load -i data_agent-$ETA_VERSION.tar && echo "Loaded data_agent docker image.."
docker load -i data_analytics-$ETA_VERSION.tar && echo "Loaded data_analytics docker image.."
docker load -i video_ingestion-$ETA_VERSION.tar && echo "Loaded video_ingestion docker image.."
