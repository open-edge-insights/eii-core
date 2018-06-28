#!/bin/bash

etaRootDir="/var/lib/eta"
etaLogDir=$etaRootDir/logs
etaConfDir=$etaRootDir/config
etaDataDir=$etaRootDir/data

rootDir="../.."
testFile="yumei_trigger.avi"
configDir="../config"

# Creating eta directory
mkdir -p $etaRootDir
mkdir -p $etaLogDir
mkdir -p $etaConfDir

mkdir -p $etaDataDir
mkdir -p $etaDataDir/kapacitor
mkdir -p $etaDataDir/influxdb
mkdir -p $etaDataDir/classifier

# Copy config files
cp -f $configDir/influxdb.conf $etaConfDir
cp -f $configDir/redis.conf $etaConfDir
cp -f $configDir/kapacitor.conf $etaConfDir
cp -f $rootDir/DataAgent/DataAgent.conf $etaConfDir
cp -f $configDir/mosquitto.conf $etaConfDir
cp -f $rootDir/factory.json $etaConfDir

# Copy avi file
cp -f $rootDir/$testFile $etaRootDir/