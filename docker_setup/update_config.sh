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
sed -i 's/Host = .*/Host = "'"$hostIP"'"/g' config/DataAgent.conf

# Replacing 'localhost or any other hostname' in docker_setup/config/factory.json & factory_cam.json
# for mosquitto and postgres with host IP
sed -i 's/"mqtt_broker_host": .*/"mqtt_broker_host": "'"$hostIP"'",/g' config/factory.json
sed -i 's/"mqtt_broker_host": .*/"mqtt_broker_host": "'"$hostIP"'",/g' config/factory_cam.json

# Replacing 'localhost or host ip address' in docker_setup/config/DataAgent.conf with host IP
sed -i 's/HOSTNAME=.*/HOSTNAME='$hostIP'/g' .env
