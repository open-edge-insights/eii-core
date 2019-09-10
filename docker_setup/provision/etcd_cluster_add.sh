#!/bin/bash
# This script will add new member to ETCD cluster
# Usage: sudo ./etcd_cluster_add.sh <etcd_node_name> <ip_address>
set -a
source ../.env
set +a

if [ $# -lt 2 ]
  then
    echo 'Required arguments not supplied. Please supply unique node name and IP address for the new etcd node to be added to the cluster.'
    echo 'Usage: sudo ./etcd_cluster_add.sh <etcd_node_name> <ip_address>'
    exit 1
fi

if ! [[ $2 =~ ^[0-9]+\.[0-9]+\.[0-9]+\.[0-9]+$ ]]; then
    echo 'Invalid IP address'
    echo 'Usage: sudo ./etcd_cluster_add.sh <etcd_node_name> <ip_address>'
    exit 1
fi

if [ $DEV_MODE = 'true' ]; then
    docker exec -it ia_etcd ./etcdctl  member add $1 --peer-urls http://$2:$ETCD_PEER_PORT > $1.env
else
    docker exec -it ia_etcd ./etcdctl --cacert /run/secrets/ca_etcd --cert /run/secrets/etcd_root_cert --key /run/secrets/etcd_root_key member add $1 --peer-urls https://$2:$ETCD_PEER_PORT  > $1.env
fi

if grep -q 'Error: etcdserver: Peer URLs already exists' $1.env; then
    echo 'Error: IP address supplied as new node already exists in the cluster.'
    rm $1.env
    exit 1
fi

cat $1.env
tr -d '\r' < $1.env > dep/$1tmp.env
sed '1d' dep/$1tmp.env > dep/$1.env
rm $1.env dep/$1tmp.env


