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

3. Have the TICK stack softwares (influx, kapacitor) and [Redis](https://redis.io/topics/quickstart) installed locally. 

4. Install below DataAnalytics dependencies:
    - python3.6 package by following [this](http://ubuntuhandbook.org/index.php/2017/07/install-python-3-6-1-in-ubuntu-16-04-lts/)
    - **python3.6 -m pip install -r classifier_requirements.txt** - installs all dependencies for classifer program
    - **python2 -m pip install protobuf** - Needed by the kapasender.py UDF file.
    - **sudo apt-get install mosquitto** - MQTT implementation
    - **sudo apt-get install postgresql** - database

5. Copy the yumei_trigger.avi from "\\Vmspfsfsbg01\qsd_sw_ba\FOG\test_video" shared folder and put it under the root directory - this is needed by VideoIngestion and DataAnalytics module

6. For running the DataAnalytics, the kapacitor UDF setup is required. For more details, refer below links:
   - Writing a sample UDF at [anomaly detection](https://docs.influxdata.com/kapacitor/v1.5/guides/anomaly_detection/)
   - UDF and kapacitor interaction [here](https://docs.influxdata.com/kapacitor/v1.5/guides/socket_udf/)

   The udf is **DataAnalytics/kapasender.py** and the tick script is **DataAnalytics/classifier.tick**
   
   Follow below steps to start DataAnalytics module:
    1. Kapacitor configuration:
        Use the below config in **kapacitor/kapacitor.conf** file for enabling the UDF. Change 
        ```
        -------------------------------------------------------------------
        [udf]
        [udf.functions]
            [udf.functions.kapasender]
                # Run python
                prog = "/usr/bin/python2"
                args = ["-u", "~/go/src/iapoc_elephanttrunkarch/DataAnalytics/kapasender.py"]
                timeout = "10s"
                [udf.functions.kapasender.env]
                    PYTHONPATH = "<pathtokapacitorrepo>/kapacitor/udf/agent/py"
        -------------------------------------------------------------------
        ```
        > Note:
        > Please provide abosolute paths to **kapasender.py** and to PYTHONPATH in the kapacitor.conf file.
        > Pre-requisite for this step: **go get github.com/influxdata/kapacitor/cmd/kapacitor** and **go get github.com/influxdata/kapacitor/cmd/kapacitord** to get the go kapacitor lib source under **$GOPATH/src/github.com/influxdata/kapacitor**. The bins **kapacitor** and **kapacitord** would be copied to $GOPATH/bin. 
    2. Run this command in a terminal
        ```
        **python3.6 DataAnalytics/classifier.py factory.json**
        ```
    3. Run this command in another terminal
        ```
        **kapacitord -config kapacitor.conf**
        ```
    4. Run this command in another terminal
        ```
        **kapacitor/kapacitor define classifier_task -tick classifier.tick**
        **kapacitor/kapacitor enable classifier_task**
        ```

# Follow the below steps to start the Video ingestion and see frames processed in the UDF. This can be done in 3 ways:

## Standalone installation of ETA modules (See individual modules README.md on how to run them)

1. Start all dependency modules:
    - influxdb: **sudo service influxdb [start|stop|status|restart]**
    - redis: use redis-server executable or sudo service command
    - mosquitto:  **sudo service mosquitto [start|stop|status|restart]**
    - postgresql: **sudo service postgresql [start|stop|status|restart]**
    - NATS server: Refer StreamManager module's README.md file
2. Start the DataAgent by providing the right config file having the right configs
3. Start VideoIngestion/VideoIngestion.py to ingest the data to InfluxDB/ImageStore
   **python3 VideoIngestion/VideoIngestion.py --config factory.json**
   > Note: factory.json is for the yumei_trigger.avi file. If one wants to try with Baslera camera, use factory_cam.json instead of factory.json.
4. See the terminal of classifier.py to see the analysis results.
   The results are also published to mqtt topic.
   The result images can be seen in the ~/saved_images (as per the factory.json configuration)
5. The data being ingested can be seen coming into the DataAgent
6. Start the 2 nats client like below to see messages being received on the interested topic (We are using the same measurement name as topic name as of now)
    * go run StreamManager/test/natsClient.go -topic="topic name"
    * python3 StreamManager/test/natsClient.py "topic name"

## Running as docker containers

Refer **README-DOCKER.md** file