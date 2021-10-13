**Contents**

- [Introduction](#introduction)
- [Minimum System Requirements](#minimum-system-requirements)
- [EII Prerequisites installation](#eii-prerequisites-installation)
- [Generate deployment and configuration files](#generate-deployment-and-configuration-files)
  - [1. Generating consolidated docker-compose.yml and eii_config.json files:](#generating-consolidated-docker-composeyml-and-eii_configjson-files)
  - [2. Using builder script](#using-builder-script)
    - [2.1 Running builder to generate the above listed consolidated files for all applicable EII services](#running-builder-to-generate-the-above-listed-consolidated-files-for-all-applicable-eii-services)
    - [2.2 Running builder to generate the above listed consolidated files for a subset of EII services:](#running-builder-to-generate-the-above-listed-consolidated-files-for-a-subset-of-eii-services)
    - [2.3 Running builder to generate multi instance configs:](#running-builder-to-generate-multi-instance-configs)
    - [2.4 Running builder to generate benchmarking configs:](#running-builder-to-generate-benchmarking-configs)
  - [3. Adding new EII service so it gets picked up by Builder](#adding-new-eii-service-so-it-gets-picked-up-by-builder)
- [Distribution of EII container images](#distribution-of-eii-container-images)
- [Provision](#provision)
- [Build and Run EII video/timeseries use cases](#build-and-run-eii-videotimeseries-use-cases)
  - [Build EII stack](#build-eii-stack)
  - [Run EII services](#run-eii-services)
  - [Push required EII images to docker registry](#push-required-eii-images-to-docker-registry)
- [List of All EII Services](#list-of-all-eii-services)
  - [Common EII services](#common-eii-services)
  - [Video related services](#video-related-services)
  - [Timeseries related services](#timeseries-related-services)
- [Video pipeline Analytics](#video-pipeline-analytics)
  - [Enable camera based Video Ingestion](#enable-camera-based-video-ingestion)
  - [Using video accelerators in ingestion/analytics containers](#using-video-accelerators-in-ingestionanalytics-containers)
    - [**To run on USB devices**](#to-run-on-usb-devices)
    - [**To run on MYRIAD devices**](#to-run-on-myriad-devices)
    - [**To run on HDDL devices**](#to-run-on-hddl-devices)
    - [**To run on Intel(R) Processor Graphics (GPU/iGPU)**](#to-run-on-intelr-processor-graphics-gpuigpu)
  - [Custom Udfs](#custom-udfs)
- [Time-series Analytics](#time-series-analytics)
- [EII multi node cluster provision and deployment](#eii-multi-node-cluster-provision-and-deployment)
  - [**Without orchestrator**](#without-orchestrator)
  - [**With k8s orchestrator**](#with-k8s-orchestrator)
- [EII tools](#eii-tools)
- [EII Uninstaller](#eii-uninstaller)
- [Debugging options](#debugging-options)
- [Troubleshooting guide](#troubleshooting-guide)

# Introduction

Edge Insights for Industrial from Intel is a set of pre-validated ingredients for integrating video and time-series data analytics on edge compute nodes.
Edge Insights for Industrial helps to address various industrial and manufacturing usages, which include data collection, storage, and analytics on a variety of hardware nodes across the factory floor.

# Minimum System Requirements

EII software will run on the below mentioned Intel platforms:

```
* 6th generation Intel® CoreTM processor onwards OR
  6th generation Intel® Xeon® processor onwards OR
  Pentium® processor N4200/5, N3350/5, N3450/5 with Intel® HD Graphics
* At least 16GB RAM
* At least 64GB hard drive
* An internet connection
* Ubuntu 18.04 / Ubuntu 20.04
```

For performing Video Analytics, a 16GB of RAM is recommended.
For time-series ingestion and analytics, a 2GB RAM is sufficient.
The EII is validated on Ubuntu 18.04 and though it can run on other platforms supporting docker, it is not recommended.

# EII Prerequisites installation

> **Note**
>   * Recommended `Docker CLI & Daemon version` is `20.10.6`
>   * Recommended `docker-compose version` is `1.29.0`

The script automates the installation & configuration of all the prerequisite software needed to be installed on a system with a freshly installed operating system to make the system ready for provisioning, building and running EII stack. These pre-requisite softwares include:
1. **docker daemon**
2. **docker client**
3. **docker-compose**
4. **Python packages**

The script checks if docker & docker-compose are already installed in the system. If not installed, then it installs the intended version of the docker & docker-compose which is mentioned in the script.
If docker and/or docker-compose are already installed in the system but the installed version of the docker/docker-compose is older than the intended docker/docker-compose version (which is mentioned in the script) then the script uninstalls the older version of docker/docker-compose & re-installs the intended version mentioned in the script. The script also configures the proxy settings for docker client and docker daemon to connect to internet.

The [build/pre_requisites.sh](build/pre_requisites.sh) script also does proxy setting configuration for both system wide proxy & docker proxy. The script prompts for the proxy address to be entered by the user, if the system is behind a proxy.
The script also configures proxy setting system wide `/etc/environment` and for docker by taking the proxy value as user input if the system is running behind proxy. The proxy settings for `/etc/apt/apt.conf` is also set by this script to enable apt updates & installations.

----
**Note**:
* It is recommended to install the required `docker-compose version`
  which is `1.29.0`. Older version of docker-compose binary less than 1.29.0 may not work with the video-usecase docker-compose.yml files. It is observed that `device_cgroup_rules` command is not supported in certain docker-compose binaries versions less than `1.29.0`.
* If one still needs to use the older versions of docker-compose then
  `device_cgroup_rules` command needs to be commented in `ia_video_ingestion` and `ia_video_analytics` service which could result in limited inference and device support.

    ```yaml
        ia_video_ingestion:
          ...
          #device_cgroup_rules:
            #- 'c 189:* rmw'
            #- 'c 209:* rmw'
    ```
    After making changes to the docker-compose.yml file refer `Using builder script` section to re-run `builder.py` script before running the services using `docker-compose up`
----

1. **Steps to run the pre-requisites script is as follows**:

```sh
$ cd [WORKDIR]/IEdgeInsights/build
$ sudo -E ./pre_requisites.sh --help

Usage :: sudo -E ./pre_requisites.sh [OPTION...]

List of available options...

--proxy         proxies, required when the gateway/edge node running EII (or any of EII profile) is connected behind proxy

--help / -h         display this help and exit

Note : If --proxy option is not provided then script will run without proxy

Different use cases...

                1. RUNS WITHOUT PROXY
                $sudo -E ./pre_requisites.sh


                2.RUNS WITH PROXY
                $sudo -E ./pre_requisites.sh --proxy="proxy.intel.com:891"

```


2. **Optional:** For enabling full security for production deployments, make sure host machine and docker daemon are configured with below security recommendations. [build/docker_security_recommendation.md](build/docker_security_recommendation.md)

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

# Generate deployment and configuration files

The section assumes the EII software is already downloaded from the release package or from git.
Run all the below commands in this section from `[WORKDIR]/IEdgeInsights/build/` directory.

## Generating consolidated deployment and configuration files:

> **NOTE**
> 1. Whenever we make changes to individual EII app/service directories files as mentioned above in the description column
     or in the [build/.env](build/.env) file, it is required to re-run the `builder.py` script before provisioning and running
     the EII stack to ensure that the changes done reflect in the required consolidated files.
> 2. Manual editing of above consolidated files is not recommended and we would recommend to do the required changes to
     respective files in EII app/service directories and use [build/builder.py](build/builder.py) script to generate the conslidated ones.
> 3. This [build/builder.py](build/builder.py) script removes the subscriber or client interface for EII service/app(s) configuration in consolidated [eii_config.json](build/provision/config/eii_config.json)
     if the corresponding publisher or server interface in any EII service/app(s) is missing based on the default `builder.py` execution or `builder.py` execution with `-f` switch.
     **Please note if one runs into issues where the DNS server is being overwhelmed with DNS queries for the EII service/app name, most likely this would be happening if those EII services are intentionally
     stopped, please ensure to restart those EII services/app(s). If the intention is to not have that application/service running, please execute the `builder.py` by providing usecase yml file with that application/service not     
     listed, so the corresponding subscriber/client interfaces from other EII service/app(s) are automatically removed before provisioning and deployment of EII stack. Please check    
     [#running-builder-to-generate-the-above-listed-consolidated-files-for-a-subset-of-eii-services](#running-builder-to-generate-the-above-listed-consolidated-files-for-a-subset-of-eii-services) for selectively choosing required
     EII services**

EII is equipped with [builder](build/builder.py), a robust python tool to auto-generate the required configuration files to deploy EII services on single/multiple nodes. The tool is    capable of auto-generating the following consolidated files by fetching the respective files from EII service directories which are required to bring up different EII use-cases:

| file name                    | Description   |
| ---------------------------- | ------------- |
| docker-compose.yml           | Consolidated `docker-compose.yml` file used to launch EII docker containers in a given single node using `docker-compose` tool                                       |
| docker-compose.override.yml  | Consolidated `docker-compose-dev.override.yml` of every app that is generated only in DEV mode for EII deployment on a given single node using `docker-compose` tool |
| docker-compose-build.yml     | Consolidated `docker-compose-build.yml` file having EII base images and `depends_on` and `build` keys required for building EII services                             |
| docker-compose-push.yml      | Consolidated `docker-compose-push.yml` file (same as `docker-compose.yml` file with just dummy `build` key added), used for pushing EII services to docker registry  |
| eii_config.json              | Consolidated `config.json` of every app which will be put into etcd during provisioning                                                                              |
| values.yaml                  | Consolidated `values.yaml` of every app inside helm-eii/eii-deploy directory, which is required to deploy EII services via helm                                      |
| Template yaml files          | Files copied from helm/templates directory of every app to helm-eii/eii-deploy/templates directory, which are required to deploy EII services via helm               |

## Using builder script

Builder script usage:

```sh
$ python3 builder.py -h
usage: builder.py [-h] [-f YML_FILE] [-v VIDEO_PIPELINE_INSTANCES]
                    [-d OVERRIDE_DIRECTORY]

optional arguments:
    -h, --help            show this help message and exit
    -f YML_FILE, --yml_file YML_FILE
                        Optional config file for list of services to include.
                        Eg: python3 builder.py -f video-streaming.yml
                        (default: None)
    -v VIDEO_PIPELINE_INSTANCES, --video_pipeline_instances VIDEO_PIPELINE_INSTANCES
                        Optional number of video pipeline instances to be
                        created. Eg: python3 builder.py -v 6 (default:
                        1)
    -d OVERRIDE_DIRECTORY, --override_directory OVERRIDE_DIRECTORY
                        Optional directory consisting of of benchmarking
                        configs to be present in each app directory. Eg:
                        python3 builder.py -d benchmarking (default:
                        None)
```


### Running builder to generate the above listed consolidated files for all applicable EII services

  Builder will parse the top level directories under **IEdgeInsights** to generate the above listed consolidated files.

  ```sh
  $ python3 builder.py
  ```

### Running builder to generate the above listed consolidated files for a subset of EII services:

  This is achieved by providing a yml file to Builder as config which has list of services to include. User can mention the service name as path relative to **IEdgeInsights** or Full path to the service in the config yml file.

  If user wants to include only a certain number of services in the EII stack, he can opt to provide the **-f or yml_file** flag of builder.py to allow only the services provided in the yml file mentioned with the **-f or yml_file**. Few examples of such yml files for different usecases are provided at [video](build/usecases/video-streaming.yml), [time-series](build/usecases/time-series.yml), [Azure](build/usecases/video-streaming-azure.yml) etc.,

  An example for running Builder with this flag is given below:

  ```sh
  $ python3 builder.py -f usecases/video-streaming.yml
  ```
----
**NOTE**:
* **Main usecases**:
  | Usecase                    | yaml file                                                |
  | :---                       | :---                                                    |
  | Video + Timeseries         | [build/usecases/video-timeseries.yml](build/usecases/video-timeseries.yml) |
  | Video                      | [build/usecases/video.yml](build/usecases/video.yml)                       |
  | Timeseries                 | [build/usecases/time-series.yml](build/usecases/time-series.yml)           |
* **Video pipeline sub-usecases**:

  | Usecase                                | yaml file                                                               |
  | :---                                   | :---                                                                    |
  | Video streaming                        | [build/usecases/video-streaming.yml](build/usecases/video-streaming.yml)                  |
  | Video streaming and historical         | [build/usecases/video-streaming-storage.yml](build/usecases/video-streaming-storage.yml)  |
  | Video streaming with AzureBridge       | [build/usecases/video-streaming-azure.yml](build/usecases/video-streaming-azure.yml)      |
  | Video streaming and custom udfs        | [build/usecases/video-streaming-all-udfs.yml](build/usecases/video-streaming-all-udfs.yml)|

----

### Running builder to generate multi instance configs:

  Based on the user's requirements, builder can also generate multi-instance docker-compose.yml, config.json respectively.

  If user wants to generate boiler plate config for multiple stream use cases, he can do so by using the **-v or video_pipeline_instances** flag of builder. This flag creates multi stream boiler plate config for docker-compose.yml, eii_config.json files respectively.

  An example for running builder to generate multi instance boiler plate config for 3 streams of **video-streaming** use case has been provided below:

  ```sh
  $ python3 builder.py -v 3 -f usecases/video-streaming.yml
  ```

  > **NOTE**
  > This multi-instance feature support of Builder works only for the video pipeline i.e., **usecases/video-streaming.yml** use case alone and not with any other use case yml files like **usecases/video-streaming-storage.yml** etc., Also, it doesn't work for cases without `-f` switch too. In other words, only the above example works with `-v` taking in any +ve number

### Running builder to generate benchmarking configs:

  If user wants to provide a different set of docker-compose.yml, config.json other than the ones present in every service directory, he can opt to provide the **-d or override_directory** flag which indicates to search for these required set of files within a directory provided by the flag. For example, if user wants to pick up these files from a directory named **benchmarking**, he can run the command provided below:

  ```sh
  $ python3 builder.py -d benchmarking
  ```

  > **Note**
  > * If using the override directory feature of builder, it is recommended to include set of all 3 files mentioned above. Failing to provide any of the files in the override directory results in builder not including that service in the generated final config.
  > * If user wishes to spawn a single Subscriber/Client container subscribing/receiving on multiple Publisher/Server containers, he can do so by adding the AppName of Subscriber/Client container in **subscriber_list** of [builder_config.json](build/builder_config.json) ensuring the Publisher/Server container **AppName** is added in the **publisher_list** of [builder_config.json](build/builder_config.json). For services not mentioned in **subscriber_list**, multiple containers specified by the **-v** flag are spawned.
  For eg: If builder is run with **-v 3** option and **Visualizer** isn't added in **subscriber_list** of [builder_config.json](build/builder_config.json), 3 **Visualizer** instances are spawned, each of them subscribing to 3 **VideoAnalytics** services. If **Visualizer** is added in **subscriber_list** of [builder_config.json](build/builder_config.json), a single **Visualizer** instance subscribing to 3 multiple **VideoAnalytics** is spawned.

## Adding new EII service so it gets picked up by Builder

> **NOTE**
> Please refer EII sample apps at [Samples](https://github.com/open-edge-insights/eii-samples/blob/master/README.md) written in C++, python and golang using EII Core libraries for more details on adding new EII services

Since the builder takes care of registering and running any service present in it's own directory in the [IEdgeInsights](./) directory, this section describes on how to add any new service into the EII stack, subscribe to [VideoAnalytics](https://github.com/open-edge-insights/video-analytics) and publish on a new port.

Any service that needs to be added into the EII stack should be added as a new directory in the [IEdgeInsights](./) directory. The directory should contain a **docker-compose.yml** which will be used to deploy the service as a docker container and it should also contain a **config.json** which contains the required config for the service to run once it is deployed. The **config.json** will mainly consist of a **config** section which includes the configuration related parameters required to run the application and an **interfaces** section which includes the configuration of how this service interacts with other services of the EII stack. The **AppName** present in **environment** section in **docker-compose.yml** file is appended to the **config** & **interfaces** like **/AppName/config** & **/AppName/interfaces** before being put into the main `build/provision/config/eii_config.json`.

An example has been provided below on how to write the **config.json** for any new service, subscribe to **VideoAnalytics** and publish on a new port:

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

In the above specified **config.json**, the value of **config** key is the config required by the service to run and the value of the **interfaces** key is the config required by the service to interact with other services of EII stack over EII message bus.

The **Subscribers** value in the **interfaces** section denotes that this service should act as a subscriber to the stream being published by the value specified by **PublisherAppName** on the
endpoint mentioned in value specified by **EndPoint** on topics specified in value of **Topics** key.

The **Publishers** value in the **interfaces** section denotes that this service publishes a stream of data after obtaining and processing it from **VideoAnalytics**. The stream is published on
the endpoint mentioned in value of **EndPoint** key on topics mentioned in the value of **Topics** key. The services mentioned in the value of **AllowedClients** are the only clients able to
subscribe to the published stream if being published securely over the EIIMessageBus.

Similar to above interface keys, EII services can also have "Servers" and "Clients" interface keys too. For example, check [config.json](https://github.com/open-edge-insights/video-ingestion/blob/master/config.json) of VideoIngestion service and [config.json](https://github.com/open-edge-insights/eii-tools/blob/master/SWTriggerUtility/config.json) of SWTriggerUtility tool on how to use.

More details on the `interfaces` key responsible for the EII MessageBus endpoint configuration
can be found at [common/libs/ConfigMgr/README.md#interfaces](common/libs/ConfigMgr/README.md#interfaces). For more details on Etcd secrets configuration, visit [Etcd_Secrets_Configuration](./Etcd_Secrets_Configuration.md)

# Distribution of EII container images

EII services are available as pre-built container images in docker hub at https://hub.docker.com/u/openedgeinsights

Below are the list of pre-built container images that are accessible at https://hub.docker.com/u/openedgeinsights:

1. **Provisioning images**

  * openedgeinsights/ia_etcd_provision
  * openedgeinsights/ia_etcd

2. **Common EII images applicable for video and timeseries use cases**

  * openedgeinsights/ia_etcd_ui
  * openedgeinsights/ia_influxdbconnector
  * openedgeinsights/ia_rest_export
  * openedgeinsights/ia_opcua_export

3. **Video pipeline images**

  * openedgeinsights/ia_video_ingestion
  * openedgeinsights/ia_video_analytics
  * openedgeinsights/ia_web_visualizer
  * openedgeinsights/ia_visualizer
  * openedgeinsights/ia_imagestore
  * openedgeinsights/ia_azure_bridge
  * openedgeinsights/ia_azure_simple_subscriber

4. **Timeseries pipeline images**

  * openedgeinsights/ia_grafana

Additionally, we have `openedgeinsights/ia_edgeinsights_src` image available at the above docker hub
location which consists of source code of GPL/LGPL/AGPL components of EII stack.

For the EII docker images not listed on docker hub at above location, one needs to do the build from source 
before running `docker-compose up -d` command or bringing up the pod in kubernetes cluster on the build/development
node.

Eg:
```sh
$ # Update the DOCKER_REGISTRY value in [WORKDIR]/IEdgeInsights/build/.env as DOCKER_RESISTRY=<docker_registry> (Make sure `docker login <docker_registry>` to the docker reigstry works)
$ cd [WORKDIR]/IEdgeInsights/build
$ # Base images that needs to be built
$ docker-compose -f docker-compose-build.yml build ia_eiibase
$ docker-compose -f docker-compose-build.yml build ia_common
$ # Assuming here that the `python3 builder.py` step is been executed and ia_kapacitor
$ # service exists in the generated compose files and also, provisioning step is done
$ docker-compose -f docker-compose-build.yml build ia_kapacitor
$ docker-compose up -d
$ # Push all the applicable EII images to <docker_registry>. Ensure to use the same DOCKER_REGISTRY value on the deployment machine while deployment
$ docker-compose -f docker-compose-push.yml push
```


# Provision

<b>By default EII is provisioned in Secure mode</b>.

Follow below steps to provision. Provisioning must be done before deploying EII on any node. It will start ETCD as a container and load it with configuration required to run EII for single node or multi node cluster set up.

Please follow below steps to provision in Developer mode. Developer mode will have all security disabled.

* Please update DEV_MODE=true in [build/.env](build/.env) to provision in Developer mode.
* Please re-run the [build/builder.py](build/builder.py) to re-generate the consolidated files as mentioned above

Following actions will be performed as part of Provisioning

 * Loading initial ETCD values from json file located at `build/provision/config/eii_config.json`.
 * For Secure mode only, Generating ZMQ secret/public keys for each app and putting them in ETCD.
 * Generating required X509 certs and putting them in etcd.
 * All server certificates will be generated with 127.0.0.1, localhost and HOST_IP mentioned in [build/.env](build/.env).
 * If HOST_IP is blank in [build/.env](build/.env), then HOST_IP will be automatically detected when server certificates are generated.

**Optional:** In case of any errors during provisioning w.r.t existing volumes, please remove the existing volumes by running the EII uninstaller script: [eii_uninstaller.sh](build/eii_uninstaller.sh)

Run all the below commands in this section from `[WORKDIR]/IEdgeInsights/build/provision` directory.

Below script starts `etcd` as a container and provision. Please pass docker-compose file as argument, against which provisioning will be done.
```sh
$ cd [WORKDIR]/IEdgeInsights/build/provision
$ sudo -E ./provision.sh <path_to_eii_docker_compose_file>

$ # eq. $ sudo -E ./provision.sh ../docker-compose.yml

```
>**Optional**: By default, the provisioning step will pull the provisioning images from docker registry. In case the user wants to build the images, `--build` flag should be provided in the provisioning command i.e.
```sh
$ sudo -E ./provision.sh <path_to_eii_docker_compose_file> --build
$ # eq. $ sudo -E ./provision.sh ../docker-compose.yml --build
```
>**Note**
> The above command only `build` the provisioining images, will not do any other functions of provisioning script.

**Optional:** For capturing the data back from Etcd to a JSON file, run the [etcd_capture.sh](build/provision/etcd_capture.sh) script. This can be achieved using the following command:

```sh
$ ./etcd_capture.sh
```

# Build and Run EII video/timeseries use cases

  ---
  > **Note**
  > * For running EII services in IPC mode, make sure that the same user should be there in publisher and subscriber.
     If publisher is running as root (eg: VI, VA), then the subscriber also need to run as root.
     In `build/docker-compose.yml` if `user: ${EII_UID}` is in publisher service, then the
     same `user: ${EII_UID}` has to be in subscriber service. If the publisher doesn't have the user specified like above,
     then the subscriber service should not have that too.
  > * If services needs to be running in multiple nodes in TCP mode of communication, msgbus subscribers and clients of `AppName` are required to configure the "EndPoint" in config.json with HOST_IP and PORT under "Subscribers/Publishers" or "Clients/Servers" interfaces section.
  > * Make sure the PORT is being exposed in `build/docker-compose.yml` of the respective `AppName`
    Eg: If the `"EndPoint": <HOST_IP>:65012` is configured in `config.json`, then expose the port 65012 in `build/docker-compose.yml` of the service `ia_video_ingestion`

    ```yaml
      ia_video_ingestion:
        ...
        ports:
          - 65012:65012
    ```

  ---

All the below EII build and run commands needs to be executed from the `[WORKDIR]/IEdgeInsights/build/` directory.

Below are the usecases supported by EII to bring up the respective services mentioned in the
yaml file.

## Build EII stack

> **NOTE**
>
> 1. This step is optional if one wants to use the EII pre-built
>    container images itself and doesn't want to build from source.
>    For more details, refer: [Distribution of EII container images](#distribution-of-eii-container-images)
> 2. Base EII services like ia_eiibase, ia_video_common etc., are required only at the build time and not at
>    the runtime.

Builds all EII services in the `build/docker-compose-build.yml` along with the base EII services.

```sh
$ docker-compose -f docker-compose-build.yml build
```

If any of the services fails during build, it can be built using below command

```sh
$ docker-compose -f docker-compose-build.yml build --no-cache <service name>
```

## Run EII services

> **NOTE**
>
> If the images tagged with EII_VERSION as in [build/.env](build/.env) does not exist locally on the system,
> they would be pulled if those images exist int he docker hub during `docker-compose up`.

Runs all the EII services in the `build/docker-compose.yml`

```sh
$ xhost +
$ docker-compose up -d
```

A successful run will open Visualizer UI with results of video analytics for all video usecases.

## Push required EII images to docker registry

Pushes all the EII service docker images in the `build/docker-compose-push.yml`. Ensure to update the DOCKER_REGISTRY value in [.env](build/.env) file.
```sh
$ docker-compose -f docker-compose-push.yml push
```

# List of All EII Services

EII stack comes with following services, which can be included/excluded in docker-compose file based on requirements.

## Common EII services

1. [EtcdUI](EtcdUI/README.md)
2. [InfluxDBConnector](https://github.com/open-edge-insights/eii-influxdb-connector/blob/master/README.md)
3. [OpcuaExport](https://github.com/open-edge-insights/eii-opcua-export/blob/master/README.md) - Optional service to read from VideoAnalytics container to publish data to opcua clients
4. [RestDataExport](https://github.com/open-edge-insights/eii-rest-data-export/blob/master/README.md) - Optional service to read the metadata and image blob from InfluxDBConnector and ImageStore services respectively

## Video related services

1. [VideoIngestion](https://github.com/open-edge-insights/video-ingestion/blob/master/README.md)
2. [VideoAnalytics](https://github.com/open-edge-insights/video-analytics/blob/master/README.md)
3. [Visualizer](https://github.com/open-edge-insights/video-native-visualizer/blob/master/README.md)
4. [WebVisualizer](https://github.com/open-edge-insights/video-web-visualizer/blob/master/README.md)
5. [ImageStore](https://github.com/open-edge-insights/video-imagestore/blob/master/README.md)
6. [AzureBridge](https://github.com/open-edge-insights/eii-azure-bridge/blob/master/README.md)
7. [FactoryControlApp](https://github.com/open-edge-insights/eii-factoryctrl/blob/master/README.md) - Optional service to read from VideoAnalytics container if one wants to control the light based on defective/non-defective data

## Timeseries related services

1. [Telegraf](https://github.com/open-edge-insights/ts-telegraf/blob/master/README.md)
2. [Kapacitor](https://github.com/open-edge-insights/ts-kapacitor/blob/master/README.md)
3. [Grafana](https://github.com/open-edge-insights/ts-grafana/blob/master/README.md)
4. [ZMQ Broker](https://github.com/open-edge-insights/eii-zmq-broker/blob/master/README.md)

# Video pipeline Analytics
## Enable camera based Video Ingestion

For detailed description on configuring different types of cameras and  filter algorithms, refer to the [VideoIngestion/README.md](https://github.com/open-edge-insights/video-ingestion/blob/master/README.md).

## Using video accelerators in ingestion/analytics containers

EII supports running inference on `CPU`, `GPU`, `MYRIAD`(NCS2), and `HDDL` devices by accepting `device` value ("CPU"|"GPU"|"MYRIAD"|"HDDL"), part of the `udf` object configuration in `udfs`
key. The `device` field in UDF config of `udfs` key in `VideoIngestion` and `VideoAnalytics` configs can either be changed in the `build/provision/config/eii_config.json`
before provisioning (or re-provision it again after the change to the apps config.json, re-running `builder.py` script and then re-running the provisioning script) or at run-time via EtcdUI. For more details on the udfs config,
check [common/udfs/README.md](https://github.com/open-edge-insights/video-common/blob/master/udfs/README.md).

> **Note**
> There is an initial delay of upto ~30s while running inference on `GPU` (only for the first frame) as dynamically certain packages get created during runtime.

### **To run on USB devices**

For actual deployment in case USB camera is required then mount the device node of the USB camera for `ia_video_ingestion` service. When multiple USB cameras are connected to host m/c the required camera should be identified with the device node and mounted.

Eg: Mount the two USB cameras connected to the host m/c with device node as `video0` and `video1`

```yaml
  ia_video_ingestion:
    ...
    devices:
      - "/dev/dri"
      - "/dev/video0:/dev/video0"
      - "/dev/video1:/dev/video1"
```

> **Note**
> /dev/dri is needed for Graphic drivers

### **To run on MYRIAD devices**

  * To run inference on `MYRIAD` device `root` user permissions needs to be used at runtime. To enable root user at runtime in either `ia_video_ingestion`, `ia_video_analytics` or any of the custom udf services add `user: root` command in the respective docker-compose.yml file.

    Eg: To use `MYRAID` device in `ia_video_analytics` service refer the below example

    ```yaml
     ia_video_analytics:
       ...
       user: root
    ```

  * **Troubleshooting issues for MYRIAD(NCS2) devices**

    * Following is an workaround can be excercised if in case user observes `NC_ERROR` during device initialization of NCS2 stick.
      While running EII if NCS2 devices failed to initialize properly then user can re-plug the device for the init to happen freshly.
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

    * The below link can be referred in case user observes `global mutex initialization failed` during device initialization of NCS2 stick
      https://www.intel.com/content/www/us/en/support/articles/000033390/boards-and-kits.html

### **To run on HDDL devices**

  * Download the full package for OpenVINO toolkit for Linux version "2021.4"
    (`OPENVINO_IMAGE_VERSION` used in [build/.env](build/.env)) from the official website
    (https://software.intel.com/en-us/openvino-toolkit/choose-download/free-download-linux).
    Please refer to the OpenVINO links below for to install and running the HDDL daemon on host.

    1. OpenVINO install:
      https://docs.openvinotoolkit.org/2021.4/_docs_install_guides_installing_openvino_linux.html#install-openvino
    2. HDDL daemon setup:
      https://docs.openvinotoolkit.org/2021.4/_docs_install_guides_installing_openvino_linux_ivad_vpu.html

      **NOTE**:
      ----
      * OpenVINO 2021.4 installation creates a symbolic link to latest installation with
        filename as `openvino_2021` instead of `openvino`. Hence one can create a symbolic link with filename as `openvino` to the latest installation using the below steps.

        ```sh
        $ cd /opt/intel
        $ sudo ln -s <OpenVINO latest installation> openvino
        ```
        Eg: sudo ln -s openvino_2021.4.582 openvino

        In case there are older versions of OpenVINO installed on the host system please un-install them.

    3. Running hddldaemon:
      Refer the below command to run the hddldaemon once the setup is done(it should run in a different terminal or in the background on the host system where inference would be done)
      ```sh
      $ source /opt/intel/openvino/bin/setupvars.sh
      $ $HDDL_INSTALL_DIR/bin/hddldaemon
       ```
    4. Changing ownership of hddl files on host system.
      Before running inference with EII, ownership of hddl files should be changed to the value of `EII_USER_NAME` key set in [build/.env](build/.env).
      Refer the below command to set the ownership of hddl files on the host system.`EII_USER_NAME=eiiuser` in [build/.env](build/.env).
      ```
      $ sudo chown -R eiiuser /var/tmp/hddl_*
      ```

    * For actual deployment one could choose to mount only the required devices for
      services using OpenVINO with HDDL (`ia_video_analytics` or `ia_video_ingestion`) in `build/docker-compose.yml`.

      Eg: Mount only the Graphics and HDDL ion device for `ia_video_anaytics` service

      ```
        ia_video_analytics:
          ...
          devices:
                  - "/dev/dri"
                  - "/dev/ion:/dev/ion"
      ```
    * **Troubleshooting issues for HDDL devices**

      * Please verify the hddldaemon started on host m/c to verify if it is using the libraries of the correct OpenVINO version used in [build/.env](build/.env). One could enable the `device_snapshot_mode` to `full` in $HDDL_INSTALL_DIR/config/hddl_service.config on host m/c to get the complete snapshot of the hddl device.

      * Please refer OpenVINO 2021.4 release notes in the below link for new features and changes from the previous versions.
        https://software.intel.com/content/www/us/en/develop/articles/openvino-relnotes.html

      * Refer OpenVINO website in the below link to skim through known issues, limitations and troubleshooting
        https://docs.openvinotoolkit.org/2021.4/index.html
### **To run on Intel(R) Processor Graphics (GPU/iGPU)**

  > **Note**
  > The below step is required only for 11th gen Intel Processors

  Upgrade the kernel version to 5.8 and install the required drivers from the below OpenVINO link:
  https://docs.openvinotoolkit.org/latest/openvino_docs_install_guides_installing_openvino_linux.html#additional-GPU-steps

## Custom Udfs

The following are the two Custom Udfs workflow which EII supports:

1. Build / Run custom udfs as standalone applications

   For running custom udfs as standalone application one must download the video-custom-udfs repo and refer [CustomUdfs/README.md](https://github.com/open-edge-insights/video-custom-udfs/blob/master/README.md)

2. Build / Run custom udfs in VI or VA

   For running custom udfs either in VI or VA one must refer [VideoIngestion/docs/custom_udfs_doc.md](https://github.com/open-edge-insights/video-ingestion/blob/master/docs/custom_udfs_doc.md)

# Time-series Analytics

> **NOTE**:
> By default, `ia_telegraf` uses the mqtt plugin to talk to the `ia_mqtt_broker` service, so please make sure `ia_mqtt_broker` service is running and you are publishing the sample sensor data by
> following the guide at https://github.com/open-edge-insights/eii-tools/blob/master/mqtt/README.md. If one does not want to run MQTT broker, please make sure to comment/remove the `[[inputs.mqtt_consumer]]` sections to avoid DNS queries on `ia_mqtt_broker` in the [respective conf files](https://github.com/open-edge-insights/ts-telegraf/tree/master/config/Telegraf) based on DEV or PROD mode.

For time-series data, a sample analytics flow uses Telegraf for ingestion, Influx DB for storage and Kapacitor for classification. This is demonstrated with an MQTT based ingestion of sample temperature sensor data and analytics with a Kapacitor UDF which does threshold detection on the input values.

The services mentioned in [build/usecases/time-series.yml](build/usecases/time-series.yml) will be available in the consolidated `build/docker-compose.yml` and consolidated `build/provision/config/eii_config.json` of the EII stack for timeseries use case when built via `builder.py` as called out in previous steps.

This will enable building of Telegraf and the Kapacitor based analytics containers.
More details on enabling this mode can be referred from [Kapacitor/README.md](https://github.com/open-edge-insights/ts-kapacitor/blob/master/README.md)

The sample temperature sensor can be simulated using the [tools/mqtt/README.md](https://github.com/open-edge-insights/eii-tools/blob/master/mqtt-publisher/README.md) application.

# EII multi node cluster provision and deployment

## **Without orchestrator**

By default EII is provisioned with single node cluster.

One of the below options could be tried out:
* [`Recommended`] For deploying through ansible playbook on multiple nodes automatically, please refer [build/ansible/README.md](build/ansible/README.md#updating-the-eii-source-folder-usecase--proxy-settings-in-group-variables)
* In order to deploy EII on multiple nodes using docker registry and provision ETCD, please follow [build/deploy/README.md](build/deploy/README.md).

## **With k8s orchestrator**

One of the below options could be tried out:
* [`Recommended`] For deploying through ansible playbook on multiple nodes automatically, please refer [build/ansible/README.md](build/ansible/README.md##deploying-eii-using-helm-in-kubernetes-k8s-environment)
* Please refer [build/helm-eii/README.md](build/helm-eii/README.md) on using helm charts to provision the node and
  deploy EII services

# EII tools

EII stack has below set of tools which run as containers too:
* Benchmarking
  * [Video Benchmarking](https://github.com/open-edge-insights/eii-tools/blob/master/Benchmarking/video-benchmarking-tool/README.md)
  * [Time-series Benchmarking](https://github.com/open-edge-insights/eii-tools/blob/master/Benchmarking/time-series-benchmarking-tool/README.md)
* [DiscoverHistory](https://github.com/open-edge-insights/eii-tools/blob/master/DiscoverHistory/README.md)
* [EmbPublisher](https://github.com/open-edge-insights/eii-tools/blob/master/EmbPublisher/README.md)
* [EmbSubscriber](https://github.com/open-edge-insights/eii-tools/blob/master/EmbSubscriber/README.md)
* [GigEConfig](https://github.com/open-edge-insights/eii-tools/blob/master/GigEConfig/README.md)
* [HttpTestServer](https://github.com/open-edge-insights/eii-tools/blob/master/HttpTestServer/README.md)
* [JupyterNotebook](https://github.com/open-edge-insights/eii-tools/blob/master/JupyterNotebook/README.md)
* [mqtt](https://github.com/open-edge-insights/eii-tools/blob/master/mqtt-publisher/README.md)
* [SWTriggerUtility](https://github.com/open-edge-insights/eii-tools/blob/master/SWTriggerUtility/README.md)
* [TimeSeriesProfiler](https://github.com/open-edge-insights/eii-tools/blob/master/TimeSeriesProfiler/README.md)
* [VideoProfiler](https://github.com/open-edge-insights/eii-tools/blob/master/VideoProfiler/README.md)

# EII Uninstaller

The uninstaller script automates the removal of all the EII Docker configuration installed on a system. This uninstaller will perform the following tasks:

1. **Stops and removes all EII running and stopped containers**
2. **Removes all EII docker volumes**
3. **Removes all EII docker images \[Optional\]**
4. **Removes all EII install directory**

Run the below commmand from `[WORKDIR]/IEdgeInsights/build/` directory.

```sh
$ ./eii_uninstaller.sh -h
Usage: ./eii_uninstaller.sh [-h] [-d]

 This script uninstalls previous EII version.
 Where:
    -h show the help
    -d triggers the deletion of docker images (by default it will not trigger)

 Example:
  1) Deleting only EII Containers and Volumes
    $ ./eii_uninstaller.sh

  2) Deleting EII Containers, Volumes and Images
    $ export EII_VERSION=2.4
    $ ./eii_uninstaller.sh -d
    above example will delete EII containers, volumes and all the docker images having 2.4 version.

```
# Debugging options

1. To check if all the EII images are built successfully, use cmd: `docker images|grep ia` and
   to check if all containers are running, use cmd: `docker ps` (`one should see all the dependency containers and EII containers up and running`). If you see issues where the build is failing due to non-reachability to Internet, please ensure you have correctly configured proxy settings and restarted docker service. Even after doing this, if you are running into the same issue, please add below instrcutions to all the dockerfiles in `build\dockerfiles` at the top after the LABEL instruction and retry the building EII images:

    ```sh
    ENV http_proxy http://proxy.iind.intel.com:911
    ENV https_proxy http://proxy.iind.intel.com:911
    ```

2. `docker ps` should list all the enabled containers which are included in docker-compose.yml

3. To verify if the default video pipeline with EII is working fine i.e., from video ingestion->video analytics->visualizer, please check the visualizer UI

4. `/opt/intel/eii` root directory gets created - This is the installation path for EII:
     * `data/` - stores the backup data for persistent imagestore and influxdb
     * `sockets/` - stores the IPC ZMQ socket files

---
**Note**:
1. Few useful docker-compose and docker commands:
     * `docker-compose -f docker-compose-build.yml build` - builds all the service containers. To build a single service container, use `docker-compose -f docker-compose-build.yml build [serv_cont_name]`
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

# Troubleshooting guide

1. Please refer to [TROUBLESHOOT.md](./TROUBLESHOOT.md) guide for any troubleshooting tips related to EII configuration and installation
2. If any issues are observed w.r.t the python package installation then manually install the python packages as shown below :

   **Note**: It is highly recommended that you use a python virtual environment to install the python packages, so that the system python installation doesn't get
   altered. Details on setting up and using python virtual environment can be found here: https://www.geeksforgeeks.org/python-virtual-environment/

    ```sh
    $ cd [WORKDIR]/IEdgeInsights/build
    # Install requirements for builder.py
    $ pip3 install -r requirements.txt
    ```
