#!/bin/bash 
set -a
source ../.env
source dep/.cluster.env
set +a

check_ETCD_port() {
	echo "Checking if ETCD ports are already up..."
	ports=($ETCD_CLIENT_PORT)
	for port in "${ports[@]}"
	do
		set +e
		fuser $port/tcp
		if [ $? -eq 0 ]; then
			echo "$port is already being used, so please kill that process and re-run the script."
			exit -1
		fi
		set -e
	done
}



echo "1 Bringing down existing ETCD container"
docker-compose -f dep/docker-compose-provision.yml down 

echo "1.2 Checking ETCD port"

check_ETCD_port


echo "2. Create $EIS_USER_NAME if it doesn't exists. Update UID from env if already exits with different UID"

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


echo "3. Creating Required Directories"

if $ETCD_RESET = "true"; then
    rm -rf $EIS_INSTALL_PATH/data/etcd
fi

mkdir -p $IEI_INSTALL_PATH/data/influxdata
mkdir -p $EIS_INSTALL_PATH/data/etcd
mkdir -p $EIS_INSTALL_PATH/sockets/
chown -R $EIS_USER_NAME:$EIS_USER_NAME $EIS_INSTALL_PATH



echo "4. Copying docker compose yaml file which is provided as argument."
# This file will be volume mounted inside the provisioning container and deleted once privisioning it done

cp $1 ./docker-compose.yml

echo "5. Starting and provisioning ETCD ..."


docker-compose -f dep/docker-compose-provision.yml up --build -d

rm ./docker-compose.yml
