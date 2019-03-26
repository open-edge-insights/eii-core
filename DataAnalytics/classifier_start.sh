#!/bin/bash

configFile=$1
logLevel=$2

python3.6 classifier_startup.py --log-dir /IEI/classifier_logs --config $configFile --log $logLevel
