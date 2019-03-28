#!/bin/bash

configFile=$1
logLevel=$2
source /opt/intel/computer_vision_sdk/bin/setupvars.sh

python3.6 classifier_startup.py --log-dir /IEI/classifier_logs --config $configFile --log $logLevel
