#!/bin/bash
source dep/.env
echo "1. create $EIS_USER_NAME if it doesn't exists. Update UID from env if already exits with different UID"

# EIS containers will be executed as eisuser
if ! id $EIS_USER_NAME >/dev/null 2>&1; then
    groupadd $EIS_USER_NAME -g $EIS_UID
    useradd -r -u $EIS_UID -g $EIS_USER_NAME $EIS_USER_NAME
else
    if ! [ $(id -u $EIS_USER_NAME) = $EIS_UID ]; then
        usermod -u $EIS_UID $EIS_USER_NAME
        groupmod -g $EIS_UID $EIS_USER_NAME
    fi
fi


echo "2. Creating Required Directories"

if $ETCD_RESET = "true"; then
    rm -r $EIS_INSTALL_PATH/data/etcd
fi

mkdir -p $EIS_INSTALL_PATH/data/etcd
mkdir -p $EIS_INSTALL_PATH/sockets/
chown -R $EIS_USER_NAME:$EIS_USER_NAME $EIS_INSTALL_PATH



echo "2. Copying docker compose yaml file which is provided as argument."
# This file will be volume mounted inside the provisioning container and deleted once privisioning it done

cp $1 ./docker-compose.yml

echo "3. Starting and provisioning ETCD ..."

docker-compose -f dep/docker-compose-provision.yml up --build -d

rm ./docker-compose.yml
