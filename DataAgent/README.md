
# DataAgent

DataAgent module is responsible for initializing the stream manager to listen to the data going into InfluxDB and starting off the gRPC server for rpc calls to exposed interfaces (GetConfigInt - Internal, Config and Query - External)

## 3 ways to run from $GOPATH/src/ElephantTrunkArch - present working directory
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

Here, `--input_file` argument value would be read and it's data gets stored in ImageStore (Store API) which returns the `imgHandle`. Using `GetBlob(imgHandle)` gRPC interface, the byte array corresponding to that `imgHandle` is received and `--output_file` is created and both files `md5sum` value is compared to verify if they are the same or not.

* Start C++ gRPC client: `./clientTest [imgHandle] [output_image_file_path]`
  Since you need to compile the test files before running them, follow the below given steps:
  * Change to ElephantTrunkArch directory and run these following commands one by one:
    * cd DataAgent/da_grpc/protobuff/
    * g++ -std=c++11 `pkg-config --cflags protobuf grpc`  -c -o cpp/da.pb.o cpp/da.pb.cc
    * g++ -std=c++11 `pkg-config --cflags protobuf grpc`  -c -o cpp/da.grpc.pb.o cpp/da.grpc.pb.cc
    * cd ../test/
    * g++ -std=c++11 `pkg-config --cflags protobuf grpc`  -c -o clientTest.o clientTest.cc
    * g++ ../protobuff/cpp/da.pb.o ../protobuff/cpp/da.grpc.pb.o clientTest.o -L/usr/local/lib `pkg-config --libs protobuf grpc++ grpc` -Wl,--no-as-needed -lgrpc++_reflection -Wl,--as-needed -ldl -o clientTest
  * If all of these steps are run successfully, a clientTest file should be generated within your    test folder.
    * Pre-requisite: Run python gRPC client and get imgHandle of the Image frame which was given
      as input.
    * Run the clientTest using the following command:
      * ./clientTest [imgHandle] [output_image_file_path]
    * Verify the result by making sure md5sum of both the generated output image file and input
      file are the same.

      **Note**: To use this client file outside the project workspace, just make sure you copy the `DataAgent/da_grpc/client/cpp/client.cc` file along with all the files present in `DataAgent/da_grpc/protobuff/cpp/` folder and take care of imports accordingly.