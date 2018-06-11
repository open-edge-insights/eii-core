# StreamManager
StreamManager package subscribes to InfluxDB and posts topic data to Data bus

Pre-requisites:
1. In Influx terminal, create db by running "create database DA_INTERNAL_DB"

## 3 ways to run test file from $GOPATH/src/iapoc_elephanttrunkarch - present working directory
* go run StreamManager/test/strmMgrTest.go
* go build StreamManager/test/strmMgrTest.go && ./strmMgrTest
* go install StreamManager/test/strmMgrTest.go && strmMgrTest

