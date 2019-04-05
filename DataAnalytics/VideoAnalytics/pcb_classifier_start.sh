#!/bin/bash

configFile=$1
logLevel=$2

source /opt/intel/computer_vision_sdk/bin/setupvars.sh

python3.6 PCBDemo.py --container-mode yes --log-dir /IEI/video_analytics_logs  --config $configFile --log $logLevel
