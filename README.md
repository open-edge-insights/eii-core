# ElephantTrunkArch project

# Pre-requisites:
1. Setting up GO dev env (Install latest Go 1.10.3 version)
    * Follow guide [go installation](https://www.digitalocean.com/community/tutorials/how-to-install-go-1-6-on-ubuntu-16-04) to install latest golang and setting up go workspace directory (have 2 folders: src and bin here)
    * Export GOPATH to point to your go workspace dir and GOBIN to $GOPATH/bin
    * Clone this repo under $GOPATH/src
    * Install gRPC, protocal buffer compiler by following steps mentioned @ [go grpc quick start](https://grpc.io/docs/quickstart/go.html). As file already exists, no need to re-generate the gRPC code file (da.pb.go) here, it's only needed if da.proto is been changed.
    > Note:
    > 1. **go build** puts the binary artifact in the current folder whereas **go install** puts it in the $GOPATH/bin folder. Command **go clean** deletes the artifact specific to the project
    > 2. While running Go programs, one may face issues with missing dependency packages. Please use **go get -u <pacakage>** cmd to install each.

2. Setting up python dev env
    * Install python3. Follow guide [python3 installation](http://docs.python-guide.org/en/latest/starting/install3/linux/)
    * One can run the python programs from the $GOPATH/src/iapoc_elephanttrunkarch
    * Change directory to above repo and export PYTHONPATH=[abspathto_iapoc_folder]:[abspathto_iapoc_folder]/DataAgent/da_grpc/protobuff
    * Install gRPC, gRPC tools by running below commands. More details @ [python grpc quick start](https://grpc.io/docs/quickstart/python.html)
        * python3 -m pip install grpcio
        * python3 -m pip install grpcio-tools
      As the files already exist, no need to re-generate the the 2 py files da_pb2.py and da_pb2_grpc.py here, it's only needed if da.proto is been changed.
    > Note:
    > 1. While running python programs, one may face issues with missing dependency modules. Please use **python3 -m pip install <module>** cmd to install each.

3. Have the TICK stack softwares and Redis installed locally

4. For running the DataAnalytics, the kapacitor UDF setup is required.
   Refer to the link : https://docs.influxdata.com/kapacitor/v1.5/guides/anomaly_detection/

   The udf is DataAnalytics/kapasender.py and the tick script is DataAnalytics/classifier.tick
   Use the below config in kapacitor.conf file for enabling the UDF.
-------------------------------------------------------------------
[udf]
[udf.functions]
    [udf.functions.kapasender]
        # Run python
        prog = "/usr/bin/python2"
        args = ["-u", "~/go/src/iapoc_elephanttrunkarch/DataAnalytics/kapasender.py"]
        timeout = "10s"
        [udf.functions.kapasender.env]
            PYTHONPATH = "<path to kapacitor>/kapacitor/udf/agent/py"
-------------------------------------------------------------------

   Run below steps to start analytics after doing kapacitor configuration

   # python3.6 DataAnalytics/classifier.py factory.json

   Start kapacitor in another terminal -
   # kapacitor -config kapacitor.conf

   Follow the below steps to start the Video ingestion and see frames processed in the UDF.

# Workflow as of now (See individual modules README.md on how to run them)

1. Start InfluxDB and Redis
2. Start the DataAgent by providing the right config file having the right configs
3. Start VideoIngestion/VideoIngestion.py to ingest the data to InfluxDB/ImageStore
   # python3 VideoIngestion/VideoIngestion.py --config factory.json
4. See the terminal of classifier.py to see the analysis results.
   The results are also published in mqtt topic.
   The result images can be seen in the ~/saved_images (as per the factory.json configuration)
5. As DataIngestionLib_Test.py is ingesting the data, the same data can be seen coming into the DataAgent
6. Start the 2 nats client like below to see messages being received on the interested topic (We are using the same measurement name as topic name as of now)
    * go run StreamManager/test/natsClient.go -topic="topic name"
    * python3 StreamManager/test/natsClient.py "topic name"
