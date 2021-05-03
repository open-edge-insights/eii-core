#!/bin/bash

# Copyright (c) 2020 Intel Corporation.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

#Script to write back data from ETCD Cluster to JSON file.
function helpFunction {
    echo >&2
    echo "Usage: ./etcd_capture.sh --ca_etcd ca_certificate --etcd_root_cert root_cert --etcd_root_key root_key" >&2
    echo >&2
    echo "SUMMARY": >&2
    echo >&2
    echo "  --ca_etcd  etcd ca certificate" >&2
    echo >&2
    echo "  --etcd_root_cert  root client certificate" >&2
    echo >&2
    echo "  --etcd_root_key  root client key" >&2
    echo >&2
    exit 1
}

function etcdCapture {
    set -a
    source ../.env
    set +a
    if [ -z $ETCD_HOST ]; then
        unset ETCD_HOST
    fi
    if [ -z $ETCD_CLIENT_PORT ]; then
        unset ETCD_CLIENT_PORT
    fi
    if [ ! -f ./etcd/etcdctl ]; then
        # Fetching etcd package to use etcdctl
        echo "Downloading etcd package"
        mkdir -p etcd
        curl -L https://github.com/coreos/etcd/releases/download/${ETCD_VERSION}/etcd-${ETCD_VERSION}-linux-amd64.tar.gz -o ./etcd/etcd-${ETCD_VERSION}-linux-amd64.tar.gz && \
            tar -xvf ./etcd/etcd-${ETCD_VERSION}-linux-amd64.tar.gz -C ./etcd --strip 1 && \
            rm -f ./etcd/etcd-${ETCD_VERSION}-linux-amd64.tar.gz
    fi

    python3 etcd_capture.py $@
}

if [ "$1" = "--help" ] || [ "$1" = "-h" ]; then
    helpFunction
fi
etcdCapture $@
