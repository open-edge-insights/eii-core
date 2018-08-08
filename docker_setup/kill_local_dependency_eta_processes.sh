#!/bin/bash -x

# Kill all processes running natively on the host m/c
echo "Killing all dependency processes..."
sudo service influxd stop
sudo service redis stop
sudo service mosquitto stop
sudo service postgresql stop
sudo service kapacitor stop
pkill -9 -f kapacitord

echo "Killing all eta processes..."
pkill -9 -f DataAgent.go
pkill -9 -f VideoIngestion.py
pkill -9 -f classifier.py
