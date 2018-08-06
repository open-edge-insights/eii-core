#!/bin/bash

# This scripts brings down the previous containers, builds the
# images and runs them in the dependency order using 
# docker-compose.yml

# copies resolv.conf 
cp -f resolv.conf /etc/resolv.conf

# Get Docker Host IP Address
hostIP=`hostname -I | awk '{print $1}'`

# Replacing 'localhost or host ip address' in docker_setup/config/DataAgent.conf with host IP
sed -i 's/Host = .*/Host = "'"$hostIP"'"/g' config/DataAgent.conf

# Replacing 'localhost or any other hostname' in docker_setup/config/factory.json & factory_cam.json
# for mosquitto and postgres with host IP
sed -i 's/"mqtt_broker_host": .*/"mqtt_broker_host": "'"$hostIP"'",/g' config/factory.json
sed -i 's/"mqtt_broker_host": .*/"mqtt_broker_host": "'"$hostIP"'",/g' config/factory_cam.json

source ./init.sh

echo "1. Removing previous dependency/eta containers if existed..."
docker-compose down 

echo "2. Buidling the dependency/eta containers..."
docker-compose build

echo "3. Creating and starting the dependency/eta containers..."
docker-compose up -d

echo "4. Starting the kapacitor daemon in the ia_data_analytics container..."
docker exec -d ia_data_analytics ./run_kapacitord.sh

echo "5. Defining and enabling the classifier task in the ia_data_analytics container..."
docker exec -d ia_data_analytics ./enable_kapacitor_task.sh

echo "6. Restarting the ia_video_ingestion container..."
docker restart ia_video_ingestion
