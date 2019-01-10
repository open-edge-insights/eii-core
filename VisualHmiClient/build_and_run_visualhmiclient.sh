#!/bin/bash

if [ "$#" -lt 3 ]
then
    fileName=`basename "$0"`
    echo "Usage: sudo ./$fileName ETA_IP_ADDR=[ETA_IP_ADDR] IMG_DIR=[IMG_DIR] LOCAL=[yes|no]"
    exit 1
fi

for ARGUMENT in "$@"
do

    KEY=$(echo $ARGUMENT | cut -f1 -d=)
    VALUE=$(echo $ARGUMENT | cut -f2 -d=)

    case "$KEY" in
            ETA_IP_ADDR)    ETA_IP_ADDR=${VALUE} ;;
            IMG_DIR)        IMG_DIR=${VALUE} ;;
            LOCAL)          LOCAL=${VALUE} ;;
            *)
    esac
done

echo "1. Building VisualHmiClient image..."
cp docker_setup/dockerignores/.dockerignore .
docker build -f VisualHmiClient/Dockerfile -t visual_hmi .

echo "2. Removing any previous VisualHmiClient container..."
docker rm -f visualhmi

if [ "$LOCAL" == "yes" ]
then
    echo "VisualHmiClient app would be running in local mode with no post to Visual HMI backend"
fi

echo "3.1 Running VisualHmiClient container..."
docker run --env no_proxy="localhost,127.0.0.1,$ETA_IP_ADDR" \
           -v $PWD/cert-tool/Certificates:/eta/cert-tool/Certificates \
           -v $PWD/VisualHmiClient/config.json:/eta/VisualHmiClient/config.json \
           -v $IMG_DIR:/root/saved_images --privileged=true \
           --network host --name visualhmi \
           -it --restart always visual_hmi -local $LOCAL
