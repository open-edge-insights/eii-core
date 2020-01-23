#!/bin/bash -e
# Provision slave node on EIS without using ETCD Clustering
# Usage: sudo ./slave_provision_noetcd.sh

set -a
source ../.env
set +a
if [ -z $HOST_IP ]; then
	hostIP=`hostname -I | awk '{print $1}'`
	export HOST_IP=$hostIP
fi
echo 'System IP Address is:' $HOST_IP
export no_proxy=$eis_no_proxy,$HOST_IP

echo "Updating .env for container timezone..."
# Get Docker Host timezone
hostTimezone=`timedatectl status | grep "zone" | sed -e 's/^[ ]*Time zone: \(.*\) (.*)$/\1/g'`
hostTimezone=`echo $hostTimezone`

# This will remove the HOST_TIME_ZONE entry if it exists and adds a new one with the right timezone
sed -i '/HOST_TIME_ZONE/d' ../.env && echo "HOST_TIME_ZONE=$hostTimezone" >> ../.env

echo "Create $EIS_USER_NAME if it doesn't exists. Update UID from env if already exits with different UID"

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


echo "Creating Required Directories"


if [ $ETCD_RESET = 'true' ]; then
    rm -rf $EIS_INSTALL_PATH/data/etcd
fi

mkdir -p $EIS_INSTALL_PATH/data/influxdata
mkdir -p $EIS_INSTALL_PATH/data/etcd/data
mkdir -p $EIS_INSTALL_PATH/sockets/
chown -R $EIS_USER_NAME:$EIS_USER_NAME $EIS_INSTALL_PATH

if [ -d $TC_DISPATCHER_PATH ]; then
	chown -R $EIS_USER_NAME:$EIS_USER_NAME $TC_DISPATCHER_PATH
	chmod -R 760 $TC_DISPATCHER_PATH
fi

if [ $DEV_MODE = 'false' ]; then
	chmod -R 760 $EIS_INSTALL_PATH/data/
else
	chmod -R 777 $EIS_INSTALL_PATH/data/
fi