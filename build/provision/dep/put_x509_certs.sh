#!/bin/sh
# $1 == AppName, $2 == CertType, $3 == ETCD_PREFIX
cat Certificates/$1_Server/$1_Server_server_certificate.$2 | ./etcdctl put $3/$1/server_cert
cat Certificates/$1_Server/$1_Server_server_key.$2 | ./etcdctl put $3/$1/server_key
cat Certificates/ca/ca_certificate.$2 | ./etcdctl put $3/$1/ca_cert
rm -rf Certificates/$1_Server
