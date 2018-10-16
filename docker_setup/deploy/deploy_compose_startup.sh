#!/bin/bash

# This scripts brings down the previous containers
# and runs them in the dependency order using
# docker-compose.yml

# It also logs the docker compose logs to file

source ./setenv.sh

echo "1. Removing previous dependency/eta containers if existed..."
docker-compose down

echo "2. Creating and starting the dependency/eta containers..."
mkdir -p $etaLogDir/consolidatedLogs

if [ -e $etaLogDir/consolidatedLogs/eta.log ]; then
    DATE=`echo $(date '+%Y-%m-%d_%H:%M:%S,%3N')`
    mv $etaLogDir/consolidatedLogs/eta.log $etaLogDir/consolidatedLogs/eta_$DATE.log.bkp
fi

#Logging the docker compose logs to file.
docker-compose up &> $etaLogDir/consolidatedLogs/eta.log