#!/bin/bash

# to debug uncomment below line
#set -x


# Start DataAgent
if [ "$3" = "true" ]
then
    DataAgent -log_dir=${GO_WORK_DIR}/log/DataAgent -config=DataAgent/provision_config.json -stderrthreshold=$1 -v=$2
else
    # Setting the VAULT_ADDR to override the config provided API_ADDR
    VAULT_ADDR=http://localhost

    # Start the server skip the TLS verifcation unlike what config file imposes.
    vault server -config ./vault/config/vault_config.hcl -tls-skip-verify & 2>&1

    DataAgent -log_dir=${GO_WORK_DIR}/log/DataAgent -stderrthreshold=$1 -v=$2
fi
