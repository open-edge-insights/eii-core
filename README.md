Edge Insights Software (EIS) is the framework for enabling smart manufacturing with visual and point defect inspections.

# Contents:

1. [Minimum System Requirements](#minimum-system-requirements)

2. [Docker pre-requisites](#docker-pre-requisities)

3. [EIS Pre-requisites](#eis-pre-requisites)

4. [Provision EIS](#provision-eis)

5. [Build / Run EIS PCB Demo Example](#build-and-run-eis-pcb-demo-example)

6. [Custom Udfs](#custom-udfs)

7. [Etcd Secrets and MessageBus Endpoint Configuration](#etcd-secrets-and-messagebus-endpoint-configuration)

8. [Enable camera based Video Ingestion](#enable-camera-based-video-ingestion)

9. [Using video accelerators](#using-video-accelerators)

10. [Time-series Analytics](#time-series-analytics)

11. [DiscoveryCreek](#DiscoveryCreek)

12. [List of All EIS services](#list-of-all-eis-services)

13. [EIS multi node cluster provision and deployment using Turtlecreek](#eis-multi-node-cluster-provision-and-deployment-using-turtlecreek)

14. [Debugging options](#debugging-options)



# Minimum System Requirements

EIS software will run on the below mentioned Intel platforms:

```
* 6th generation Intel® CoreTM processor onwards OR
  6th generation Intel® Xeon® processor onwards OR
  Pentium® processor N4200/5, N3350/5, N3450/5 with Intel® HD Graphics
* At least 16GB RAM
* At least 64GB hard drive
* An internet connection
* Ubuntu 18.04
```

For performing Video Analytics, a 16GB of RAM is recommended.
For time-series ingestion and analytics, a 2GB RAM is sufficient.
The EIS is validated on Ubuntu 18.04 and though it can run on other platforms supporting docker, it is not recommended.


# Docker Pre-requisities

1. **Installing docker daemon and docker-compose tools with proxy settings configuration**.

  Please follow the steps mentioned in [Installing_docker_pre_requisites.md](./Installing_docker_pre_requisites.md) for installing docker daemon and docker-compose tool

2. **Optional:** For enabling full security, make sure host machine and docker daemon are configured with below security recommendations. [build/docker_security_recommendation.md](build/docker_security_recommendation.md)

3. **Optional:** If one wishes to enable log rotation for docker containers

    There are two ways to configure logging driver for docker containers

    * Set logging driver as part of docker daemon (**applies to all docker containers by default**):

        * Configure `json-file` driver as default logging driver by following [https://docs.docker.com/config/containers/logging/json-file/](https://docs.docker.com/config/containers/logging/json-file/). Sample json-driver config which can be copied to `/etc/docker/daemon.json` is provided below.

            ```
            {
                "log-driver": "json-file",
                "log-opts": {
                "max-size": "10m",
                "max-file": "5"
                }
            }
            ```

        * Reload the docker daemon
            ```
            $ sudo systemctl daemon-reload
            ```
        * Restart docker
            ```
            $ sudo systemctl restart docker
            ```

    * Set logging driver as part of docker compose which is conatiner specific and which always overwrites 1st option (i.e /etc/docker/daemon.json)

        Example to enable logging driver only for video_ingestion service:

        ```
        ia_video_ingestion:
            ...
            ...
            logging:
             driver: json-file
             options:
              max-size: 10m
              max-file: 5
        ```

# EIS Pre-Requisites

The section assumes the EIS software is already downloaded from the release package or from git.

## 1. Generating consolidated docker-compose.yml, eis_config.json, module_spec.json, app_spec.json and eis-k8s-deploy.yml files:

EIS is equipped with [eis_builder](build/eis_builder.py), a robust python tool to auto-generate the required configuration files to deploy EIS services on single/multiple nodes. The tool is    capable of auto-generating the following consolidated files by fetching the respective files from EIS service directories which are required to bring up different EII use-cases:

| file name                    | Description   |
| ---------------------------- | ------------- |
| docker-compose.yml           | Consolidated `docker-compose.yml` file used to launch EIS docker containers in a given single node using `docker-compose` tool                                       |
| docker-compose.override.yml  | Consolidated `docker-compose-dev.override.yml` of every app that is generated only in DEV mode for EIS deployment on a given single node using `docker-compose` tool |
| eis_config.json              | Consolidated `config.json` of every app which will be put into etcd during provisioning                                                                              |
| module_spec.json             | Consolidated `module_spec.json` of every app that is required to register an artifact required for CSL use case                                                      |
| app_spec.json                | Consolidated `app_spec.json` of every app that is required to deploy EIS services via CSL orchestrator                                                               |
| eis-k8s-deploy.yml           | Consolidated `k8s-service.yml` of every app that is required to deploy EIS servcie via Kubernetes orchestrator                                                       |
  
> **NOTE**:
> 1. Whenever we make changes to individual EII app/service directories files as mentioned above in the description column, 
     it is required to re-run the eis_builder.py script before provisioning and running the EII stack to ensure that the 
     changes done reflect in the required consolidated files.
> 2. Manual editing of above consolidated files is not recommended and we would recommend to do the required changes to 
     respective files in EII app/service directories and use EIS builder script to generate the conslidated ones.

### 2. Using EIS builder script

#### * Install requirements

```sh
$ cd [WORKDIR]/IEdgeInsights/build
# Install requirements for eis_builder.py
$ pip3 install -r requirements.txt
```

#### * Running EIS builder

EIS builder script usage:

```sh
$ python3.6 eis_builder.py -h
usage: eis_builder.py [-h] [-f YML_FILE] [-v VIDEO_PIPELINE_INSTANCES]
                    [-d OVERRIDE_DIRECTORY]

optional arguments:
    -h, --help            show this help message and exit
    -f YML_FILE, --yml_file YML_FILE
                        Optional config file for list of services to include.
                        Eg: python3.6 eis_builder.py -f video-streaming.yml
                        (default: None)
    -v VIDEO_PIPELINE_INSTANCES, --video_pipeline_instances VIDEO_PIPELINE_INSTANCES
                        Optional number of video pipeline instances to be
                        created. Eg: python3.6 eis_builder.py -v 6 (default:
                        1)
    -d OVERRIDE_DIRECTORY, --override_directory OVERRIDE_DIRECTORY
                        Optional directory consisting of of benchmarking
                        configs to be present in each app directory. Eg:
                        python3.6 eis_builder.py -d benchmarking (default:
                        None)
```


* `Running eis_builder to generate the above listed consolidated files for all applicable EIS services`:

   EIS builder will parse the top level directories under **IEdgeInsights** to generate the above listed consolidated files.

   ```sh
   $ python3 eis_builder.py
   ```

* `Running eis_builder to generate the above listed consolidated files for a subset of EIS services`:

   This is achieved by providing a yml file to EIS builder as config which has list of services to include. User can mention the service name as path relative to **IEdgeInsights** or Full path to the service in the config yml file.

  If user wants to include only a certain number of services in the EIS stack, he can opt to provide the **-f or yml_file** flag of eis_builder to allow only the services provided in the yml file mentioned with the **-f or yml_file**. Few examples of such yml files for different usecases are provided at [video](build/video-streaming.yml), [time-series](build/time-series.yml), [Azure](build/video-streaming-azure.yml), [TLS](build/video-streaming-tls.yml). 
  
  An example for running EIS builder with this flag is given below:

  ```sh
  $ python3 eis_builder.py -f video-streaming.yml
  ```

* `Running eis_builder to generate multi instance configs`:

  Based on the user's requirements, eis_builder can also generate multi-instance docker-compose.yml, config.json, k8s-service.yml, csl_app_spec.json & every module's module_spec.json respectively.

  If user wants to generate boiler plate config for multiple stream use cases, he can do so by using the **-v or video_pipeline_instances** flag of eis_builder. This flag creates multi stream boiler plate config for docker-compose.yml, eis_config.json, csl app_spec.json, csl module_spec.json & k8s k8s-service.yml files respectively.
  
  An example for running eis_builder to generate multi instance boiler plate config for 3 streams of **video-streaming** use case has been provided below:

  ```sh
  $ python3 eis_builder.py -v 3 -f video-streaming.yml
  ```

  > **NOTE**: This multi-instance feature support of EIS builder works only for the video pipeline i.e., **video-streaming.yml** use case alone and not with any other use case yml files like **video-streaming-storage.yml** etc., Also, it doesn't work for cases without `-f` switch too. In other words, only the above example works with `-v` taking in any +ve number

* `Running eis_builder to generate benchmarking configs`:
   
  If user wants to provide a different set of docker-compose.yml, config.json, csl module_spec.json, csl app_spec.json & k8s k8s-service.yml other than the ones present in every service directory, he can opt to provide the **-d or override_directory** flag which indicates to search for these required set of files within a directory provided by the flag. For example, if user wants to pick up these files from a directory named **benchmarking**, he can run the command provided below:

  ```sh
  $ python3 eis_builder.py -d benchmarking
  ```

    > **Note:**
    > * If using the override directory feature of eis_builder, it is recommended to include set of all 5 files mentioned above. Failing to provide any of the files in the override directory results in eis_builder not including that service in the generated final config. Eg: If a user fails to provide an app_spec.json in the override directory for a particular service, the final [csl_app_spec.json](build/csl/csl_app_spec.json) will not include the app_spec of that service.
    > * If user wishes to spawn a single Subscriber/Client container subscribing/receiving on multiple Publisher/Server containers, he can do so by adding the AppName of Subscriber/Client container in **subscriber_list** of [eis_builder_config.json](build/eis_builder_config.json) ensuring the Publisher/Server container **AppName** is added in the **publisher_list** of [eis_builder_config.json](build/eis_builder_config.json). For services not mentioned in **subscriber_list**, multiple containers specified by the **-v** flag are spawned.
    For eg: If eis_builder is run with **-v 3** option and **Visualizer** isn't added in **subscriber_list** of [eis_builder_config.json](build/eis_builder_config.json), 3 **Visualizer** instances are spawned, each of them subscribing to 3 **VideoAnalytics** services. If **Visualizer** is added in **subscriber_list** of [eis_builder_config.json](build/eis_builder_config.json), a single **Visualizer** instance subscribing to 3 multiple **VideoAnalytics** is spawned.

 #### 3. Adding new EIS service so it gets picked up by EIS builder

Since the eis_builder takes care of registering and running any service present in it's own directory in the [IEdgeInsights](./) directory, this section describes on how to add any new service the user wants to add into the EIS stack, subscribe to [VideoAnalytics](./VideoAnalytics) and publish on a new port.

Any service that needs to be added into the EIS stack should be added as a new directory in the [IEdgeInsights](./) directory. The directory should contain a **docker-compose.yml** which will be used to deploy the service as a docker container and it should also contain a **config.json** which contains the required config for the service to run once it is deployed. The **config.json** will mainly consist of a **config** section which includes the configuration related parameters required to run the application and an **interfaces** section which includes the configuration of how this service interacts with other services of the EIS stack. The **AppName** present in **environment** section in **docker-compose.yml** file is appended to the **config** & **interfaces** like **/AppName/config** & **/AppName/interfaces** before being put into the main [eis_config.json](build/provision/config/eis_config.json). Additionally, if the EIS service needs to be deployed over CSL orchestrator,then the EIS service directory should have **module_spec.json** and **app_spec.json* files defined and for deployment over k8s orchestrator, the **k8s-service.yml** file needs to be defined.

An example has been provided below on how to write the **config.json** for any new service, subscribe it to **VideoAnalytics** and publish on a new port:

```javascript
    {
        "config": {
            "paramOne": "Value",
            "paramTwo": [1, 2, 3],
            "paramThree": 4000,
            "paramFour": true
        },
        "interfaces": {
            "Subscribers": [
                {
                    "Name": "default",
                    "Type": "zmq_tcp",
                    "EndPoint": "127.0.0.1:65013",
                    "PublisherAppName": "VideoAnalytics",
                    "Topics": [
                        "camera1_stream_results"
                    ]
                }
            ],
            "Publishers": [
                {
                    "Name": "default",
                    "Type": "zmq_tcp",
                    "EndPoint": "127.0.0.1:65113",
                    "Topics": [
                        "publish_stream"
                    ],
                    "AllowedClients": [
                        "ClientOne",
                        "ClientTwo",
                        "ClientThree"
                    ]
                }
            ]
        }
    }
```

In the above specified **config.json**, the value of **config** key is the config required by the service to run and the value of the **interfaces** key is the config required by the service to 
interact with other services of EIS stack over EIS message bus.

The **Subscribers** value in the **interfaces** section denotes that this service should act as a subscriber to the stream being published by the value specified by **PublisherAppName** on the 
endpoint mentioned in value specified by **EndPoint** on topics specified in value of **Topics** key.

The **Publishers** value in the **interfaces** section denotes that this service publishes a stream of data after obtaining and processing it from **VideoAnalytics**. The stream is published on 
the endpoint mentioned in value of **EndPoint** key on topics mentioned in the value of **Topics** key. The services mentioned in the value of **AllowedClients** are the only clients able to 
subscribe to the published stream if being published securely over the EISMessageBus.

Similar to above interface keys, EIS services can also have "Servers" and "Clients" interface keys too. For example, check [config.json](VideoIngestion/config.json) of VideoIngestion service and [config.json](tools/SWTriggerUtility/config.json) of SWTriggerUtility tool on how to use.

# Provision EIS

<b>`By default EIS is provisioned in Secure mode`</b>.

Follow below steps to provision EIS. Provisioning must be done before deploying EIS on any node. It will start ETCD as a container and load it with configuration required to run EIS for single node or multi node cluster set up.

Please follow below steps to provision EIS in Developer mode. Developer mode will have all security disabled.

* Please update DEV_MODE=true in [build/.env](build/.env) to provision EIS in Developer mode.
* <b>Please comment secrets section for all services in [build/docker-compose.yml](../docker-compose.yml)</b>

Following actions will be performed as part of Provisioning

 * Loading inital ETCD values from json file located at [build/provision/config/eis_config.json](build/provision/config/eis_config.json).
 * For Secure mode, Generating ZMQ secret/public keys for each app and putting them in ETCD.
 * Generating required X509 certs and putting them in etcd.
 * All server certificates will be generated with 127.0.0.1, localhost and HOST_IP mentioned in [build/.env](build/.env).
 * If HOST_IP is blank in [build/.env](build/.env), then HOST_IP will be automatically detected when server certificates are generated.

**Optional:** In case of cleaning existing volumes, please run the [volume_data_script.py](build/provision/volume_data_script.py). The script can be run by the command:
```sh
$ python3.6 volume_data_script.py
```

Below script starts `etcd` as a container and provision EIS. Please pass docker-compose file as argument, against which provisioning will be done.
```sh
$ cd [WORKDIR]/IEdgeInsights/build/provision
$ sudo ./provision_eis.sh <path_to_eis_docker_compose_file>

# eq. $ sudo ./provision_eis.sh ../docker-compose.yml

```
**Optional:** For capturing the data back from ETCD Cluster to a JSON file, run the [etcd_capture.sh](build/provision/etcd_capture.sh) script. This can be achieved using the following command:
```sh
$ ./etcd_capture.sh
```

# Build and Run EIS PCB video/timeseries use cases

  ---
  > **Note:**
  > * If `ia_visualizer` service is enabled in the [docker-compose.yml](build/docker-compose.yml) file, please
     run command `$ xhost +` in the terminal before starting EIS stack, this is a one time configuration.
     This is needed by `ia_visualizer` service to render the UI
  > * For running EIS services in IPC mode, make sure that the same user should be there in publisher and subscriber.
     If publisher is running as root (eg: VI, VA), then the subscriber also need to run as root.
     In [docker-compose.yml](build/docker-compose.yml) if `user: ${EIS_UID}` is in publisher service, then the
     same `user: ${EIS_UID}` has to be in subscriber service. If the publisher doesn't have the user specified like above,
     then the subscriber service should not have that too.
  > * In case multiple VideoIngestion or VideoAnalytics services are needed to be launched, then the
     [docker-compose.yml](build/docker-compose.yml) file can be modified with the required configurations and
     below command can be used to build and run the containers.
  >    ```sh
  >     $ docker-compose up --build -d
  >     ```
  ---

All the below EIS build and run commands to be executed from the [WORKDIR]/IEdgeInsights/build/ directory.

Below are the usecases supported by EIS to bring up the respective services mentioned in the
yaml file.

## Main usecases

| Usecase                    | yaml file                                                |
| :---                       | :---                                                    |
| Video + Timeseries         | [build/video-timeseries.yml](build/video-timeseries.yml) |
| Video                      | [build/video.yml](build/video.yml)                       |
| Timeseries                 | [build/time-series.yml](build/time-series.yml)           |

## Video pipeline sub-usecases

| Usecase                                | yaml file                                                               |
| :---                                   | :---                                                                    |
| Video streaming                        | [build/video-streaming.yml](build/video-streaming.yml)                  |
| Video streaming and historical         | [build/video-streaming-storage.yml](build/video-streaming-storage.yml)  |
| Video streaming with EISAzureBridge    | [build/video-streaming-azure.yml](build/video-streaming-azure.yml)      |
| Video streaming with TLSRemoteAgent    | [build/video-streaming-tls.yml](build/video-streaming-tls.yml)          |
| Video streaming and custom udfs        | [build/video-streaming-all-udfs.yml](build/video-streaming-all-udfs.yml)|

To build and run EIS in one command:

```sh
$ docker-compose up --build -d
```

The build and run steps can be split into two as well like below:

```sh
$ docker-compose build
$ docker-compose up -d
```

If any of the services fails during build, it can be built using below command

```sh
$ docker-compose build --no-cache <service name>
```

Please note that the first time build of EIS containers may take ~70 minutes depending on the n/w speed.

A successful run will open Visualizer UI with results of video analytics for all video usecases.

# Custom Udfs

The following are the two Custom Udfs workflow which EIS supports:

1. Build / Run custom udfs as standalone applications

   For running custom udfs as standalone application one must download the video-custom-udfs repo and refer [CustomUdfs/README.md](CustomUdfs/README.md)

2. Build / Run custom udfs in VI or VA

   For running custom udfs either in VI or VA one must refer [VideoIngestion/docs/custom_udfs_doc.md](VideoIngestion/docs/custom_udfs_doc.md)

# Etcd Secrets and MessageBus Endpoint Configuration

Etcd Secrets and MessageBus endpoint configurations are done to establish the data path
and configuration of various EIS containers.

Every service in [build/docker-compose.yml](build/docker-compose.yml)
is a
* messagebus client if it needs to send or receive data over EISMessageBus
* etcd client if it needs to get data from etcd distributed key store

For more details, visit [Etcd_Secrets_and_MsgBus_Endpoint_Configuration](./Etcd_Secrets_and_MsgBus_Endpoint_Configuration.md)

# Enable camera based Video Ingestion

EIS supports various cameras like Basler (GiGE), RTSP and USB camera. The video ingestion pipeline is enabled using 'gstreamer' which ingests the frames from the camera. The Video Ingestion application accepts a user-defined filter algorithm to do pre-processing on the frames before it is ingested into the DBs and inturn to the Analytics container.

All the changes related to camera type are made in the Etcd ingestor configuration values and sample ingestor configurations are provided in [VideoIngestion/README.md](VideoIngestion/README.md) for reference.

For detailed description on configuring different types of cameras and  filter algorithms, refer to the [VideoIngestion/README.md](VideoIngestion/README.md).

For Sample docker-compose file and ETCD preload values for multiple camaras, refer to [build/samples/multi_cam_sample/README.md](build/samples/multi_cam_sample/README.md).

# Using video accelerators

EIS supports running inference on `CPU`, `GPU`, `MYRIAD`(NCS2), and `HDDL` devices by accepting `device` value ("CPU"|"GPU"|"MYRIAD"|"HDDL"), part of the `udf` object configuration in `udfs`
key. The `device` field in UDF config of `udfs` key in `VideoIngestion` and `VideoAnalytics` configs can either be changed in the [eis_config.json](build/provision/config/eis_config.json)
before provisioning (or reprovision it again after the change) or at run-time via EtcdUI. For more details on the udfs config,
check [common/udfs/README.md](common/udfs/README.md).

* For actual deployment in case USB camera is required then mount the device node of the USB camera for `ia_video_ingestion` service. When multiple USB cameras are connected to host m/c the required camera should be identified with the device node and mounted.

    Eg: Mount the two USB cameras connected to the host m/c with device node as `video0` and `video1`
    ```
     ia_video_ingestion:
        ...
        devices:
               - "/dev/dri"
               - "/dev/video0:/dev/video0"
               - "/dev/video1:/dev/video1"
    ```

    Note: /dev/dri is needed for Graphic drivers

* **To run on HDDL devices**

  * Download the full package for OpenVINO toolkit for Linux version "2021.2" (`OPENVINO_IMAGE_VERSION` used in [build/.env](build/.env)) from the official website
  (https://software.intel.com/en-us/openvino-toolkit/choose-download/free-download-linux).

  Please refer to the OpenVINO links below for to install and running the HDDL daemon on host.

  1. OpenVINO install:
     https://docs.openvinotoolkit.org/2021.2/_docs_install_guides_installing_openvino_linux.html#install-openvino
  2. HDDL daemon setup:
     https://docs.openvinotoolkit.org/2021.2/_docs_install_guides_installing_openvino_linux_ivad_vpu.html

     **NOTE**: OpenVINO 2021.2 installation creates a symbolic link to latest installation with filename as `openvino_2021` instead of `openvino`. Hence one can create a symbolic link with filename as `openvino` to the latest installation using the below steps.

     ```sh
     $ cd /opt/intel
     $ sudo ln -s openvino_2021.2.185 openvino

     In case there are older versions of OpenVINO installed on the host system please un-install them.

     When running on HDDL devices, the HDDL daemon should be running in a different terminal, or in the background like shown below on the host m/c.

     ```sh
     $ source /opt/intel/openvino/bin/setupvars.sh
     $ $HDDL_INSTALL_DIR/bin/hddldaemon
     ```

   * For actual deployment one could choose to mount only the required devices for services using OpenVINO with HDDL (`ia_video_analytics` or `ia_video_ingestion`) in [docker-compose.yml](build/docker-compose.yml).

    Eg: Mount only the Graphics and HDDL ion device for `ia_video_anaytics` service
    ```
      ia_video_analytics:
         ...
         devices:
                 - "/dev/dri"
                 - "/dev/ion:/dev/ion"
    ```
**Note**:
----

* **Troubleshooting issues for MYRIAD(NCS2) devices**

  * Following is an workaround can be excercised if in case user observes `NC_ERROR` during device initialization of NCS2 stick.
     While running EIS if NCS2 devices failed to initialize properly then user can re-plug the device for the init to happen freshly.
     User can verify the successfull initialization by executing ***dmesg**** & ***lsusb***  as below:

     ```sh
     lsusb | grep "03e7" (03e7 is the VendorID and 2485 is one of the  productID for MyriadX)
     ```

     ```sh
     dmesg > dmesg.txt
     [ 3818.214919] usb 3-4: new high-speed USB device number 10 using xhci_hcd
     [ 3818.363542] usb 3-4: New USB device found, idVendor=03e7, idProduct=2485
     [ 3818.363546] usb 3-4: New USB device strings: Mfr=1, Product=2, SerialNumber=3
     [ 3818.363548] usb 3-4: Product: Movidius MyriadX
     [ 3818.363550] usb 3-4: Manufacturer: Movidius Ltd.
     [ 3818.363552] usb 3-4: SerialNumber: 03e72485
     [ 3829.153556] usb 3-4: USB disconnect, device number 10
     [ 3831.134804] usb 3-4: new high-speed USB device number 11 using xhci_hcd
     [ 3831.283430] usb 3-4: New USB device found, idVendor=03e7, idProduct=2485
     [ 3831.283433] usb 3-4: New USB device strings: Mfr=1, Product=2, SerialNumber=3
     [ 3831.283436] usb 3-4: Product: Movidius MyriadX
     [ 3831.283438] usb 3-4: Manufacturer: Movidius Ltd.
     [ 3831.283439] usb 3-4: SerialNumber: 03e72485
     [ 3906.460590] usb 3-4: USB disconnect, device number 11

* **Troubleshooting issues for HDDL devices**
  * ** Note:** HDDL was tested with OpenVINO 2021.2 on Ubuntu 18.04 with kernel version 5.4.0-050400-generic

  * In case one notices shared memory error with OpenVINO 2021.2 on Ubuntu 18.04 with kernel version above 5.3 please downgrade the kernel version. The ION driver could have compatibility issues getting installed with kernel version above 5.3

  * Please verify the hddldaemon started on host m/c to verify if it is using the libraries of the correct OpenVINO version used in [build/.env](build/.env). One could enable the `device_snapshot_mode` to `full` in $HDDL_INSTALL_DIR/config/hddl_service.config on host m/c to get the complete snapshot of the hddl device.

  * Please refer OpenVINO 2021.2 release notes in the below link for new features and changes from the previous versions.
    https://software.intel.com/content/www/us/en/develop/articles/openvino-relnotes.html

  * Refer OpenVINO website in the below link to skim through known issues, limitations and troubleshooting
    https://docs.openvinotoolkit.org/2021.2/index.html

----

# Time-series Analytics

For time-series data, a sample analytics flow uses Telegraf for ingestion, Influx DB for storage and Kapacitor for classification. This is demonstrated with an MQTT based ingestion of sample temperature sensor data and analytics with a Kapacitor UDF which does threshold detection on the input values.

The services mentioned in [build/time-series.yml](build/time-series) will be available in the consolidated [build/docker-compose.yml](build/docker-compose.yml) and consolidated [build/eis_config.json](build/eis_config.json) of the EIS stack for timeseries use case when built via `eis_builder.py` as called out in previous steps.

This will enable building of Telegraf and the Kapacitor based analytics containers.
More details on enabling this mode can be referred from [Kapacitor/README.md](Kapacitor/README.md)

The sample temperature sensor can be simulated using the [tools/mqtt-temp-sensor](tools/mqtt-temp-sensor) application.

# DiscoveryCreek

DiscoveryCreek is a machine learning based anomaly detection engine.

Add the `DiscoveryCreek` entry to [build/time-series.yml](build/time-series) and the services mentioned in there will be available in the consolidated [build/docker-compose.yml](build/docker-compose.yml) and consolidated [build/eis_config.json](build/eis_config.json) of the EIS stack for timeseries use case when built via `eis_builder.py` as called out in previous steps.

More details on enabling DiscoveryCreek based analytics can be referred at [DiscoveryCreek/README.md](DiscoveryCreek/README.md)

# List of All EIS Services

EIS stack comes with following services, which can be included/excluded in docker-compose file based on requirements.

## Common EIS services

1. [EtcdUI](EtcdUI/README.md)
2. [InfluxDBConnector](InfluxDBConnector/README.md)
3. [OpcuaExport](OpcuaExport/README.md) - Optional service to read from VideoAnalytics container to publish data to opcua clients
4. [RestDataExport](RestDataExport/README.md) - Optional service to read the metadata and image blob from InfluxDBConnector and ImageStore services respectively

## Video related services

1. [VideoIngestion](VideoIngestion/README.md)
2. [VideoAnalytics](VideoAnalytics/README.md)
3. [Visualizer](Visualizer/README.md)
4. [WebVisualizer](WebVisualizer/README.md)
5. [ImageStore](ImageStore/README.md)
6. [EISAzureBridge](EISAzureBridge/README.md)
7. [FactoryControlApp](FactoryControlApp/README.md) - Optional service to read from VideoAnalytics container if one wants to control the light based on defective/non-defective data

## Timeseries related services

1. [Telegraf](Telegraf/README.md)
2. [Kapacitor](Kapacitor/README.md)
3. [Grafana](Grafana/README.md)
4. [DiscoveryCreek](DiscoveryCreek/README.md)

# EIS multi node cluster provision and deployment using Turtlecreek

By default EIS is provisioned with Single node cluster. In order to deploy EIS on multiple nodes using docker registry, provision ETCD cluster and
remote managibility using turtlecreek, please follow [build/deploy/README.md](build/deploy/README.md)

# Debugging options

1. To check if all the EIS images are built successfully, use cmd: `docker images|grep ia` and
   to check if all containers are running, use cmd: `docker ps` (`one should see all the dependency containers and EIS containers up and running`). If you see issues where the build is failing due to non-reachability to Internet, please ensure you have correctly configured proxy settings and restarted docker service. Even after doing this, if you are running into the same issue, please add below instrcutions to all the dockerfiles in `build\dockerfiles` at the top after the LABEL instruction and retry the building EIS images:

    ```sh
    ENV http_proxy http://proxy.iind.intel.com:911
    ENV https_proxy http://proxy.iind.intel.com:911
    ```

2. `docker ps` should list all the enabled containers which are included in docker-compose.yml

3. To verify if the default video pipeline with EIS is working fine i.e., from video ingestion->video analytics->visualizer, please check the visualizer UI

4. `/opt/intel/eis` root directory gets created - This is the installation path for EIS:
     * `data/` - stores the backup data for persistent imagestore and influxdb
     * `sockets/` - stores the IPC ZMQ socket files

---
**Note**:
1. Few useful docker-compose and docker commands:
     * `docker-compose build` - builds all the service containers. To build a single service container, use `docker-compose build [serv_cont_name]`
     * `docker-compose down` - stops and removes the service containers
     * `docker-compose up -d` - brings up the service containers by picking the changes done in `docker-compose.yml`
     * `docker ps` - check running containers
     * `docker ps -a` - check running and stopped containers
     * `docker stop $(docker ps -a -q)` - stops all the containers
     * `docker rm $(docker ps -a -q)` - removes all the containers. Useful when you run into issue of already container is in use.
     * [docker compose cli](https://docs.docker.com/compose/reference/overview/)
     * [docker compose reference](https://docs.docker.com/compose/compose-file/)
     * [docker cli](https://docs.docker.com/engine/reference/commandline/cli/#configuration-files)

2. If you want to run the docker images separately i.e, one by one, run the command `docker-compose run --no-deps [service_cont_name]` Eg: `docker-compose run --name ia_video_ingestion --no-deps      ia_video_ingestion` to run VI container and the switch `--no-deps` will not bring up it's dependencies mentioned in the docker-compose file. If the container is not launching, there could be
   some issue with entrypoint program which could be overrided by providing this extra switch `--entrypoint /bin/bash` before the service container name in the docker-compose run command above, this would let one inside the container and run the actual entrypoint program from the container's terminal to rootcause the issue. If the container is running and one wants to get inside, use cmd: `docker-compose exec [service_cont_name] /bin/bash` or `docker exec -it [cont_name] /bin/bash`

3. Best way to check logs of containers is to use command: `docker logs -f [cont_name]`. If one wants to see all the docker-compose service container logs at once, then just run
   `docker-compose logs -f`

---

