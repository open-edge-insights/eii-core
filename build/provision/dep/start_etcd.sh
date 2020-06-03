#!/bin/sh

# Copyright (c) 2020 Intel Corporation.

# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

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
	if [ -z "$ETCD_CERT_FILE" ]; then
	export ETCD_CERT_FILE="/run/secrets/etcd_server_cert"
	fi
	if [ -z "$ETCD_KEY_FILE" ]; then
	export ETCD_KEY_FILE="/run/secrets/etcd_server_key"
	fi
	if [ -z "$ETCD_TRUSTED_CA_FILE" ]; then
	export ETCD_TRUSTED_CA_FILE="/run/secrets/ca_etcd"
    fi
	export ETCD_CLIENT_CERT_AUTH="true"
	
fi
./etcd
