#!/bin/bash

logLevel=$1
devMode=$2

python3.6 classifier_startup.py --log-dir /EIS/classifier_logs --log $logLevel --dev-mode $devMode
