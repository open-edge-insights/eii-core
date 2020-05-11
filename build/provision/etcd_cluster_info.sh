#!/bin/bash -e
# Usage: This script will fetch all the memeber info for a cluster
set -a
source ../.env
set +a

if [ -z "$ETCD_HOST" ] && [ -z "$ETCD_CLIENT_PORT" ] ; then
  ETCDCTL_ENDPOINTS= $ETCD_HOST:$ETCD_CLIENT_PORT
fi

if [ $DEV_MODE = 'true' ]; then
    docker exec -it ia_etcd ./etcdctl member list -w table
else
    docker exec -it ia_etcd ./etcdctl --cacert /run/secrets/ca_etcd --cert /run/secrets/etcd_root_cert --key /run/secrets/etcd_root_key member list -w table
fi
