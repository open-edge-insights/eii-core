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

echo "Updating container timezone..."
# Get Docker Host timezone
hostTimezone=`timedatectl status | grep "zone" | sed -e 's/^[ ]*Time zone: \(.*\) (.*)$/\1/g'`
hostTimezone=`echo $hostTimezone`

echo "0. Updating databus_host field in VisualHmiClient/config.json..."
sed -i 's/"databus_host": .*/"databus_host": "'"$ETA_IP_ADDR"'",/g' VisualHmiClient/config.json

echo "1. Building VisualHmiClient image..."
cp docker_setup/dockerignores/.dockerignore .
docker build -f VisualHmiClient/Dockerfile --build-arg HOST_TIME_ZONE="$hostTimezone" -t visual_hmi .
errorCode=`echo $?`
if [ $errorCode != "0" ]; then
    echo "docker build failed..."
    exit -1
fi

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
