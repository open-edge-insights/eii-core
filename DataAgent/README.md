
# DataAgent

DataAgent module is responsible for initializing the stream manager to listen to the data going into InfluxDB and starting off the gRPC server for rpc calls to exposed interfaces (GetConfigInt - Internal, Config and Query - External)

## 3 ways to run from $GOPATH/src/iapoc_elephanttrunkarch - present working directory
* go run DataAgent/DataAgent.go -config=[configfilepath] -log_dir=[glogdirpath]
* cd Datagent && go build DataAgent/DataAgent.go && ./DataAgent -config=[configfilepath] -log_dir=[glogdirpath]
* go install DataAgent/DataAgent.go && DataAgent -config=[configfilepath] -log_dir=[glogdirpath]
> Note:
> 1. Use **DataAgent -h** to see all the flags, it shows glog and DataAgent flags. The same holds true for all test go programs defined in the project
> 2. Here -log_dir is optional, if not provided, logs will not be logged to any directory

## gRPC server module testing alone (Tests gRPC interfaces: Internal: GetConfigInt("RedisCfg"|"InfluxDBCfg"), External: GetBlob("imgHandle))

* Start go gRPC client: `go run DataAgent/da_grpc/test/clientTest.go --input_file=[input_image_file_path] --output_file=[output_image_file_path]`. 

    **Note**: To use this client file outside the project workspace, just make sure to copy the `DataAgent/da_grpc/client/client.go` file along with `DataAgent/da_grpc/protobuff/da.pb.go` and take care of imports accordingly

* Start python gRPC client: `python3.6 DataAgent/da_grpc/test/client_test.py --input_file [input_image_file_path] --output_file [output_image_file_path]`

    **Note**: To use this client file outside the project workspace, just make sure one copy the `DataAgent/da_grpc/client/client.py` file along with `DataAgent/da_grpc/protobuff/da_pb2.py` and `DataAgent/da_grpc/protobuff/da_pb2_grpc.py` and take care of imports accordingly

Here, `--input_file` argument value would be read and it's data gets stored in ImageStore (Store API) which returns the `imgHandle`. Using `GetBlob(imgHandle)` gRPC interface, the byte array corresponding to that `imgHandle` is received and `--output_file` is created and both files `md5sume` value is compared to verify if they are the same or not.