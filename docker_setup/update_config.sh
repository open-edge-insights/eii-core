#!/bin/bash

# Get Docker Host IP Address
set +e
hostIP=`hostname -I | awk '{print $1}'`
if [ ! "$hostIP" ]; then
    echo "hostname command fail to obtain IP address..."
    echo "Using user populated .env file to obtain IP address..."
    hostIP=`grep HOST_IP .env | cut -d= -f2`
    if [ ! "$hostIP" ]; then
        cat << EOF
        NO IP ADDRESS FOUND IN .env file.
        Please update the IP address present in INSTALL PATH's docker_setup/.env file.
        And add the proper IP ADDRESS of the host against the HOST_IP
        field. For example the line in file should look like below
                 HOST_IP=10.223.97.5
        Kindly re-run the ./setup_eta -i if you are installing with "-i" option.
EOF
	exit -1
    fi
fi

set -e
# Replacing 'localhost or host ip address' in docker_setup/config/DataAgent.conf with host IP
sed -i 's/\"host\": .*/\"host\": "'"$hostIP"'",/g' ./provision_config.json

# Replacing 'localhost or any other hostname' in docker_setup/config/factory.json & factory_prod.json
# for mosquitto and postgres with host IP
sed -i 's/"mqtt_broker_host": .*/"mqtt_broker_host": "'"$hostIP"'",/g' config/factory.json
sed -i 's/"mqtt_broker_host": .*/"mqtt_broker_host": "'"$hostIP"'",/g' config/factory_prod.json

# Replacing 'localhost or host ip address' in docker_setup/config/DataAgent.conf with host IP
sed -i 's/HOST_IP=.*/HOST_IP='$hostIP'/g' .env

