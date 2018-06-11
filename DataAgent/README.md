
# DataAgent

DataAgent module is responsible for initializing the stream manager to listen to the data going into InfluxDB and starting off the gRPC server for rpc calls to exposed interfaces (GetConfigInt - Internal, Config and Query - External)

## 3 ways to run from $GOPATH/src/iapoc_elephanttrunkarch - present working directory
* go run DataAgent/DataAgent.go -config=<config_filepath> -log_dir=<glog_dir_path>
* cd Datagent && go build DataAgent/DataAgent.go && ./DataAgent -config=<config_filepath> -log_dir=<glog_dir_path>
* go install DataAgent/DataAgent.go && DataAgent -config=<config_filepath> -log_dir=<glog_dir_path>
> Note:
> 1. Use **DataAgent -h** to see all the flags, it shows glog and DataAgent flags. The same holds true for all test go programs defined in the project
> 2. Here -log_dir is optional, if not provided, logs will not be logged to any directory

## gRPC server module testing alone

* Start Go gRPC client test: go run DataAgent/da_grpc/test/clientTest.go
* Start python gRPC client test: python3 run DataAgent/da_grpc/test/client_test.go