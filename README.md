# ElephantTrunkArch project

# Pre-requisites:
1. Setting up GO dev env
    * Follow guide [go installation](http://docs.python-guide.org/en/latest/starting/install3/linux/) to install latest golang and setting up go workspace directory (have 2 folders: src and bin here)
    * Export GOPATH to point to your go workspace dir and GOBIN to $GOPATH/bin
    * Clone this repo under $GOPATH/src
    * (This step is needed only if you are tweaking .proto file and regenerating the <file_name>.pb.go file) Install gRPC, protocal buffer compiler and generate gRPC code by following steps mentioned @ [go grpc quick start](https://grpc.io/docs/quickstart/go.html)
    > Note:
    > 1. **go build** puts the binary artifact in the current folder whereas **go install** puts it in the $GOPATH/bin folder. Command **go clean** deletes the artifact specific to the project
    > 2. While running Go programs, one may face issues with missing dependency packages. Please use **go get -u <pacakage_name>** cmd to install each.

2. Setting up python dev env
    * Install python3. Follow guide [python3 installation](http://docs.python-guide.org/en/latest/starting/install3/linux/)
    * One can run the python programs from the $GOPATH/src/iapoc_elephanttrunkarch
    * Change directory to above repo and export PYTHONPATH=.:./DataAgent/da_grpc/protobuff
    * (This step is needed only if you are tweaking .proto file and regenerating the 2 py files <file_name>_pb2.py and <file_name>_pb2_grpc.py) Install gRPC, gRPC tools by running below commands. More details @ [python grpc quick start](https://grpc.io/docs/quickstart/python.html)
        * python3 -m pip install grpcio
        * python3 -m pip install grpcio-tools
    > Note:
    > 1. While running python programs, one may face issues with missing dependency modules. Please use **python3 -m pip install <module>** cmd to install each.

3. Have the TICK stack softwares and Redis installed locally

# Workflow as of now (See individual modules README.md on how to run them)

1. Start InfluxDB and Redis
2. Start the DataAgent by providing the right config file having the right configs
3. Start DataIngestionLib_Test.py to ingest the data to InfluxDB/ImageStore
4. As DataIngestionLib_Test.py is ingesting the data, the same data can be seen coming into the DataAgent
