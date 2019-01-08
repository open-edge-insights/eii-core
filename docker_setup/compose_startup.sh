#!/bin/bash

# This scripts brings down the previous containers, builds the
# images and runs them in the dependency order using
# docker-compose.yml

source .env

echo "0.1 Copying resolv.conf to /etc/resolv.conf (On non-proxy environment, please comment below cp instruction before you start)"
cp -f resolv.conf /etc/resolv.conf

echo "0.2 Setting up $ETA_INSTALL_PATH directory and copying all the necessary config files..."
source ./init.sh

echo "0.3 Updating .env for container timezone..."
# Get Docker Host timezone
hostTimezone=`timedatectl status | grep "zone" | sed -e 's/^[ ]*Time zone: \(.*\) (.*)$/\1/g'`
hostTimezone=`echo $hostTimezone`

# This will remove the HOST_TIME_ZONE entry if it exists and adds a new one with the right timezone
sed -i '/HOST_TIME_ZONE/d' .env && echo "HOST_TIME_ZONE=$hostTimezone" >> .env

# This will remove the eta user id entry if it exists and adds a new one with the right eta user id
sed -i '/ETA_UID/d' .env && echo "ETA_UID=$(id -u $ETA_USER_NAME)" >> .env


echo "0.4 create $COMPOSE_PROJECT_NAME if it doesn't exists"
if [ ! "$(docker network ls | grep -w $COMPOSE_PROJECT_NAME)" ]; then
        docker network create $COMPOSE_PROJECT_NAME
fi

echo "0.5 Generating shared key and nonce for grpc internal secrets..."
source ./set_shared_key_nonce_env_vars.sh

echo "0.6 Get docker Host IP address and write it to .env"
./update_host_ip.sh

if [ "$TPM_ENABLE" = "true" ]
then
	OVERRIDE_COMPOSE_YML="-f docker-compose.yml -f docker-compose.tpm.yml"
	# Intentionally not chnaging the group of device file to keep the
	# root group based users unaffected and keep the host machine setting change minimal.
	# TODO: Revert the chnages the after the TPM read is over forever.
	chown $ETA_USER_NAME /dev/tpm0
	chown $ETA_USER_NAME /dev/tpmrm0
fi

echo "1. Removing previous dependency/eta containers if existed..."
docker-compose down --remove-orphans

echo "2. Building the dependency/eta containers..."

# set .dockerignore to the base one
ln -sf docker_setup/dockerignores/.dockerignore ../.dockerignore

services=(ia_log_rotate ia-gobase ia-pybase ia-gopybase ia_data_agent ia_imagestore ia_data_analytics ia_factoryctrl_app ia_video_ingestion ia_telegraf)
servDockerIgnore=(.dockerignore.common .dockerignore.common .dockerignore.common .dockerignore.common .dockerignore.da .dockerignore.imagestore .dockerignore.classifier .dockerignore.factoryctrlapp .dockerignore.vi .dockerignore.telegraf)

count=0
echo "services: ${services[@]}"
for service in "${services[@]}"
do
    echo "Building $service image..."
    ln -sf docker_setup/dockerignores/${servDockerIgnore[$count]} ../.dockerignore
    docker-compose $OVERRIDE_COMPOSE_YML build --build-arg HOST_TIME_ZONE="$hostTimezone" $service
    errorCode=`echo $?`
    if [ $errorCode != "0" ]; then
        echo "docker-compose build failed for $service..."
        exit -1
    fi
    count=$((count+1))
done

# unlinking .dockerignore
unlink ../.dockerignore

mkdir -p $ETA_INSTALL_PATH/data
mkdir -p $ETA_INSTALL_PATH/data/influxdata
mkdir -p $ETA_INSTALL_PATH/grpc_int_ssl_secrets
chown -R $ETA_USER_NAME:$ETA_USER_NAME $ETA_INSTALL_PATH


# don't start containers if $1 is set - needed when starting eta.service
# to avoid unnecessary start of containers by compose_startup.sh script
if [ -z "$1" ]; then
   echo "3. Creating and starting the dependency/eta containers..."
   docker-compose $OVERRIDE_COMPOSE_YML up -d

   if [ -e $etaLogDir/consolidatedLogs/eta.log ]; then
       DATE=`echo $(date '+%Y-%m-%d_%H:%M:%S,%3N')`
       mv $etaLogDir/consolidatedLogs/eta.log $etaLogDir/consolidatedLogs/eta_$DATE.log.bkp
   fi
   #Logging the docker compose logs to file.
   docker-compose logs -f &> $etaLogDir/consolidatedLogs/eta.log &

fi
chmod -R 760 $ETA_INSTALL_PATH/data
