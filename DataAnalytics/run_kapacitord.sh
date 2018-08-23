#!/bin/bash

socket_file="/tmp/classifier"
while [ ! -S $socket_file ]
do
    echo " Waiting for socket file: $socket_file to be bounded"
    sleep 1
done

chmod 777 /tmp/classifier

#this unsets the proxy in this shell, this isn't a system-wide unsetting
unset {http,https}_proxy
unset {HTTP,HTTPS}_PROXY

kapacitord -hostname ia_data_analytics