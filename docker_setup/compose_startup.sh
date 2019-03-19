#!/bin/bash

# This scripts brings down the previous containers, builds the
# images and runs them in the dependency order using
# docker-compose.yml

pre_build_steps() {
	source .env

	echo "Checking if required ports are already up..."
	apt-get -y -qq install lsof
	ports=($GRPC_EXTERNAL_PORT $GRPC_INTERNAL_PORT $OPCUA_PORT $INFLUXDB_PORT $REDIS_PORT $IMAGESTORE_PORT $MINIO_PORT $KAPACITOR_PORT)
	for port in "${ports[@]}"
	do
		lsof -t -i:$port
		if [ $? -eq 0 ]; then
			echo "$port already up. Stop process using $port. Exiting build...."
			exit -1
		fi
	done

	if ! id $IEI_USER_NAME >/dev/null 2>&1;
	then
		echo "User for IEI does not exist on host machine, please execute provision."
		exit 1
	fi

	echo "0.1 Setting up $IEI_INSTALL_PATH directory and copying all the necessary config files..."
	if [ "$1" = "deploy_mode" ]
	then
		source ./setenv.sh
	else
		source ./init.sh
	fi

	echo "0.2 Updating .env for container timezone..."
	# Get Docker Host timezone
	hostTimezone=`timedatectl status | grep "zone" | sed -e 's/^[ ]*Time zone: \(.*\) (.*)$/\1/g'`
	hostTimezone=`echo $hostTimezone`

	# This will remove the HOST_TIME_ZONE entry if it exists and adds a new one with the right timezone
	sed -i '/HOST_TIME_ZONE/d' .env && echo "HOST_TIME_ZONE=$hostTimezone" >> .env

	echo "0.3 create $COMPOSE_PROJECT_NAME if it doesn't exists"
	if [ ! "$(docker network ls | grep -w $COMPOSE_PROJECT_NAME)" ]; then
		docker network create $COMPOSE_PROJECT_NAME
	fi

	echo "0.4 Generating shared key and nonce for grpc internal secrets..."
	source ./set_shared_key_nonce_env_vars.sh

	if [ "$TPM_ENABLE" = "true" ]
	then
		OVERRIDE_COMPOSE_YML="-f docker-compose.yml -f docker-compose.tpm.yml"
		# Intentionally not chnaging the group of device file to keep the
		# root group based users unaffected and keep the host machine setting change minimal.
		# TODO: Revert the changes the after the TPM read is over forever.
		chown $IEI_USER_NAME /dev/tpm0
	fi

	echo "0.5 Get docker Host IP address and write it to .env"
	echo "If automatic IP discovery is not working well, comment this line and update HOST_IP in .env file"
	./update_host_ip.sh
}

post_build_steps() {

	mkdir -p $IEI_INSTALL_PATH/grpc_int_ssl_secrets
	chown -R $IEI_USER_NAME:$IEI_USER_NAME $IEI_INSTALL_PATH/grpc_int_ssl_secrets
	mkdir -p $IEI_INSTALL_PATH/data
	mkdir -p $IEI_INSTALL_PATH/data/influxdata
	chown $IEI_USER_NAME:$IEI_USER_NAME $IEI_INSTALL_PATH/data
	chown $IEI_USER_NAME:$IEI_USER_NAME $IEI_INSTALL_PATH/data/influxdata
	chmod -R 760 $IEI_INSTALL_PATH/data
	chown $IEI_USER_NAME:$IEI_USER_NAME $IEI_INSTALL_PATH/logs
	mkdir -p $IEI_INSTALL_PATH/logs/classifier_logs
	mkdir -p $IEI_INSTALL_PATH/logs/DataAgent
	mkdir -p $IEI_INSTALL_PATH/logs/factoryctrl_app_logs
	mkdir -p $IEI_INSTALL_PATH/logs/telegraf_logs
	mkdir -p $IEI_INSTALL_PATH/logs/video_ingestion_logs
	chmod -R 777 $IEI_INSTALL_PATH/logs
}

build_iei() {

	echo "1. Removing previous dependency/iei containers if existed..."
	docker-compose down --remove-orphans

	echo "2. Building the dependency/iei containers..."
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
		# disabling error check here as any failure in building the image was aborting the script
		set +e
	    if [ "$service" == "ia-gobase" ] || [ "$service" == "ia-pybase" ]; then
	        docker-compose $OVERRIDE_COMPOSE_YML build --build-arg HOST_TIME_ZONE="$hostTimezone" $service
	    else
	        docker-compose $OVERRIDE_COMPOSE_YML build $service
	    fi
		errorCode=`echo $?`

		# error code - 100 refers to "Unable to fetch some archives, maybe run apt-get update or try with --fix-missing" error
		if [ $errorCode = "100" ]
		then
			echo "ERROR: docker-compose build failed for $service. Rebuilding it with --no-cache switch..."
			docker-compose $OVERRIDE_COMPOSE_YML build --no-cache $service
			errorCode=`echo $?`
			if [ $errorCode != "0" ]
			then
				echo "ERROR: docker-compose build --no-cache $service failed too, so exiting..."
				echo "ERROR: Error code for docker-compose build --no-cache $service : $errorCode"
				# exiting to notify users immediately, no point in proceeding further
				exit -1
			fi
		elif [ $errorCode != "0" ]
		then
			echo "ERROR: Error code for docker-compose build for $service : $errorCode"
			# exiting to notify users immediately, no point in proceeding further
			exit -1
		fi
		set -e
	    count=$((count+1))
	done

	# unlinking .dockerignore
	unlink ../.dockerignore
}

# don't start containers if $1 is set - needed when starting iei.service
# to avoid unnecessary start of containers by compose_startup.sh script
up_iei() {

	if [ -e $ieiLogDir/consolidatedLogs/iei.log ]; then
	    DATE=`echo $(date '+%Y-%m-%d_%H:%M:%S,%3N')`
	    mv $ieiLogDir/consolidatedLogs/iei.log $ieiLogDir/consolidatedLogs/iei_$DATE.log.bkp
	fi

	if [ "$1" = "deploy_mode" ]
	then
		#Logging the docker compose logs to file.
		docker-compose $OVERRIDE_COMPOSE_YML up &> $ieiLogDir/consolidatedLogs/iei.log
	else
		echo "3. Creating and starting the dependency/iei containers..."
		docker-compose $OVERRIDE_COMPOSE_YML up &> $ieiLogDir/consolidatedLogs/iei.log &
	fi
}

down_iei() {
	echo "Shutting down & Removing iei & dependent containers"
	docker-compose down
}


# While loop is kept as place holder for future implementaion to suuport more
# one argument in a single command line.
while [[ $# -gt 0 ]]
do
key="$1"
case $key in
	-d|--down)
	down_iei
	shift # past argument
	exit 0
	;;
	-u|--up)
	deploy_mode="$2"
	pre_build_steps "$deploy_mode"
	post_build_steps
	down_iei
	up_iei "$deploy_mode"
	shift # past argument
	if [ "$2" != "" ]
	then
		shift # past the value
	fi
	exit 0
	;;
	-b|--build)
	pre_build_steps
	build_iei
	post_build_steps
	shift # past argument
	exit 0
	# this is place holder for future implementations
	;;
	--default)
	DEFAULT=YES
	shift # past argument
	;;
esac
done

pre_build_steps
build_iei
post_build_steps
up_iei
# end of the file
