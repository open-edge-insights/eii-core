
Bare-metal setup of ETA solution:
=====================================
--------------------------------------

## <u>Pre-requisities</u>:

* **Setting up GO dev env (Install latest Go 1.10.3 version or 1.9 version)**
    * Follow guide [go installation](https://golang.org/doc/install#install) to install latest golang and setting up go workspace directory (have 2 folders: src and bin here)
    * Export GOPATH to point to your go workspace dir and GOBIN to $GOPATH/bin
    * Clone this repo under $GOPATH/src
    * Install gRPC, protocal buffer compiler by following steps mentioned @ [go grpc quick start](https://grpc.io/docs/quickstart/go.html). As gRPC code file already exists, no need to re-generate the gRPC code file (da.pb.go) here, it's only needed if da.proto is been changed.

> Note:
> 1. **go build** puts the binary artifact in the current folder whereas **go install** puts it in the $GOPATH/bin folder. Command **go clean** deletes the artifact specific to the project
> 2. Run cmd `dep ensure -vendor-only -v` to pull all the dependencies for all Go modules from the project directory. One can get the `dep` tool by running this cmd: `go get -u github.com/golang/dep/cmd/dep`. For more details on how to use `dep`, visit [here](https://gist.github.com/subfuzion/12342599e26f5094e4e2d08e9d4ad50d)

* **Setting up python dev env**
    * Install python3. Follow guide [python3 installation](http://docs.python-guide.org/en/latest/starting/install3/linux/)
    * One can run the python programs from the $GOPATH/src/ElephantTrunkArch
    * Change directory to above repo and export PYTHONPATH=[abspathto_ElephantTrunkArch_folder]:[abspathto_ElephantTrunkArch_folder]/DataAgent/da_grpc/protobuff
    * Install gRPC, gRPC tools by running below commands. More details @ [python grpc quick start](https://grpc.io/docs/quickstart/python.html)
        * `sudo -H pip3 install grpcio`
        * `sudo -H pip3 install grpcio-tools`

    As the files already exist, no need to re-generate the the 2 py files da_pb2.py and da_pb2_grpc.py here, it's only needed if da.proto is been changed.
    > Note:
    > 1. While running python programs, one may face issues with missing dependency modules. Please use **sudo -H pip3 install  <module>** cmd to install each.

* **Setting up C++ dev env**
    * Run the [DataAgent/da_grpc/test/cpp/examples/setup_ubuntu_dev_env_cpp.sh](DataAgent/da_grpc/test/cpp/examples/setup_ubuntu_dev_env_cpp.sh) script file after copying it to the directory where you need grpc to be installed. Give necessary permissions required.
    * In case of any issues running the above script file, use the following guide
        (https://github.com/grpc/grpc/blob/master/BUILDING.md)
    * To verify successfull installation, try running gRPC C++ HelloWorld example:
        * cd grpc/examples/cpp/helloworld
        * make
        * ./greeter_server
        * ./greeter_client (In a separate terminal)
        * Terminal displaying Greeter received: Hello world on correct installation.
    * Refer DataAgent README for further instructions on how to run gRPC C++ client.
    

* Have the TICK stack softwares (influx, kapacitor - [refer](https://www.digitalocean.com/community/tutorials/how-to-monitor-system-metrics-with-the-tick-stack-on-ubuntu-16-04)) and [Redis](https://askubuntu.com/questions/868848/how-to-install-redis-on-ubuntu-16-04) installed locally. 

* Copy all the video files from "\\Vmspfsfsbg01\qsd_sw_ba\FOG\Validation\validation_videos" in the `test_videos` folder under `ElephantTrunkArch` folder

## <u>Steps to run ETA modules</u>:

* Start all dependency modules:
    - influxdb: **sudo service influxd [start|stop|status|restart]**
    - redis: **sudo service redis [start|stop|status|restart]**
    - mosquitto:  **sudo service mosquitto [start|stop|status|restart]**
    
* Start the DataAgent by providing the right config file having the right configs. Refer [DataAgent/README.md](DataAgent/README.md)

* Start DataAnalytics module by referring to [DataAnalytics/README.md](DataAnalytics/README.md)

* Start OPCUA message bus client to listen on topic `classifier_results` by running below cmd in another terminal (For more details on dependencies to be installed, refer: [DataBusAbstraction/README.md](DataBusAbstraction/README.md):

    ```sh
    python2.7 DataBusAbstraction/py/test/DataBusTest.py --endpoint opcua://0.0.0.0:65003/elephanttrunk --direction SUB --ns streammanager --topic classifier_results
    ``` 

* Start VideoIngestion/VideoIngestion.py: `python3 VideoIngestion/VideoIngestion.py --config factory.json` to ingest the data to InfluxDB/ImageStore. Refer **VideoIngestion/README.md** for more details.

    **Note**: To test with video file, use `factory.json` and to test with Basler's camera, use `factory_prod.json`. Just provide the right serial number for the camera in `factory_prod.json` under `basler` json field

* See the terminal of classifier.py to see the analysis results. The results are also published to mqtt topic. The result images can be seen in the ~/saved_images (as per the factory.json configuration)

* The data being ingested can be seen coming into the DataAgent via StreamManager's UDP subscription to Influxdb which is published to OPCUA message bus and the same should be seen being received in the OPCUA client terminal started in `step 4`

* Using OPCUA client one gets to know the `classifier_results` measurement point data for the classified frames. To get the corresponding classified image itself for each of the point data, one need to call into gRPC `GetBlob(imgHandle)` interface exposed by DataAgent. There are both `go` and `py` gRPC client implementation available. For more details, refer `gRPC server module testing alone` section in [DataAgent/README.md](DataAgent/README.md)