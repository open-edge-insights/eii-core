#!/bin/bash

# Sets the env variables needed for ETA build environment

# Exit the script if any one command fails
set -e

source .env

etaRootDir=$ETA_INSTALL_PATH
etaLogDir=$etaRootDir/logs
etaConfDir=$etaRootDir/config
etaDataDir=$etaRootDir/data

# We have to create here as this a deploy mode
# required folder. And setenv.sh script is executed
# during deploy mode instead of init.sh
mkdir -p $etaLogDir/consolidatedLogs

rootDir=".."
configDir="config"
