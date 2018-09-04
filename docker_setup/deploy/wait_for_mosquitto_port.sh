#!/bin/bash

# This script waits for mosquitto port to be up
# before starting other ETA containers. It starts
# the ETA containers normally if its already up.

# To debug, uncomment below line
# set -x

# Undo the set-e for capturing the negative error code.
set +e
for (( ; ; ))
do
    # At times mosquitto opens on IPv6 address only.
    # May be fuser version issue. To be in safe end
    # scan all protocol version.
    sudo fuser 1883/tcp || sudo fuser 1883/tcp6
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
set -e