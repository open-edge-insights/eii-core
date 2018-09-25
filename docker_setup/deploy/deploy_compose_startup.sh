#!/bin/bash

# This scripts brings down the previous containers
# and runs them in the dependency order using
# docker-compose.yml

# It also logs the docker compose logs to file

echo "1. Removing previous dependency/eta containers if existed..."
docker-compose down

source ./init.sh

echo "2. Creating and starting the dependency/eta containers..."
#DATE=`echo $(date '+%Y-%m-%d_%H:%M:%S,%3N')`
#docker-compose up &> $etaLogDir/consolidatedLogs/eta_$DATE.log
docker-compose up
