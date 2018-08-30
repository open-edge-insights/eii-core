#!/bin/bash 

# Undo the set -e because we need to capture the negative returns
set +e

sudo fuser 1883/tcp
exit_code=`echo $?`
# Reverting because we need to stop the script if docker run fails
set -e

if [ $exit_code -eq "1" ]
then
    echo "Starting mosquitto container..."
    docker run -d -p 1883:1883 eclipse-mosquitto:1.4.12
else
    echo "mosquitto is already up and running, so not starting mosquitto container..."
fi
