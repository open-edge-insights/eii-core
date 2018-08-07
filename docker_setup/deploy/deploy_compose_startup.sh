#!/bin/bash

# This scripts brings down the previous containers
# and runs them in the dependency order using 
# docker-compose.yml
echo "0.1 Copying resolv.conf to /etc/resolv.conf (On non-proxy environment, please comment below cp instruction before you start)"
cp -f resolv.conf /etc/resolv.conf

echo "0.2 Adding Docker Host IP Address to config/DataAgent.conf, config/factory.json and config/factory_cam.json files..."
# Get Docker Host IP Address
hostIP=`hostname -I | awk '{print $1}'`

# Replacing 'localhost or host ip address' in docker_setup/config/DataAgent.conf with host IP
sed -i 's/Host = .*/Host = "'"$hostIP"'"/g' config/DataAgent.conf

# Replacing 'localhost or any other hostname' in docker_setup/config/factory.json & factory_cam.json
# for mosquitto and postgres with host IP
sed -i 's/"mqtt_broker_host": .*/"mqtt_broker_host": "'"$hostIP"'",/g' config/factory.json
sed -i 's/"mqtt_broker_host": .*/"mqtt_broker_host": "'"$hostIP"'",/g' config/factory_cam.json

echo "0.3 Setting up /var/lib/eta directory and copying all the necessary config files..."
source ./init.sh

echo "1. Removing previous dependency/eta containers if existed..."
docker-compose down 

echo "2. Creating and starting the dependency/eta containers..."
docker-compose up -d

echo "3. Starting the kapacitor daemon in the ia_data_analytics container..."
docker exec -d ia_data_analytics ./run_kapacitord.sh

echo "4. Defining and enabling the classifier task in the ia_data_analytics container..."
docker exec -d ia_data_analytics ./enable_kapacitor_task.sh

echo "5. Restarting the ia_video_ingestion container..."
docker restart ia_video_ingestion