#!/bin/bash

# This scripts brings down the previous containers
# and runs them in the dependency order using
# docker-compose.yml

# It also logs the docker compose logs to file

source ./setenv.sh

echo "0.1 Copying resolv.conf to /etc/resolv.conf (On non-proxy environment, please comment below cp instruction before you start)"
cp -f resolv.conf /etc/resolv.conf

echo "0.2 Adding Docker Host IP Address to config/DataAgent.conf, config/factory.json and config/factory_prod.json files..."
source ./update_config.sh

echo "0.3 Updating .env for container timezone..."
# Get Docker Host timezone
hostTimezone=`timedatectl status | grep "zone" | sed -e 's/^[ ]*Time zone: \(.*\) (.*)$/\1/g'`
hostTimezone=`echo $hostTimezone`

#
# This will remove the HOST_TIME_ZONE entry if it exists and adds a new one with the right timezone
sed -i '/HOST_TIME_ZONE/d' .env && echo "HOST_TIME_ZONE=$hostTimezone" >> .env

# this two changes will go away once we merge DA and vault container, temporary change. 
touch /opt/intel/eta/vault_secret_file
chmod 700 /opt/intel/eta/vault_secret_file

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
