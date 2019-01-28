#!/bin/bash

# Sets the env variables needed for IEI build environment

# Exit the script if any one command fails
set -e

source .env

ieiRootDir=$IEI_INSTALL_PATH
ieiLogDir=$ieiRootDir/logs
ieiConfDir=$ieiRootDir/config
ieiDataDir=$ieiRootDir/data

# We have to create here as this a deploy mode
# required folder. And setenv.sh script is executed
# during deploy mode instead of init.sh
mkdir -p $ieiLogDir/consolidatedLogs

rootDir=".."
configDir="config"
