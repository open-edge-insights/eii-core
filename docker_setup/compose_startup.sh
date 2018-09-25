#!/bin/bash

# This scripts brings down the previous containers, builds the
# images and runs them in the dependency order using 
# docker-compose.yml

echo "0.1 Copying resolv.conf to /etc/resolv.conf (On non-proxy environment, please comment below cp instruction before you start)"
cp -f resolv.conf /etc/resolv.conf

echo "0.2 Adding Docker Host IP Address to config/DataAgent.conf, config/factory.json and config/factory_cam.json files..."
source ./update_config.sh

echo "0.3 Setting up /var/lib/eta directory and copying all the necessary config files..."
source ./init.sh

echo "0.4 Creating the external dist_libs client package..."
./create_client_dist_package.sh

echo "0.5 Updating .env for container timezone..."
# Get Docker Host timezone
hostTimezone=`timedatectl status | grep "zone" | sed -e 's/^[ ]*Time zone: \(.*\) (.*)$/\1/g'`
hostTimezone=`echo $hostTimezone`

echo "0.6 Checking if mosquitto is up..."
./start_mosquitto.sh

echo "1. Removing previous dependency/eta containers if existed..."
docker-compose down 

echo "2. Buidling the dependency/eta containers..."

# set .dockerignore to the base one
ln -sf docker_setup/dockerignores/.dockerignore ../.dockerignore

services=(ia_influxdb ia_redis ia-gobase ia-pybase ia-gopybase ia_data_agent ia_data_analytics ia_video_ingestion)
servDockerIgnore=(.dockerignore.common .dockerignore.common .dockerignore.common .dockerignore.common .dockerignore.common .dockerignore.da .dockerignore.classifier .dockerignore.vi)

count=0
echo "services: ${services[@]}"
for service in "${services[@]}"
do
    echo "Building $service image..."
    ln -sf docker_setup/dockerignores/${servDockerIgnore[$count]} ../.dockerignore
	docker-compose build --build-arg HOST_TIME_ZONE="$hostTimezone" $service
    count=$((count+1))
done

# Relinking back to the base one
ln -sf docker_setup/dockerignores/.dockerignore ../.dockerignore

echo "3. Creating and starting the dependency/eta containers..."
docker-compose up -d

#Logging the docker compose logs to file.
DATE=`echo $(date '+%Y-%m-%d_%H:%M:%S,%3N')`
docker-compose logs -f &> $etaLogDir/consolidatedLogs/eta_$DATE.log &
