#!/bin/bash
source ./setenv.sh
mkdir -p $PWD/deploy/docker_images
cd $PWD/deploy/docker_images
curDir=`pwd`

echo "Saving all the docker images to $curDir folder..."

# saving all docker images of iei along with its dependencies
docker save -o log_rotate-$LOG_ROTATE_VERSION.tar blacklabelops/logrotate:$LOG_ROTATE_VERSION && echo "Saved log_rotate docker image.."
docker save -o gobase-$IEI_VERSION.tar ia/gobase:$IEI_VERSION && echo "Saved gobase docker image.."
docker save -o pybase-$IEI_VERSION.tar ia/pybase:$IEI_VERSION && echo "Saved pybase docker image.."
docker save -o gopybase-$IEI_VERSION.tar ia/gopybase:$IEI_VERSION && echo "Saved gopybase docker image.."
docker save -o data_agent-$IEI_VERSION.tar ia/data_agent:$IEI_VERSION && echo "Saved data agent docker image.."
docker save -o data_analytics-$IEI_VERSION.tar ia/data_analytics:$IEI_VERSION && echo "Saved data analytics docker image.."
docker save -o video_ingestion-$IEI_VERSION.tar ia/video_ingestion:$IEI_VERSION && echo "Saved video ingestion docker image.."
docker save -o telegraf-$IEI_VERSION.tar ia/telegraf:$IEI_VERSION && echo "Saved telegraf docker image.."
docker save -o factoryctrl_app-$IEI_VERSION.tar ia/factoryctrl_app:$IEI_VERSION && echo "Saved factory control App docker image.."
docker save -o provision-$IEI_VERSION.tar ia/provision:$IEI_VERSION && echo "Saved provision docker image.."
