#!/bin/bash

# This scripts brings down the previous containers, builds the
# images and runs them in the dependency order using
# provision-compose.yml

source .env

if [ ! -n "$1" ]
then
  echo "USAGE: `basename $0` <PATH_TO_CERTIFICATES_DIRECTORY> NOTE: certificate directory must be generated by cert tool"
  echo "Example: provision_startup.sh ../cert-tool/Certificates/"
  exit 1
fi

if [ -d "$1" ]
then
    # TODO : We can check directory sanity as per cert-tool requirement here
    export CERTIFICATES_PATH=$(dirname $(readlink -e $1))/$(basename $1)
else
    echo "$1 directory doesn't exist"
fi

echo "0.1 Copying resolv.conf to /etc/resolv.conf (On non-proxy environment, please comment below cp instruction before you start)"
cp -f resolv.conf /etc/resolv.conf

echo "0.2 Setting up $ETA_INSTALL_PATH directory and copying all the necessary config files..."
source ./init.sh



echo "0.4 Updating .env for container timezone..."
# Get Docker Host timezone
hostTimezone=`timedatectl status | grep "zone" | sed -e 's/^[ ]*Time zone: \(.*\) (.*)$/\1/g'`
hostTimezone=`echo $hostTimezone`

# This will remove the HOST_TIME_ZONE entry if it exists and adds a new one with the right timezone
sed -i '/HOST_TIME_ZONE/d' .env && echo "HOST_TIME_ZONE=$hostTimezone" >> .env

echo "1. Removing previous provisioning containers if existed..."
docker-compose down --remove-orphans
docker-compose -f provision-compose.yml down

echo "2. Buidling the provisioning containers..."

# set .dockerignore to the base one
ln -sf docker_setup/dockerignores/.dockerignore ../.dockerignore

services=(ia-gobase ia_provision)
servDockerIgnore=(.dockerignore.common .dockerignore.provision)

count=0
echo "services: ${services[@]}"
for service in "${services[@]}"
do
    echo "Building $service image..."
    ln -sf docker_setup/dockerignores/${servDockerIgnore[$count]} ../.dockerignore
    docker-compose -f provision-compose.yml build --build-arg HOST_TIME_ZONE="$hostTimezone" $service
    errorCode=`echo $?`
    if [ $errorCode != "0" ]; then
        echo "docker-compose build failed for $service..."
        exit -1
    fi
    count=$((count+1))
done

echo "3. Creating and starting the provisioning containers..."
docker-compose -f provision-compose.yml up

docker-compose -f provision-compose.yml down
