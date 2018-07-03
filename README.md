# ElephantTrunkArch project

# Pre-requisites:
1. Setting up GO dev env (Install latest Go 1.10.3 version or 1.9 version)
    * Follow guide [go installation](https://golang.org/doc/install#install) to install latest golang and setting up go workspace directory (have 2 folders: src and bin here)
    * Export GOPATH to point to your go workspace dir and GOBIN to $GOPATH/bin
    * Clone this repo under $GOPATH/src
    * Install gRPC, protocal buffer compiler by following steps mentioned @ [go grpc quick start](https://grpc.io/docs/quickstart/go.html). As gRPC code file already exists, no need to re-generate the gRPC code file (da.pb.go) here, it's only needed if da.proto is been changed.
    > Note:
    > 1. **go build** puts the binary artifact in the current folder whereas **go install** puts it in the $GOPATH/bin folder. Command **go clean** deletes the artifact specific to the project
    > 2. While running Go programs, one may face issues with missing dependency packages. Please use **go get -u <pacakage>** cmd to install each.

2. Setting up python dev env
    * Install python3. Follow guide [python3 installation](http://docs.python-guide.org/en/latest/starting/install3/linux/)
    * One can run the python programs from the $GOPATH/src/iapoc_elephanttrunkarch
    * Change directory to above repo and export PYTHONPATH=[abspathto_iapoc_folder]:[abspathto_iapoc_folder]/DataAgent/da_grpc/protobuff
    * Install gRPC, gRPC tools by running below commands. More details @ [python grpc quick start](https://grpc.io/docs/quickstart/python.html)
        * **sudo -H pip3 install grpcio**
        * **sudo -H pip3 install grpcio-tools**
      As the files already exist, no need to re-generate the the 2 py files da_pb2.py and da_pb2_grpc.py here, it's only needed if da.proto is been changed.
    > Note:
    > 1. While running python programs, one may face issues with missing dependency modules. Please use **sudo -H pip3 install  <module>** cmd to install each.
    > 2. Install below DataAnalytics dependencies:
    	* python3.6 package by following [this](http://ubuntuhandbook.org/index.php/2017/07/install-python-3-6-1-in-ubuntu-16-04-lts/)
    	* **sudo -H pip3.6 install -r classifier_requirements.txt** - installs all dependencies for classifer program
    	* **sudo -H pip2 install protobuf** - Needed by the kapasender.py UDF file.
    	* **sudo apt-get install mosquitto** - MQTT implementation
    	* **sudo apt-get install postgresql** - database used by ETR

3. Have the TICK stack softwares (influx, kapacitor - [refer](https://www.digitalocean.com/community/tutorials/how-to-monitor-system-metrics-with-the-tick-stack-on-ubuntu-16-04)) and [Redis](https://askubuntu.com/questions/868848/how-to-install-redis-on-ubuntu-16-04) installed locally. 

4. Copy the yumei_trigger.avi from "\\Vmspfsfsbg01\qsd_sw_ba\FOG\test_video" shared folder and put it under the root directory - this is needed by VideoIngestion and DataAnalytics module

# Follow the below steps to start the Video ingestion and see frames processed in the UDF. This can be done in 3 ways:

## Standalone installation of ETA modules (See individual modules README.md on how to run them)

1. Start all dependency modules:
    - influxdb: **sudo service influxd [start|stop|status|restart]**
    - redis: **sudo service redis [start|stop|status|restart]**
    - mosquitto:  **sudo service mosquitto [start|stop|status|restart]**
    - postgresql: **sudo service postgresql [start|stop|status|restart]**
    - NATS server: Refer **StreamManager/README.md** file
2. Start the DataAgent by providing the right config file having the right configs. Refer **DataAgent/README.md**.
3. Start DataAnalytics module by referring to **DataAnalytics/README.md**
4. Start the 2 nats client like below to see messages being received on the interested topic (We are using the same measurement name as topic name as of now). Here, the topic name is "classifier_results".
    * go run StreamManager/test/natsClient.go -topic="topic name"
    * python3 StreamManager/test/natsClient.py "topic name"
5. Start VideoIngestion/VideoIngestion.py: `python3 VideoIngestion/VideoIngestion.py --config factory.json` to ingest the data to InfluxDB/ImageStore. Refer **VideoIngestion/README.md** for more details.
   **Note**: factory.json is for the yumei_trigger.avi file. If one wants to try with Baslera camera, use factory_cam.json instead of factory.json. If using containers, copy factory_cam.json as factory.json. In otherwords, we only mount 1 file factory.json and expects everything to be configured in that file.
6. See the terminal of classifier.py to see the analysis results.
   The results are also published to mqtt topic.
   The result images can be seen in the ~/saved_images (as per the factory.json configuration)
7. The data being ingested can be seen coming into the DataAgent via StreamManager's UDP subscription to Influxdb which is published to message bus like NATS server and the same should be seen being received in the started nats client above.

## Running as docker containers

Refer docker_setup/README.md file