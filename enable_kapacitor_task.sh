#!/bin/bash

errCode=$(kapacitor show tasks 2>&1)

echo "Waiting for kapacitor daemon to come up..."
while [[ $errCode =~ .*refused.* ]]
do
    echo "Waiting for 1 second..."
    sleep 1
    errCode=$(kapacitor show tasks 2>&1)
done

echo "Kapacitor daemon is running now..."
kapacitor/kapacitor define classifier_task -tick classifier.tick
kapacitor/kapacitor enable classifier_task
