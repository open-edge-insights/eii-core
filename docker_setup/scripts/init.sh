#!/bin/bash

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
cp -f $rootDir/DataAgent/DataAgent.conf $etaConfDir
cp -f $configDir/mosquitto.conf $etaConfDir
cp -f $rootDir/factory.json $etaConfDir
cp -f $rootDir/factory_cam.json $etaConfDir

# Copy avi file
cp -rf $rootDir/test_videos $etaRootDir/