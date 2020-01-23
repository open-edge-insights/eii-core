Edge Insights Software (EIS) is the framework for enabling smart manufacturing with visual and point defect inspections.

# Contents:

1. [Minimum System Requirements](#minimum-system-requirements)

2. [Docker pre-requisites](#docker-pre-requisities)

3. [EIS Pre-requisites](#eis-pre-requisites)

4. [Provision EIS](#provision-eis)

5. [Build / Run EIS PCB Demo Example](#build-and-run-eis-pcb-demo-example)

6. [Etcd Secrets and MessageBus Endpoint Configuration](#etcd-secrets-and-messagebus-endpoint-configuration)

7. [Enable camera based Video Ingestion](#enable-camera-based-video-ingestion)

8. [Using video accelerators](#using-video-accelerators)

9. [Time-series Analytics](#time-series-analytics)

10. [DiscoveryCreek](#DiscoveryCreek)

11. [EIS multi node cluster provision and deployment using Turtlecreek](#eis-multi-node-cluster-provision-and-deployment-using-turtlecreek)

12. [Debugging options](#debugging-options)



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

   This could be done in 2 ways:

   * [**Recommended**] Using setup script from `EIS Installer` available in the release package
     to auto-install and configure

     Follow the below steps in `installer` repo/folder:
     * Add below 2 entries in `installer/installation/config/config.cfg` file
        
       ```sh
       dockerce
       docker_compose
       ```
    
     * Follow the pre-requisite section to install EIS from `installer/README.md`
     * Run the below steps to install above binaries:
        
       ```sh
       $ cd installer/installation
       $ chmod +x setup.sh
       $ sudo ./setup.sh
        ```
     * Logout and log back in to manage docker as a non-root user
   
   * If one wants to install the tools and configure it manually, please follow the steps mentioned in [Installing_docker_pre_requisites.md](./Installing_docker_pre_requisites.md)

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

* Download the full package for OpenVINO toolkit for Linux version "2020.1" from the official website
  (https://software.intel.com/en-us/openvino-toolkit/choose-download/free-download-linux), extract it and copy the
  directory `l_openvino_toolkit_xxxxx/` inside `[repo]/common/openvino`.

   > **NOTE**: Make sure there is always one `l_openvino_toolkit_xxxxx/` folder under [repo]/common/openvino folder as 
   > we are adding `l_openvino_toolkit_*` into Dockerfile which could result in  build failure of 
   > openvino base container if there are multiple openvino sdk's in there (especially the old ones)

# Provision EIS

<b>`By default EIS is provisioned in Secure mode`</b>.

Follow below steps to provision EIS. Provisioning must be done before deploying EIS on any node. It will start ETCD as a container and load it with configuration required to run EIS for single node or multi node cluster set up.

Please follow below steps to provision EIS in Developer mode. Developer mode will have all security disabled.

* Please update DEV_MODE=true in [build/.env](build/.env) to provision EIS in Developer mode.
* <b>Please comment secrets section for all services in [build/docker-compose.yml](../docker-compose.yml)</b>

Following actions will be performed as part of Provisioning

 * Loading inital ETCD values from json file located at [build/provision/config/etcd_pre_load.json](build/provision/config/etcd_pre_load.json).
 * For Secure mode, Generating ZMQ secret/public keys for each app and putting them in ETCD.
 * Generating required X509 certs and putting them in etcd.
 * All server certificates will be generated with 127.0.0.1, localhost and HOST_IP mentioned in [build/.env](build/.env).
 * If HOST_IP is blank in [build/.env](build/.env), then HOST_IP will be automatically detected when server certificates are generated.

**Optional:** In case of cleaning existing volumes, please run the [volume_data_script.py](build/provision/volume_data_script.py). The script can be run by the command:
```
python3.6 volume_data_script.py
```

Below script starts `etcd` as a container and provision EIS. Please pass docker-compose file as argument, against which provisioning will be done.
```
$ cd [repo]/build/provision
$ sudo ./provision_eis.sh <path_to_eis_docker_compose_file>

eq. $ sudo ./provision_eis.sh ../docker-compose.yml

```
**Optional:** For capturing the data back from ETCD Cluster to a JSON file, run the [etcd_capture.sh](build/provision/etcd_capture.sh) script. This can be achieved using the following command:
```
./etcd_capture.sh
```

# Build and Run EIS PCB/timeseries use cases

  ---
  > **Note:**
  > * All EIS build and run commands are to be executed from the [repo]/build/ directory.
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
    
  
Below are the main usecases supported by EIS:

* For video streaming use case only:
  Refer our default docker-compose file at [build/docker-compose.yml](build/docker-compose.yml)

    > ia_video_ingestion, ia_video_analytics, ia_visualizer

* For video streaming and historical use case:
  Sample docker-compose file is accessible at [build/samples/docker-compose-video-streaming-and-historical-usecase.yml](build/samples/docker-compose-video-streaming-and-historical-usecase.yml)

    > ia_video_ingestion, ia_video_analytics, ia_visualizer, ia_imagestore, ia_influxdbconnector

* For timeseries use case only:
  Sample docker-compose file is accessible at [build/samples/docker-compose-timeseries-usecase.yml](build/samples/docker-compose-timeseries-usecase.yml)

    > ia_grafana, ia_telegraf, ia_influxdbconnector, ia_dc, ia_kapacitor
  
* For video streaming and timeseries use case:
  Sample docker-compose file is accessible at [build/samples/docker-compose-video-streaming-timeseries-usecase.yml](build/samples/docker-compose-video-streaming-timeseries-usecase.yml)

    > ia_video_ingestion, ia_video_analytics, ia_visualizer, ia_grafana, ia_telegraf, ia_influxdbconnector, ia_dc, ia_kapacitor

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

Please note that the first time build of EIS containers may take ~70 minutes depending the n/w speed.

A successful run will open Visualizer UI with results of video analytics for all video usecases.

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

EIS supports running inference on `CPU`, `GPU`, `MYRIAD`, `HDDL` and `FPGA` devices by accepting `device` value ("CPU"|"GPU"|"MYRIAD"|"HDDL"|"HETERO:FPGA,CPU"|"HETERO:FPGA,GPU"), part of the `udf` object configuration in `udfs`
key. The `device` field in UDF config of `udfs` key in `VideoIngestion` and `VideoAnalytics` configs can either be changed in the [etcd_pre_load.json](build/provision/config/etcd_pre_load.json)
before provisioning (or reprovision it again after the change) or at run-time via EtcdUI. For more details on the udfs config, 
check [common/udfs/README.md](common/udfs/README.md).

**Note**:
----
* **Troubleshooting issues for HDDL and HDDLF(FGPA) devices**

  * Running HDDL devices on Ubuntu 18.04 with Kernel 5.3 is not supported by OpenVINO 2020.1 so please downgrade the kernel version
    (HDDL was tested with OpenVINO 2020.1 on Ubuntu 18.04 with kernel version 4.15.0-76-generic)

  * Please refer OpenVINO 2020.1 release notes in the below link for new features and changes from the previous versions.
    https://software.intel.com/en-us/articles/OpenVINO-RelNotes

  * Refer OpenVINO website in the below link to skim through known issues, limitations and troubleshooting
    https://docs.openvinotoolkit.org/2020.1/index.html

  * The OS Kernel version shouldn't be greater than 4.18 for FPGA accelerator inference, Please follow below steps to downgrade the Kernel on host m/c.
     ```
    $ sudo gedit /etc/default/grub
    ```
    Change GRUB_TIMEOUT to -1 and run below command.
    ```
    $ sudo update-grub
    ```
    Reboot the machine with `sudo reboot`. Please select the Kernel version 4.18 or lower in the grub menu.

* **To run on HDDL devices**

  Please refer to the OpenVINO links below for to install and running the HDDL daemon on host.

  1. OpenVINO install:
     https://docs.openvinotoolkit.org/2020.1/_docs_install_guides_installing_openvino_linux.html#install-openvino
  2. HDDL daemon setup:
     https://docs.openvinotoolkit.org/2020.1/_docs_install_guides_installing_openvino_linux_ivad_vpu.html
  3. No Extra steps are required if using the iEi Mustang V100 HDDL card with OpenVINO 2020.1

     When running on HDDL devices, the HDDL daemon should be running in a different terminal, or in the background like shown below on the host m/c.

     ```sh
     $ source /opt/intel/openvino/bin/setupvars.sh
     $ $HDDL_INSTALL_DIR/bin/hddldaemon &
     ```

  4. Additionally following is an workaround can be excercised if in case user observes NC_ERROR during device initialization of NCS2 stick.
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
     ```

* **To run on HDDLF(FGPA) devices**

  Please Refer the [README_fpga.md](README_fpga.md) for installing FPGA drivers. Once that is setup, please follow below steps:
  
  * Set the argument `FPGA_ENABLE` value to `enable` to build `ia_openvino_base` service with all the required FPGA config in [docker-compose.yml](build/docker-compose.yml).
    If one is using docker-compose samples, please ensure to follow the same.
  
    Eg: Just set the value of `FPGA_ENABLE` to `enable` like below for `ia_openvino_base` service:
    ```sh
      ia_openvino_base:
        ...
        args: ...
              FPGA_ENABLE: "enable"
        ...
    ```
  * Please provision, build and run the EIS stack as mentioned in the `Provision EIS` and `Build and Run EIS PCB Demo Example` sections above. 
    For `FPGA` inferencing, make sure to set `device` value to `HETERO:FPGA,CPU` or `HETERO:FPGA,GPU"` in the `udf` object configuration in
    the `udfs` key. The `device` field in UDF config of `udfs` key in `VideoIngestion` and `VideoAnalytics` configs can either be changed in the [etcd_pre_load.json](build/provision/config/etcd_pre_load.json)
    before provisioning (or reprovision it again after the change) or at run-time via EtcdUI. For more details on the udfs config, 
    check [common/udfs/README.md](common/udfs/README.md).

----

# Time-series Analytics

For time-series data, a sample analytics flow uses Telegraf for ingestion, Influx DB for storage and Kapacitor for classification. This is demonstrated with an MQTT based ingestion of sample temperature sensor data and analytics with a Kapacitor UDF which does threshold detection on the input values.

For enabling this, different set of containers need to be built in EIS and it can be selected by modifying the docker-compose file.

Please incldue following services in [docker-compose.yml](build/docker-compose.yml) for Time series analytics example.

> ia_telegraf, ia_influxdbconnector, ia_kapacitor, ia_grafana

This will enable building of Telegraf and the Kapacitor based analytics containers.
More details on enabling this mode can be referred from [Kapacitor/README.md](Kapacitor/README.md)

The sample temperature sensor can be simulated using the [tools/mqtt-temp-sensor](tools/mqtt-temp-sensor) application.

For sample docker-compose file for TimeSeries analytics , refer to [build/samples/docker-compose-time-series-analytics.yml](build/samples/docker-compose-time-series-analytics.yml).

# DiscoveryCreek

DiscoveryCreek is a machine learning based anomaly detection engine.

For enabling DiscoveryCreek, please include the following services in the [docker-compose.yml](build/docker-compose.yml) file:

> ia_grafana, ia_telegraf, ia_influxdbconnector, ia_dc

More details on enabling DiscoveryCreek based analytics can be referred at [DiscoveryCreek/README.md](DiscoveryCreek/README.md)

For sample docker-compose file for DiscoveryCreek analytics , refer to [build/samples/docker-compose-discovery-creek.yml](build/samples/docker-compose-discovery-creek.yml).

# List of All EIS Services

EIS stack comes with following services, which can be included/excluded in docker-compose file based on requirements.

1. [VideoIngestion](VideoIngestion/README.md)
2. [VideoAnalytics](VideoAnalytics/README.md)
3. [Visualizer](Visualizer/README.md)
4. [ImageStore](ImageStore/README.md)
5. [InfluxDBConnector](InfluxDBConnector/README.md)
6. [OpcuaExport](OpcuaExport/README.md) - Optional service to read from VideoAnalytics container to publish data to opcua    clients
7. [FactoryControlApp](FactoryControlApp/README.md) - Optional service to read from VideoAnalytics container if one wants
   to control the light based on defective/non-defective data
8. Telegraf
9. [Kapacitor](Kapacitor/README.md)
10. [EtcdUI](EtcdUI/README.md)
11. [DiscoveryCreek](DiscoveryCreek/README.md)
12. [WebVisualizer](WebVisualizer/README.md)
13. [Grafana](Grafana/README.md)
14. [Rest Data Export](RestDataExport/README.md)

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

