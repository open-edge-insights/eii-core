#!/bin/bash

source ./setenv.sh
mkdir -p $PWD/deploy/docker_images
cd $PWD/deploy/docker_images
curDir=`pwd`

echo "Loading all the docker images from $curDir folder..."

# loading all docker images of iei along with its dependencies
docker load -i log_rotate-$LOG_ROTATE_VERSION.tar && echo "Loaded log_rotate docker image.."
docker load -i gobase-$IEI_VERSION.tar && echo "Loaded gobase docker image.."
docker load -i pybase-$IEI_VERSION.tar && echo "Loaded pybase docker image.."
docker load -i gopybase-$IEI_VERSION.tar && echo "Loaded gopybase docker image.."
docker load -i data_agent-$IEI_VERSION.tar && echo "Loaded data_agent docker image.."
docker load -i data_analytics-$IEI_VERSION.tar && echo "Loaded data_analytics docker image.."
docker load -i video_ingestion-$IEI_VERSION.tar && echo "Loaded video_ingestion docker image.."
docker load -i telegraf-$IEI_VERSION.tar && echo "Loaded telegraf docker image.."
docker load -i factoryctrl_app-$IEI_VERSION.tar && echo "Loaded factory control App docker image.."
docker load -i provision-$IEI_VERSION.tar && echo "Loaded provision docker image.."
