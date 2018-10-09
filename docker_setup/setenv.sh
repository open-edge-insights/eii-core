#!/bin/bash

# Sets the env variables needed for ETA build environment

# Exit the script if any one command fails
set -e

source .env

etaRootDir=$ETA_INSTALL_PATH
etaLogDir=$etaRootDir/logs
etaConfDir=$etaRootDir/config
etaDataDir=$etaRootDir/data

rootDir=".."
configDir="config"
