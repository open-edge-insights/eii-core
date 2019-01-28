#!/bin/bash

# Creates the IEI_INSTALL_PATH directory and other sub-dirs here like logs, config
# data, test_videos which will be used during volume mount of containers

source ./setenv.sh

# Creating iei directory
mkdir -p $ieiRootDir
mkdir -p $ieiLogDir
mkdir -p $ieiConfDir

# Creating iei log directory
mkdir -p $ieiLogDir/consolidatedLogs


# Copy config files
cp -rf $configDir $ieiRootDir

# Copy test video files
if [ -d "$rootDir/test_videos" ]; then
    echo "test_videos exist..."
    cp -rf $rootDir/test_videos $ieiRootDir/
fi

