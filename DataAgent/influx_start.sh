#!/bin/bash

# to debug uncomment below line
set -x

# Starting influxdb service
influxd -config /etc/influxdb/influxdb.conf &> influxd.log &
