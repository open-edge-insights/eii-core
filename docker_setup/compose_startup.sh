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

echo "0.4 Checking if mosquitto is up..."
./start_mosquitto.sh

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

#Logging the docker compose logs to file.
DATE=`echo $(date '+%Y-%m-%d_%H:%M:%S,%3N')`
docker-compose logs -f &> $etaLogDir/consolidatedLogs/eta_$DATE.log &
