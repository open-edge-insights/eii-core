#!/bin/bash

socket_file="/tmp/classifier"
while [ ! -S $socket_file ]
do
    echo " Waiting for socket file: $socket_file to be bounded"
    sleep 1
done

chmod 777 /tmp/classifier

kapacitord -hostname ia_data_analytics