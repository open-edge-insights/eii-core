#!/bin/bash

# to debug uncomment below line
#set -x

# Setting the VAULT_ADDR to override the config provided API_ADDR
VAULT_ADDR=http://localhost:8200

# Start the server skip the TLS verifcation unlike what config file imposes.
vault server -config ./vault/config/vault_config.hcl -tls-skip-verify & 2>&1

# Start DataAgent
provision_eta -config=./config_file.json -log_dir=${GO_WORK_DIR}/log/provision
