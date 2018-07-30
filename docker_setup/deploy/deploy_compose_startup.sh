#!/bin/bash

# This scripts brings down the previous containers
# and runs them in the dependency order using 
# docker-compose.yml

# Get Docker Host IP Address
hostIP=`hostname -I | awk '{print $1}'`

# Replacing 'localhost or host ip address' in docker_setup/config/DataAgent.conf with host IP
sed -i 's/Host = .*/Host = "'"$hostIP"'"/g' config/DataAgent.conf

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