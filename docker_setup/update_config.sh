#!/bin/bash

# Get Docker Host IP Address
hostIP=`hostname -I | awk '{print $1}'`

# Replacing 'localhost or host ip address' in docker_setup/config/DataAgent.conf with host IP
sed -i 's/Host = .*/Host = "'"$hostIP"'"/g' config/DataAgent.conf

# Replacing 'localhost or any other hostname' in docker_setup/config/factory.json & factory_cam.json
# for mosquitto and postgres with host IP
sed -i 's/"mqtt_broker_host": .*/"mqtt_broker_host": "'"$hostIP"'",/g' config/factory.json
sed -i 's/"mqtt_broker_host": .*/"mqtt_broker_host": "'"$hostIP"'",/g' config/factory_cam.json

