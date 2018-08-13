#!/bin/bash -x

# This scripts brings down the previous containers
# and runs them in the dependency order using
# docker-compose.yml

echo "0.1 Copying resolv.conf to /etc/resolv.conf (On non-proxy environment, please comment below cp instruction before you start)"
cp -f resolv.conf /etc/resolv.conf

echo "0.2 Adding Docker Host IP Address to config/DataAgent.conf, config/factory.json and config/factory_cam.json files..."
source ./update_config.sh

echo "0.3 Setting up /var/lib/eta directory and copying all the necessary config files..."
if [[ -d /var/lib/eta && -n "$(ls -A /var/lib/eta)" ]]; then
    echo "Found a non-empty /var/lib/eta folder, Avoid overriding the config..."
else
    source ./init.sh
fi

for (( ; ; ))
do
    sudo fuser 1883/tcp
    exit_code=`echo $?`
    if [ $exit_code -eq "1" ]
    then
        echo "Waiting for mosquitto port to be up"
        sleep 3
    else
        echo "Mosquitto port is UP, hence starting ETA containers"
        break
    fi
done

echo "1. Removing previous dependency/eta containers if existed..."
docker-compose down

echo "2. Creating and starting the dependency/eta containers..."
docker-compose up -d

echo "3. Starting the kapacitor daemon in the ia_data_analytics container..."
docker exec -d ia_data_analytics ./run_kapacitord.sh

echo "4. Defining and enabling the classifier task in the ia_data_analytics container..."
docker exec -d ia_data_analytics ./enable_kapacitor_task.sh

#Logging the docker compose logs to file.
DATE=`echo $(date '+%Y-%m-%d_%H:%M:%S,%3N')`
docker-compose logs -f &> $etaLogDir/consolidatedLogs/eta_$DATE.log &

for (( ; ; ))
do
   sleep 1
done
