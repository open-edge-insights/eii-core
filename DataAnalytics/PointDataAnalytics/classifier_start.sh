#!/bin/bash

configFile=$1
logLevel=$2
devMode=$3

python3.6 classifier_startup.py --log-dir /IEI/classifier_logs --config $configFile --log $logLevel --dev-mode $devMode