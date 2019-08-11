#!/bin/sh
./etcdctl user add root:$1
./etcdctl role add root
./etcdctl user grant-role root root
./etcdctl auth enable