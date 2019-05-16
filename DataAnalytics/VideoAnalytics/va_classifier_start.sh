#!/bin/bash

configFile=$1
logLevel=$2

source /opt/intel/openvino/bin/setupvars.sh

udevadm control --reload-rules
udevadm trigger

# Uncomment these lines if you are using HDDL
#$HDDL_INSTALL_DIR/bin/hddldaemon &
#sleep 20

python3.6 VideoAnalytics.py --log-dir /IEI/video_analytics_logs  --config $configFile --log $logLevel
