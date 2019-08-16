#!/bin/sh
if $DEV_MODE = "true"; then
	export ETCD_LISTEN_CLIENT_URLS=$ETCD_LISTEN_CLIENT_URLS"http://127.0.0.1:$ETCD_CLIENT_PORT"
	export ETCD_ADVERTISE_CLIENT_URLS=$ETCD_ADVERTISE_CLIENT_URLS"http://127.0.0.1:$ETCD_CLIENT_PORT"
else
	
	export ETCD_LISTEN_CLIENT_URLS=$ETCD_LISTEN_CLIENT_URLS"https://127.0.0.1:$ETCD_CLIENT_PORT"
	export ETCD_ADVERTISE_CLIENT_URLS=$ETCD_ADVERTISE_CLIENT_URLS"https://127.0.0.1:$ETCD_CLIENT_PORT"
	export ETCD_CERT_FILE="/run/secrets/etcd_server_cert"
	export ETCD_KEY_FILE="/run/secrets/etcd_server_key"
	export ETCD_TRUSTED_CA_FILE="/run/secrets/ca_etcd"
	export ETCD_CLIENT_CERT_AUTH="true"
	
fi
./etcd
