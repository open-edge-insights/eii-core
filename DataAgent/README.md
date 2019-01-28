
# DataAgent

DataAgent module is responsible for initializing the stream manager to listen to the data going into InfluxDB and starting off the gRPC server for rpc calls to exposed interfaces (GetConfigInt - Internal, Config and Query - External)

## 3 ways to run from $GOPATH/src/IEdgeInsights - present working directory
* go run DataAgent/DataAgent.go -config=[configfilepath] -log_dir=[glogdirpath]
* cd Datagent && go build DataAgent/DataAgent.go && ./DataAgent -config=[configfilepath] -log_dir=[glogdirpath]
* go install DataAgent/DataAgent.go && DataAgent -config=[configfilepath] -log_dir=[glogdirpath]
> Note:
> 1. Use **DataAgent -h** to see all the flags, it shows glog and DataAgent flags. The same holds true for all test go programs defined in the project
> 2. Here -log_dir is optional, if not provided, logs will not be logged to any directory

## gRPC server module testing alone (Tests gRPC interfaces: Internal: GetConfigInt("RedisCfg"|"InfluxDBCfg"), External: GetBlob("imgHandle))

### Pre-requisites:
If the IEI is running on a node behind a coporate network/proxy server, please set IP address    of the node in the no_proxy/NO_PROXY env variable  on the system where you are executing   
the grpc clients so that the communication doesn't go via the proxy server.
Eg. `export no_proxy=$no_proxy,<IEI node IP address>`
If this is not set, one would into gRPC errors like `StatusCode.UNAVIALABLE`      

* Start go gRPC external client: `go run DataAgent/da_grpc/test/go/clientTest.go --input_file=[input_image_file_path] --output_file=[output_image_file_path]`.

    **Note**:
    * To use this client file outside the project workspace, just make sure to copy the `DataAgent/da_grpc/client/go/client.go` file along with `DataAgent/da_grpc/protobuff/go/da.pb.go` and take care of imports accordingly

* Start go gRPC internal client: `go run DataAgent/da_grpc/test/go/test_internal/clientTest.go`.

    **Note**:
    * To use this client file outside the project workspace, just make sure to copy the `DataAgent/da_grpc/client/go/client_internal/client.go` file along with `DataAgent/da_grpc/protobuff/go/pb_internal/dainternal.pb.go` and take care of imports accordingly


* Start python gRPC external client: `python3.6 DataAgent/da_grpc/test/py/client_test.py --input_file [input_image_file_path] --output_file [output_image_file_path]`

    **Note**: To use this client file outside the project workspace, just make sure one copy the `DataAgent/da_grpc/client/py/client.py` file along with `DataAgent/da_grpc/protobuff/py/da_pb2.py` and `DataAgent/da_grpc/protobuff/py/da_pb2_grpc.py` and take care of imports accordingly

Here, `--input_file` argument value would be read and it's data gets stored in ImageStore (Store API) which returns the `imgHandle`. Using `GetBlob(imgHandle)` gRPC interface, the byte array corresponding to that `imgHandle` is received and `--output_file` is created and both files `md5sum` value is compared to verify if they are the same or not.

* Start python gRPC internal client: `python3.6 DataAgent/da_grpc/test/py/test_internal/client_test.py`

    **Note**: To use this client file outside the project workspace, just make sure one copy the `DataAgent/da_grpc/client/py/client_internal/client.py` file along with `DataAgent/da_grpc/protobuff/py/pb_internal/dainternal_pb2.py` and `DataAgent/da_grpc/protobuff/py/pb_internal/dainternal_pb2_grpc.py` and take care of imports accordingly


* Start C++ gRPC client: `./clientTest [imgHandle] [output_image_file_path]`
  Since you need to compile the test files before running them, follow the below given steps:
  * Run the [da_grpc/test/cpp/Makefile](da_grpc/test/cpp/Makefile) present in test/cpp folder using the command:
    * make
  * If make file is run successfully, a clientTest file should be generated within your test/cpp folder.
    * Pre-requisite: Run python gRPC client and get imgHandle of the Image frame which was given
      as input.
    * Run the clientTest using the following command:
      * ./clientTest [imgHandle] [output_image_file_path]
    * Verify the result by making sure md5sum of both the generated output image file and input
      file are the same.

      **Note**: To use this client file outside the project workspace, just make sure you copy the `DataAgent/da_grpc/client/cpp/client.cc` file along with all the files present in `DataAgent/da_grpc/protobuff/cpp/` folder and take care of imports accordingly.



## Secure GRPC

Secure GRPC is the secure channel enabled GRPC Server. Where it works on top of
MTLS Authenticaion method.

Under Certificates folder.

For Server it Requires,

  1. Certificates/ca/ca_certificate.pem
  2. Certificates/server/server_certificate.pem
  3. Certificates/server/server_key.pem

For Any Client it Requires,
  1. Certificates/ca/ca_certificate.pem
  2. Certificates/client/client_certificate.pem
  3. Certificates/client/client_key.pem


## Generating Certificates

**Note** Following steps are followed by us for Development & Validation
Any OpenSSL Standard Certificate Generation Will work. Please follow the certificate
names & extension as per Certificates directory / as above

To Generate the Certificate please follow the wiki.
  https://github.intel.com/IEdgeInsights/IEdgeInsights/wiki/Generating-TLS-certificates-and-keys
