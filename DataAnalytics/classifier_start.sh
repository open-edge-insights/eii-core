#!/bin/bash

configFile=$1
logLevel=$2
source /opt/intel/computer_vision_sdk_2018.4.420/bin/setupvars.sh

python3.6 classifier_startup.py --log-dir /ETA/classifier_logs --config $configFile --log $logLevel
