#!/bin/bash

socket_file="/tmp/point_classifier"
while [ ! -S $socket_file ]
do
    echo " Waiting for socket file: $socket_file to be bounded"
    sleep 1
done

chmod 777 /tmp/point_classifier
if [-z $DEV_MODE]
then
    kapacitord -config /etc/kapacitor/kapacitor.conf -hostname ${KAPACITOR_SERVER}
else
    kapacitord -config /etc/kapacitor/kapacitor_devmode.conf -hostname ${KAPACITOR_SERVER} 
