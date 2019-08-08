#!/bin/sh
./etcdctl user add $1:$2
./etcdctl role add $1
./etcdctl user grant-role $1 $1
./etcdctl role grant-permission $1 read /$1
./etcdctl role grant-permission $1 read /Publickeys
./etcdctl role grant-permission $1 read /Global