#!/bin/bash

# to debug uncomment below line
#set -x

# Starting influxdb service
#influxd -config /etc/influxdb/influxdb.conf &> influxd.log &

# Setting the VAULT_ADDR to override the config provided API_ADDR
VAULT_ADDR=http://localhost

# Start the server skip the TLS verifcation unlike what config file imposes.
vault server -config ./vault/config/vault_config.hcl -tls-skip-verify & 2>&1

# Start DataAgent
DataAgent -log_dir=${GO_WORK_DIR}/log/DataAgent
