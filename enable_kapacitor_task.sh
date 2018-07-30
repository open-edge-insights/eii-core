#!/bin/bash

# this unsets the proxy in this shell, it's not a system-wide unsetting
unset {http,https}_proxy
unset {HTTP,HTTPS}_PROXY

errCode=$(kapacitor show tasks 2>&1)

echo "Waiting for kapacitor daemon to come up..."
while [[ $errCode =~ .*refused.* ]]
do
    echo "Waiting for 1 second..."
    sleep 1
    errCode=$(kapacitor show tasks 2>&1)
done

echo "Kapacitor daemon is running now..."
kapacitor define classifier_task -tick classifier.tick
kapacitor enable classifier_task
