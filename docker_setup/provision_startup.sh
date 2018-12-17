#!/bin/bash

# This scripts brings down the previous containers, builds the
# images and runs them in the dependency order using
# provision-compose.yml

source .env

echo "0.1 Copying resolv.conf to /etc/resolv.conf (On non-proxy environment, please comment below cp instruction before you start)"
cp -f resolv.conf /etc/resolv.conf

echo "0.2 Setting up $ETA_INSTALL_PATH directory and copying all the necessary config files..."
source ./init.sh

vaultFile=$ETA_INSTALL_PATH/vault_secret_file
echo "0.3 Creating empty $vaultFile..."
# Below two line will go away once TPM is in action.
touch $vaultFile
chmod 700 $vaultFile

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

# don't start containers if $1 is set - needed when starting eta.service
# to avoid unnecessary start of containers by compose_startup.sh script
if [ -z "$1" ]; then
   echo "3. Creating and starting the provisioning containers..."
   docker-compose -f provision-compose.yml up
fi

docker-compose -f provision-compose.yml down
