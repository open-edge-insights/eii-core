#!/bin/bash
source ./setenv.sh
mkdir -p $PWD/deploy/docker_images
cd $PWD/deploy/docker_images
curDir=`pwd`

echo "Saving all the docker images to $curDir folder..."

# saving all docker images of eta along with its dependencies
docker save -o log_rotate-$LOG_ROTATE_VERSION.tar blacklabelops/logrotate:$LOG_ROTATE_VERSION && echo "Saved log_rotate docker image.."
docker save -o gobase-$ETA_VERSION.tar ia/gobase:$ETA_VERSION && echo "Saved gobase docker image.."
docker save -o pybase-$ETA_VERSION.tar ia/pybase:$ETA_VERSION && echo "Saved pybase docker image.."
docker save -o gopybase-$ETA_VERSION.tar ia/gopybase:$ETA_VERSION && echo "Saved gopybase docker image.."
docker save -o data_agent-$ETA_VERSION.tar ia/data_agent:$ETA_VERSION && echo "Saved data agent docker image.."
docker save -o data_analytics-$ETA_VERSION.tar ia/data_analytics:$ETA_VERSION && echo "Saved data analytics docker image.."
docker save -o video_ingestion-$ETA_VERSION.tar ia/video_ingestion:$ETA_VERSION && echo "Saved video ingestion docker image.."
docker save -o telegraf-$ETA_VERSION.tar ia/telegraf:$ETA_VERSION && echo "Saved telegraf docker image.."
docker save -o factoryctrl_app-$ETA_VERSION.tar ia/factoryctrl_app:$ETA_VERSION && echo "Saved factory control App docker image.."
docker save -o provision-$ETA_VERSION.tar ia/provision:$ETA_VERSION && echo "Saved provision docker image.."
