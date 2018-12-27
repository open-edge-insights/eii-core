#!/bin/bash
source ./setenv.sh
mkdir -p $PWD/deploy/docker_images
cd $PWD/deploy/docker_images
curDir=`pwd`

echo "Saving all the docker images to $curDir folder..."

# saving all docker images of eta along with its dependencies
docker save -o log_rotate-$LOG_ROTATE_VERSION.tar blacklabelops/logrotate:$LOG_ROTATE_VERSION && echo "Saved log_rotate docker image.."
docker save -o influxdb-$INFLUXDB_VERSION.tar ia/influxdb:$INFLUXDB_VERSION && echo "Saved influxdb docker image.."
docker save -o redis-$REDIS_VERSION.tar ia/redis:$REDIS_VERSION && echo "Saved redis docker image.."

docker save -o gobase-$ETA_VERSION.tar ia/gobase:$ETA_VERSION && echo "Saved gobase docker image.."
docker save -o pybase-$ETA_VERSION.tar ia/pybase:$ETA_VERSION && echo "Saved pybase docker image.."
docker save -o gopybase-$ETA_VERSION.tar ia/gopybase:$ETA_VERSION && echo "Saved gopybase docker image.."
docker save -o data_agent-$ETA_VERSION.tar ia/data_agent:$ETA_VERSION && echo "Saved data agent docker image.."
docker save -o data_analytics-$ETA_VERSION.tar ia/data_analytics:$ETA_VERSION && echo "Saved data analytics docker image.."
docker save -o video_ingestion-$ETA_VERSION.tar ia/video_ingestion:$ETA_VERSION && echo "Saved video ingestion docker image.."
