#!/bin/sh
cat Certificates/influxdb/influxdb_server_certificate.pem | ./etcdctl put /InfluxDBConnector/server_cert
cat Certificates/influxdb/influxdb_server_key.pem | ./etcdctl put /InfluxDBConnector/server_key
cat Certificates/ca/ca_certificate.pem | ./etcdctl put /InfluxDBConnector/ca_cert

cat Certificates/kapacitor_cert/kapacitor_cert_server_certificate.pem | ./etcdctl put /Kapacitor/server_cert
cat Certificates/kapacitor_cert/kapacitor_cert_server_key.pem | ./etcdctl put /Kapacitor/server_key
cat Certificates/ca/ca_certificate.pem | ./etcdctl put /Kapacitor/ca_cert

cat Certificates/opcua/opcua_server_certificate.der | ./etcdctl put /OpcuaExport/server_cert
cat Certificates/opcua/opcua_server_key.der | ./etcdctl put /OpcuaExport/server_key
cat Certificates/ca/ca_certificate.der | ./etcdctl put /OpcuaExport/ca_cert