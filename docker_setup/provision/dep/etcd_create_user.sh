#!/bin/sh
./etcdctl user add $1 --no-password
./etcdctl role add $1
./etcdctl user grant-role $1 $1
./etcdctl role grant-permission $1 read /$1 --prefix
./etcdctl role grant-permission $1 read /Publickeys --prefix
./etcdctl role grant-permission $1 read /GlobalEnv --prefix

