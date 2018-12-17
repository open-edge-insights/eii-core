#!/bin/bash

# Creates the ETA_INSTALL_PATH directory and other sub-dirs here like logs, config
# data, test_videos which will be used during volume mount of containers

source ./setenv.sh

# Creating eta directory
mkdir -p $etaRootDir
mkdir -p $etaLogDir
mkdir -p $etaConfDir

# Creating eta log directory
mkdir -p $etaLogDir/consolidatedLogs

# Copy config files
cp -rf $configDir $etaRootDir

# Copy test video files
if [ -d "$rootDir/test_videos" ]; then
    echo "test_videos exist..."
    cp -rf $rootDir/test_videos $etaRootDir/
fi

# Copy ETA Certificates folder
if [ -d "$rootDir/cert-tool/Certificates" ]; then
    echo "Certificates folder exist..."
    cp -rf $rootDir/cert-tool/Certificates $etaRootDir/
fi
