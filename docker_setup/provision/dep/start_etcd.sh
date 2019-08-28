#!/bin/sh

export ETCD_INITIAL_CLUSTER_TOKEN=eis-cluster	



if $DEV_MODE = "true"; then
	
	export ETCD_INITIAL_ADVERTISE_PEER_URLS=http://$HOST_IP:$ETCD_PEER_PORT
	export ETCD_LISTEN_PEER_URLS=http://$HOST_IP:$ETCD_PEER_PORT
	export ETCD_LISTEN_CLIENT_URLS=http://$HOST_IP:$ETCD_CLIENT_PORT,http://127.0.0.1:$ETCD_CLIENT_PORT
	export ETCD_ADVERTISE_CLIENT_URLS=http://$HOST_IP:$ETCD_CLIENT_PORT
else
	
	export ETCD_INITIAL_ADVERTISE_PEER_URLS=https://$HOST_IP:$ETCD_PEER_PORT
	export ETCD_LISTEN_PEER_URLS=https://$HOST_IP:$ETCD_PEER_PORT
	export ETCD_LISTEN_CLIENT_URLS=https://$HOST_IP:$ETCD_CLIENT_PORT,https://127.0.0.1:$ETCD_CLIENT_PORT
	export ETCD_ADVERTISE_CLIENT_URLS=https://$HOST_IP:$ETCD_CLIENT_PORT
	export ETCD_PEER_AUTO_TLS=true
	export ETCD_CERT_FILE="/run/secrets/etcd_server_cert"
	export ETCD_KEY_FILE="/run/secrets/etcd_server_key"
	export ETCD_TRUSTED_CA_FILE="/run/secrets/ca_etcd"
	export ETCD_CLIENT_CERT_AUTH="true"
	
fi
./etcd
