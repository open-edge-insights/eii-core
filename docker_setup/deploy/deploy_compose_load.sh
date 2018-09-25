#!/bin/bash

# This loads all docker images of eta along with its dependencies
./deploy/load_built_images.sh

# This brings down the previous containers

echo "0.1 Copying resolv.conf to /etc/resolv.conf (On non-proxy environment, please comment below cp instruction before you start)"
cp -f resolv.conf /etc/resolv.conf

echo "0.2 Adding Docker Host IP Address to config/DataAgent.conf, config/factory.json and config/factory_prod.json files..."
source ./update_config.sh

echo "0.3 Setting up /var/lib/eta directory and copying all the necessary config files..."
if [[ -d /var/lib/eta && -n "$(ls -A /var/lib/eta)" ]]; then
    echo "Found a non-empty /var/lib/eta folder, Avoid overriding the config..."
    source ./setenv.sh
    cp -f $configDir/* $etaConfDir
else
    source ./init.sh
fi
