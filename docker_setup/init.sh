#!/bin/bash -x

# Creates the /var/lib/eta directory and other sub-dirs here like logs, config
# data, test_videos which will be used during volume mount of containers

source ./setenv.sh

# Creating eta directory
mkdir -p $etaRootDir
mkdir -p $etaLogDir
mkdir -p $etaConfDir

mkdir -p $etaDataDir
mkdir -p $etaDataDir/kapacitor
mkdir -p $etaDataDir/influxdb
mkdir -p $etaDataDir/classifier
mkdir -p $etaLogDir/kapacitor

# Copy config files
cp -f $configDir/influxdb.conf $etaConfDir
cp -f $configDir/redis.conf $etaConfDir
cp -f $configDir/kapacitor.conf $etaConfDir
cp -f $configDir/mosquitto.conf $etaConfDir
cp -f $configDir/DataAgent.conf $etaConfDir

cp -f $configDir/factory.json $etaConfDir
cp -f $configDir/factory_cam.json $etaConfDir

# Copy test video files
cp -rf $rootDir/test_videos $etaRootDir/
