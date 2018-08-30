#!/bin/bash 

# Undo the set -e because we need to capture the negative returns
set +e

# At times mosquitto opens on IPv6 address only.
# May be fuser version issue. To be in safe end
# scan all protocol version.
sudo fuser 1883/tcp || sudo fuser 1883/tcp6
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
