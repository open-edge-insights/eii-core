#!/bin/bash

# to debug uncomment below line
# set -x

# Starting influxdb service
if [ "$1" = "dev_mode" ]; then
    influxd -config /etc/influxdb/influxdb_devmode.conf &> influxd.log &
else
    influxd -config /etc/influxdb/influxdb.conf &> influxd.log &
fi
