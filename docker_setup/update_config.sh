#!/bin/bash

### configuring provisioning related config ###
if [ -z "$1" ]; then
    # Replacing ETA DB/databus ports in provision_config.json
    sed -i 's/"influxdb_port":.*/"influxdb_port": "'"$INFLUXDB_PORT"'",/g' provision_config.json
    sed -i 's/"redis_port":.*/"redis_port": "'"$REDIS_PORT"'",/g' provision_config.json
    sed -i 's/"minio_port":.*/"minio_port": "'"$MINIO_PORT"'",/g' provision_config.json
    sed -i 's/"opcua_port":.*/"opcua_port": "'"$OPCUA_PORT"'"/g' provision_config.json   
else
    ### configuring ETA related config ###
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

    # Replacing influxdb/kapacitor ports in their respective conf files
    sed -i 's/bind-address =.*/bind-address = "'":$INFLUXDB_PORT"'"/g' config/influxdb.conf
    sed -i 's/bind-address =.*/bind-address = "'":$KAPACITOR_PORT"'"/g' config/kapacitor.conf

    # Replacing 'localhost or any other hostname' in docker_setup/config/factory.json & factory_prod.json
    # for mosquitto and postgres with host IP
    sed -i 's/"mqtt_broker_host": .*/"mqtt_broker_host": "'"$hostIP"'",/g' config/factory.json
    sed -i 's/"mqtt_broker_host": .*/"mqtt_broker_host": "'"$hostIP"'",/g' config/factory_prod.json
fi