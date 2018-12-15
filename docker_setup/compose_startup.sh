#!/bin/bash

# This scripts brings down the previous containers, builds the
# images and runs them in the dependency order using
# docker-compose.yml

source .env

echo "0.1 Copying resolv.conf to /etc/resolv.conf (On non-proxy environment, please comment below cp instruction before you start)"
cp -f resolv.conf /etc/resolv.conf

echo "0.2 Adding Docker Host IP Address to config/DataAgent.conf, config/factory.json and config/factory_prod.json files..."
source ./update_config.sh

echo "0.3 Setting up $ETA_INSTALL_PATH directory and copying all the necessary config files..."
source ./init.sh

echo "0.4 Creating the external dist_libs client package..."
./create_client_dist_package.sh

echo "0.5 Updating .env for container timezone..."
# Get Docker Host timezone
hostTimezone=`timedatectl status | grep "zone" | sed -e 's/^[ ]*Time zone: \(.*\) (.*)$/\1/g'`
hostTimezone=`echo $hostTimezone`

# This will remove the HOST_TIME_ZONE entry if it exists and adds a new one with the right timezone
sed -i '/HOST_TIME_ZONE/d' .env && echo "HOST_TIME_ZONE=$hostTimezone" >> .env


# Below two line will go away once TPM is in action.
touch /opt/intel/eta/vault_secret_file
chmod 700 /opt/intel/eta/vault_secret_file

echo "0.6 create eta_docker_network if it doesn't exists"
if [ ! "$(docker network ls | grep -w  eta_docker_network)" ]; then
        docker network create eta_docker_network
fi

echo "0.7 Checking if mosquitto is up..."
./start_mosquitto.sh


echo "1. Removing previous dependency/eta containers if existed..."
docker-compose down

echo "2. Buidling the dependency/eta containers..."

# set .dockerignore to the base one
ln -sf docker_setup/dockerignores/.dockerignore ../.dockerignore

services=(ia_log_rotate ia_influxdb ia-gobase ia-pybase ia-gopybase ia_data_agent ia_imagestore ia_data_analytics ia_yumei_app ia_video_ingestion ia_telegraf)
servDockerIgnore=(.dockerignore.common .dockerignore.common .dockerignore.common .dockerignore.common .dockerignore.common .dockerignore.da .dockerignore.redis .dockerignore.classifier .dockerignore.yumeiapp .dockerignore.vi .dockerignore.telegraf)

count=0
echo "services: ${services[@]}"
for service in "${services[@]}"
do
    echo "Building $service image..."
    ln -sf docker_setup/dockerignores/${servDockerIgnore[$count]} ../.dockerignore
    docker-compose build --build-arg HOST_TIME_ZONE="$hostTimezone" $service
    errorCode=`echo $?`
    if [ $errorCode != "0" ]; then
        echo "docker-compose build failed for $service..."
        exit -1
    fi
    count=$((count+1))
done

# unlinking .dockerignore
unlink ../.dockerignore

# don't start containers if $1 is set - needed when starting eta.service
# to avoid unnecessary start of containers by compose_startup.sh script
if [ -z "$1" ]; then
   echo "3. Creating and starting the dependency/eta containers..."
   docker-compose up -d

   if [ -e $etaLogDir/consolidatedLogs/eta.log ]; then
       DATE=`echo $(date '+%Y-%m-%d_%H:%M:%S,%3N')`
       mv $etaLogDir/consolidatedLogs/eta.log $etaLogDir/consolidatedLogs/eta_$DATE.log.bkp
   fi
   #Logging the docker compose logs to file.
   docker-compose logs -f &> $etaLogDir/consolidatedLogs/eta.log &

fi
