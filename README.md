# ElephantTrunkArch (ETA) project

ETA project is a TICK (Telegraph, Influxdb, Chronograph, Kapacitor) stack based architecture aimed at providing the scable infrastructure capability for the factory automation.

## 2 ways to Install ETA:

### a. Docker compose setup (Preferred way of installation)

   Refer `docker_setup/README.md` file

### b. Bate metal setup (See individual modules README.md on how to run them)

#### 1. Known Issues

   * If one sees `C++ exception` thrown while starting the `ia_video_ingestion`, please disconnect the power/lan cable connected to the basler camera and restart ia_video_ingestion by runnign cmd `docker run ia_video_ingestion`
   * We are intermittently seeing the `ia_data_analytics` hanging at `MQTT Client Connected`. This issue is been actively looked into. We have found restarting the containers via `compose_startup.sh` or `deploy_compose_startup.sh` script found to fix the problem

#### 1. Pre-requisites:
1. Setting up GO dev env (Install latest Go 1.10.3 version or 1.9 version)
    * Follow guide [go installation](https://golang.org/doc/install#install) to install latest golang and setting up go workspace directory (have 2 folders: src and bin here)
    * Export GOPATH to point to your go workspace dir and GOBIN to $GOPATH/bin
    * Clone this repo under $GOPATH/src
    * Install gRPC, protocal buffer compiler by following steps mentioned @ [go grpc quick start](https://grpc.io/docs/quickstart/go.html). As gRPC code file already exists, no need to re-generate the gRPC code file (da.pb.go) here, it's only needed if da.proto is been changed.
    > Note:
    > 1. **go build** puts the binary artifact in the current folder whereas **go install** puts it in the $GOPATH/bin folder. Command **go clean** deletes the artifact specific to the project
    > 2. Run cmd `dep ensure -vendor-only -v` to pull all the dependencies for all Go modules from the project directory. One can get the `dep` tool by running this cmd: `go get -u github.com/golang/dep/cmd/dep`. For more details on how to use `dep`, visit [here](https://gist.github.com/subfuzion/12342599e26f5094e4e2d08e9d4ad50d)

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
    
3. Have the TICK stack softwares (influx, kapacitor - [refer](https://www.digitalocean.com/community/tutorials/how-to-monitor-system-metrics-with-the-tick-stack-on-ubuntu-16-04)) and [Redis](https://askubuntu.com/questions/868848/how-to-install-redis-on-ubuntu-16-04) installed locally. 

4. Copy the yumei_trigger.avi from "\\Vmspfsfsbg01\qsd_sw_ba\FOG\test_video" shared folder and put it under the root directory - this is needed by VideoIngestion and DataAnalytics module

#### 2. Steps to run ETA modules:
1. Start all dependency modules:
    - influxdb: **sudo service influxd [start|stop|status|restart]**
    - redis: **sudo service redis [start|stop|status|restart]**
    - mosquitto:  **sudo service mosquitto [start|stop|status|restart]**
    - postgresql: **sudo service postgresql [start|stop|status|restart]**

2. Start the DataAgent by providing the right config file having the right configs. Refer **DataAgent/README.md**.

3. Start DataAnalytics module by referring to **DataAnalytics/README.md**

4. Start OPCUA message bus client to listen on topic `classifier_results` by running below cmd in another terminal (For more details on dependencies to be installed, refer: (**DataBusAbstraction/README.md**):

`python2.7 DataBusAbstraction/py/test/DataBusTest.py --endpoint opcua://localhost:4840/elephanttrunk --direction SUB --ns streammanager --topic classifier_results`. 

5. Start VideoIngestion/VideoIngestion.py: `python3 VideoIngestion/VideoIngestion.py --config factory.json` to ingest the data to InfluxDB/ImageStore. Refer **VideoIngestion/README.md** for more details.

   **Note**: To test with video file, use `factory.json` and to test with Basler's camera, use `factory_cam.json`. Just provide the right serial number for the camera in `factory_cam.json` under `basler` json field

6. See the terminal of classifier.py to see the analysis results. The results are also published to mqtt topic. The result images can be seen in the ~/saved_images (as per the factory.json configuration)

7. The data being ingested can be seen coming into the DataAgent via StreamManager's UDP subscription to Influxdb which is published to OPCUA message bus and the same should be seen being received in the OPCUA client terminal started in `step 4`

8. Using OPCUA client one gets to know the `classifier_results` measurement point data for the classified frames. To get the corresponding classified image itself for each of the point data, one need to call into gRPC `GetBlob(imgHandle)` interface exposed by DataAgent. There are both `go` and `py` gRPC client implementation available. For more details, refer `gRPC server module testing alone` section in `DataAgent/README.md`