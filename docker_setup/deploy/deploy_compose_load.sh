#!/bin/bash

# This loads all docker images of eta along with its dependencies
./deploy/load_built_images.sh

source .env

echo "0.1 Copying resolv.conf to /etc/resolv.conf (On non-proxy environment, please comment below cp instruction before you start)"
cp -f resolv.conf /etc/resolv.conf

echo "0.2 Adding Docker Host IP Address to config/DataAgent.conf, config/factory.json and config/factory_prod.json files..."
source ./update_config.sh

