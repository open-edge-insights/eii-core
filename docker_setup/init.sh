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
if [ -d "./test_videos" ]; then
    echo "test_videos exist..."
    cp -rf ./test_videos $ieiRootDir/
fi

# Get list of services and respective dockerignores.

# List of core IEI services and respective dockerignores.
services+=(ia_log_rotate)
services+=(ia_gobase)
services+=(ia_pybase)
services+=(ia_gopybase)
services+=(ia_data_agent)
dockerignores+=(docker_setup/dockerignores/.dockerignore.common)
dockerignores+=(docker_setup/dockerignores/.dockerignore.common)
dockerignores+=(docker_setup/dockerignores/.dockerignore.common)
dockerignores+=(docker_setup/dockerignores/.dockerignore.common)
dockerignores+=(docker_setup/dockerignores/.dockerignore.da)

# List of Configurable IEI services and respective dockerignores.
while read line ; do
	services+=($line)
done < <(python3.6 get_services.py name)
while read line ; do
	dockerignores+=($line)
done < <(python3.6 get_services.py dockerignore)