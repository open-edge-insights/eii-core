# Contents

- [Contents](#contents)
  - [About Intel® Edge Insights for Industrial](#about-intel-edge-insights-for-industrial)
  - [Minimum system requirements](#minimum-system-requirements)
  - [Install Intel® Edge Insights for Industrial from GitHub](#install-intel-edge-insights-for-industrial-from-github)
  - [Task 1: Get EII codebase from GitHub](#task-1-get-eii-codebase-from-github)
  - [Task 2: Install prerequisites](#task-2-install-prerequisites)
    - [Run the pre-requisite script](#run-the-pre-requisite-script)
    - [Optional steps](#optional-steps)
    - [Method 1](#method-1)
    - [Method 2](#method-2)
  - [Task 3: Generate deployment and configuration files](#task-3-generate-deployment-and-configuration-files)
    - [Use the builder script](#use-the-builder-script)
    - [Generate consolidated files for all applicable EII services](#generate-consolidated-files-for-all-applicable-eii-services)
    - [Generate consolidated files for a subset of EII services](#generate-consolidated-files-for-a-subset-of-eii-services)
    - [Generate multi-instance configs using builder](#generate-multi-instance-configs-using-builder)
    - [Generate benchmarking configs using builder](#generate-benchmarking-configs-using-builder)
    - [Add EII services](#add-eii-services)
    - [Distribute the EII container images](#distribute-the-eii-container-images)
    - [List of EII services](#list-of-eii-services)
  - [Task 4: Build and run the EII video and timeseries use cases](#task-4-build-and-run-the-eii-video-and-timeseries-use-cases)
    - [Build EII stack](#build-eii-stack)
    - [Run EII services](#run-eii-services)
      - [EII Provisioning](#eii-provisioning)
        - [Start EII in Dev mode](#start-eii-in-dev-mode)
        - [Start EII in Profiling mode](#start-eii-in-profiling-mode)
    - [Push the required EII images to docker registry](#push-the-required-eii-images-to-docker-registry)
  - [Video pipeline analytics](#video-pipeline-analytics)
    - [Enable camera-based video ingestion](#enable-camera-based-video-ingestion)
    - [Use video accelerators in ingestion and analytics containers](#use-video-accelerators-in-ingestion-and-analytics-containers)
      - [To run on USB devices](#to-run-on-usb-devices)
      - [To run on MYRIAD devices](#to-run-on-myriad-devices)
        - [Troubleshooting issues for MYRIAD(NCS2) devices](#troubleshooting-issues-for-myriadncs2-devices)
      - [To run on HDDL devices](#to-run-on-hddl-devices)
        - [Troubleshooting issues for HDDL devices](#troubleshooting-issues-for-hddl-devices)
      - [To run on Intel(R) Processor Graphics (GPU/iGPU)](#to-run-on-intelr-processor-graphics-gpuigpu)
    - [Custom User Defined Functions](#custom-user-defined-functions)
  - [Time-series analytics](#time-series-analytics)
  - [Debugging options](#debugging-options)
  - [EII multi node cluster provision and deployment](#eii-multi-node-cluster-provision-and-deployment)
    - [**With k8s orchestrator**](#with-k8s-orchestrator)
  - [EII tools](#eii-tools)
  - [EII uninstaller](#eii-uninstaller)
  - [Troubleshooting guide](#troubleshooting-guide)

## About Intel® Edge Insights for Industrial

Intel® Edge Insights for Industrial (EII) is a set of pre-validated ingredients for integrating video and time-series data analytics on edge compute nodes. EII includes modules to enable data collection, storage, and analytics for both time-series and video data.

## Minimum system requirements

The following are the minimum system requirements to run EII:

|System Requirement       |  Details     |
|---    |---    |
|Processor       | Any of the following processor: <ul><li>6th generation Intel® CoreTM processor onwards</li><li>6th generation Intel® Xeon® processor onwards</li><li>Pentium® processor N4200/5, N3350/5, N3450/5 with Intel® HD Graphics</li></ul>      |
|RAM       |16 GB       |
|Hard drive       | 64 GB      |
|Operating system       | Ubuntu 18.04 or Ubuntu 20.04      |

> **Note**
>
> - You should be connected to the internet to use EII.
> - The recommended RAM capacity for video analytics pipeline is 16 GB and time-series analytics pipeline is 2 GB.
> - EII is validated on Ubuntu 18.04 and Ubuntu 20.04 but you can install EII stack on other Linux distributions with support for docker-ce and docker-compose tools.

## Install Intel® Edge Insights for Industrial from GitHub

You can download and install EII from the [Open Edge Insights GitHub repository](https://github.com/open-edge-insights/). For more information on manifest files, refer [Readme](https://github.com/open-edge-insights/eii-manifests/blob/master/README.md).

To install EII, perform the tasks in the following order:

- [Task 1: Get EII codebase from GitHub](#task-1--get-eii-codebase-from-github)
- [Task 2: Install prerequisites](#task-2--install-prerequisites)
- [Task 3: Generate deployment and configuration files](#task-3--generate-deployment-and-configuration-files)
- [Task 4: Build and run the EII video and timeseries use cases](#task-4-build-and-run-the-eii-video-and-timeseries-use-cases)

## Task 1: Get EII codebase from GitHub

To get the EII codebase complete the following steps:

1. Run the following commands to install the repo tool.

    ```sh
    curl https://storage.googleapis.com/git-repo-downloads/repo > repo
    sudo mv repo /bin/repo
    sudo chmod a+x /bin/repo
    ```

2. Run the following command to create a working directory

    ```sh
    mkdir -p <work-dir>
    ```

3. Run the following commands to initialize the working directory using the repo tool.

    ```sh
    cd <work-dir>
    repo init -u "https://github.com/open-edge-insights/eii-manifests.git"
    ```

4. Run the following command to pull all the projects mentioned in the manifest xml file with the default or specific revision mentioned for each project.

    ```sh
    repo sync
    ```

## Task 2: Install prerequisites

The `pre_requisites.sh` script automates the installation and configuration of all the prerequisites required for building and running the EII stack. The prerequisites are as follows:

- docker daemon
- docker client
- docker-compose
- Python packages

The `pre-requisites.sh` file performs the following:

- Checks if docker and docker-compose is installed in the system. If required, it uninstalls the older version and installs the correct version of docker and docker-compose.
- Configures the proxy settings for the docker client and docker daemon to connect to the internet.
- Configures the proxy settings system-wide (/etc/environment) and for docker. If a system is running behind a proxy, then the script prompts users to enter the proxy address to configure the proxy settings.
- Configures proxy setting for /etc/apt/apt.conf to enable apt updates and installations.

> **Note**
>
> - The recommended version of the docker-compose is `1.29.0`. In versions older than 1.29.0, the video use case docker-compose.yml files and the device_cgroup_rules command may not work.
> - To use versions older than docker-compose 1.29.0, in the `ia_video_ingestion` and `ia_video_analytics` services, comment out the `device_cgroup_rules` command.
> - You can comment out the `device_cgroup_rules` command in the `ia_video_ingestion` and `ia_video_analytics` services to use versions older than 1.29.0 of docker-compose. This can result in limited inference and device support. The following code sample shows how the `device_cgroup_rules` commands are commented out:
>
>  ```sh
>    ia_video_ingestion:
>     ...
>      #device_cgroup_rules:
>         #- 'c 189:* rmw'
>         #- 'c 209:* rmw'
>  ```
>
> After modifying the `docker-compose.yml` file, refer to the `Using builder script` section. Before running the services using the `docker-compose up` command, rerun the `builder.py` script.

### Run the pre-requisite script

To run the pre-requisite script, execute the following commands:

  ```sh
    cd [WORKDIR]/IEdgeInsights/build
    sudo -E ./pre_requisites.sh --help

      Usage :: sudo -E ./pre_requisites.sh [OPTION...]
      List of available options...
      --proxy         proxies, required when the gateway/edge node running EII (or any of EII profile) is connected behind proxy
      --help / -h         display this help and exit

  ```

> **Note**
> If the --proxy option is not provided, then script will run without proxy. Different use cases are as follows
>
> - Runs without proxy
>
>     ```sh
>      sudo -E ./pre_requisites.sh
>     ```
>
> - Runs with proxy
>
>     ```sh
>     sudo -E ./pre_requisites.sh --proxy="proxy.intel.com:891"
>     ```

### Optional steps

- If required, you can enable full security for production deployments. Ensure that the host machine and docker daemon are configured per the security recommendation. For more info, see [build/docker_security_recommendation.md](https://github.com/open-edge-insights/eii-core/blob/master/build/docker_security_recommendation.md).

- If required, you can enable log rotation for docker containers using any of the following methods:

### Method 1

Set the logging driver as part of the docker daemon. This applies to all the docker containers by default.

1. Configure the json-file driver as the default logging driver. For more info, see [JSON File logging driver](https://docs.docker.com/config/containers/logging/json-file/). The sample json-driver configuration which can be copied to `/etc/docker/daemon.json` is as follows:

  ```json
          {
            "log-driver": "json-file",
            "log-opts": {
            "max-size": "10m",
            "max-file": "5"
            }
        }

  ```

2. Run the following command to reload the docker daemon:

    ```sh

    sudo systemctl daemon-reload

    ```

3. Run the following command to restart docker:

    ```sh

    sudo systemctl restart docker

    ```

### Method 2

Set logging driver as part of docker compose which is container specific. This overwrites the 1st option (i.e /etc/docker/daemon.json). The following example shows how to enable logging driver only for the video_ingestion service:

  ```json
    ia_video_ingestion:
      ...
      ...
      logging:
        driver: json-file
        options:
        max-size: 10m
  max-file: 5
  ```

## Task 3: Generate deployment and configuration files

After downloading EII from the release package or Git, run the commands mentioned in this section from the `[WORKDIR]/IEdgeInsights/build/` directory.

### Use the builder script

Run the following command to use the builder script:

```sh

    python3 builder.py -h
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

### Generate consolidated files for all applicable EII services

Using the builder tool, EII auto-generates configuration files that are required for deploying the EII services on single or multiple nodes. The builder tool auto-generates the consolidated files by getting the relevant files from the EII service directories that are required for different EII use-cases. The builder tool parses the top level directories under the `IEdgeInsights` directory to generate the consolidated files.

The following table shows the list of consolidated files and their details:

Table: Consolidated files

| file name                    | Description                                                                                                                                                          |
| ---------------------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| docker-compose.yml           | Consolidated `docker-compose.yml` file used to launch EII docker containers in a given single node using `docker-compose` tool                                       |
| docker-compose.override.yml  | Consolidated `docker-compose-dev.override.yml` of every app that is generated only in DEV mode for EII deployment on a given single node using `docker-compose` tool |
| docker-compose-build.yml     | Consolidated `docker-compose-build.yml` file having EII base images and `depends_on` and `build` keys required for building EII services                             |
| docker-compose-push.yml      | Consolidated `docker-compose-push.yml` file (same as `docker-compose.yml` file with just dummy `build` key added), used for pushing EII services to docker registry  |
| eii_config.json              | Consolidated `config.json` of every app which will be put into etcd during provisioning                                                                              |
| values.yaml                  | Consolidated `values.yaml` of every app inside helm-eii/eii-deploy directory, which is required to deploy EII services via helm                                      |
| Template yaml files          | Files copied from helm/templates directory of every app to helm-eii/eii-deploy/templates directory, which are required to deploy EII services via helm               |

> **Note**
>
> - If you modify an individual EII app, service directories files, or the `build/.env` file then run the `builder.py` script before running the EII stack. The changes are saved in the consolidated files.
> - Manual editing is not recommended for modifying the consolidated files.Instead make changes to the respective files in the EII app or services directories and use the `build` or `builder.py` script to generate the consolidated files.

To generate the consolidated files, run the following command:

  ```sh
  python3 builder.py
  ```

> **Note**
>
> - The [builder_config.json](./build/builder_config.json) contains keys `subscriber_list`, `publisher_list`, `include_services` and `increment_rtsp_port`.
> - The `subscriber_list` contains a list of services that act as a subscriber to the stream being published.
> - The `publisher_list` contains a list of services that publishes a stream of data.
> - The `include_services` contains the mandatory list of services that are required to be included when builder is run without the -f flag.
> - The `exclude_services` contains the mandatory list of services that are required to be excluded when builder is run without the -f flag.
> - The `increment_rtsp_port` is a boolean key used for incrementing the port number for RTSP stream pipelines.

### Generate consolidated files for a subset of EII services

Builder uses a yml file for configuration. The config yml file consists of a list of services to include. You can mention the service name as the path relative to `IEdgeInsights` or full path to the service in the config yml file.

To include only a certain number of services in the EII stack, you can add the -f or yml_file flag of builder.py. You can find the examples of yml files for different use cases as follows:

- [Azure](https://github.com/open-edge-insights/eii-core/blob/master/build/usecases/video-streaming-azure.yml)

  The following example shows running Builder with the -f flag :

    ```sh
       python3 builder.py -f usecases/video-streaming.yml

    ```



- **Main use cases**

| Usecase                    | yaml file                                                                 |
| -------------------------- |---------------------------------------------------------------------------|
| Video + Timeseries         | [build/usecases/video-timeseries.yml](build/usecases/video-timeseries.yml)|
| Video                      | [build/usecases/video.yml](build/usecases/video.yml)                      |
| Timeseries                 | [build/usecases/time-series.yml](build/usecases/time-series.yml)          |

- **Video pipeline sub use cases**

| Usecase                                | yaml file                                                                                 |
| -------------------------------------- | ----------------------------------------------------------------------------------------- |
| Video streaming                        | [build/usecases/video-streaming.yml](build/usecases/video-streaming.yml)                  |
| Video streaming and historical         | [build/usecases/video-streaming-storage.yml](build/usecases/video-streaming-storage.yml)  |
| Video streaming with AzureBridge       | [build/usecases/video-streaming-azure.yml](build/usecases/video-streaming-azure.yml)      |
| Video streaming and custom udfs        | [build/usecases/video-streaming-all-udfs.yml](build/usecases/video-streaming-all-udfs.yml)|

When you run the multi-instance config, a `build/multi_instance` directory is created in the build directory. Based on the number of `video_pipeline_instances` specified, that many directories of VideoIngestion and VideoAnalytics is created in the `build/multi_instance` directory.

The next section provides an example for running builder to generate multi-instance boiler plate config for 3 streams of **video-streaming** use case.

### Generate multi-instance configs using builder

If required, you can use builder to generate multi-instance docker-compose.yml and config.json. Using the -v or video_pipeline_instances flag of builder you can generate boiler plate config for multiple stream use cases. The -v or video_pipeline_instances flag creates the multi-stream boiler plate config for docker-compose.yml and eii_config.json files.

The following example shows running builder to generate the multi-instance boiler plate config for 3 streams of video-streaming use case:

  ```sh
    python3 builder.py -v 3 -f usecases/video-streaming.yml

  ```

Using the previous command for 3 instances, the `build/multi_instance` directory consists of VideoIngestion1, VideoIngestion2, VideoIngestion3 and VideoAnalytics1, VideoAnalytics2 , VideoAnalytics3 directories. Each of these directories initially will have the default `config.json` and the `docker-compose.yml` files that are present within the `VideoIngestion` and the `VideoAnalytics` directories.

  ```example
    ./build/multi_instance/
    |-- VideoAnalytics1
    |   |-- config.json
    |   `-- docker-compose.yml
    |-- VideoAnalytics2
    |   |-- config.json
    |   `-- docker-compose.yml
    |-- VideoAnalytics3
    |   |-- config.json
    |   `-- docker-compose.yml
    |-- VideoIngestion1
    |   |-- config.json
    |   `-- docker-compose.yml
    |-- VideoIngestion2
    |   |-- config.json
    |   `-- docker-compose.yml
    `-- VideoIngestion3
        |-- config.json
        `-- docker-compose.yml
  ```

 You can edit the configs of each of these streams within the `build/multi_instance` directory. To generate the consolidated `docker compose` and `eii_config.json` file, rerun the `builder.py` command.

  > **Note**
  >
  > - The multi-instance feature support of Builder works only for the video pipeline i.e., **usecases/video-streaming.yml** use case alone and not with any other use case yml files like **usecases/video-streaming-storage.yml** and so on. Also, it doesn't work for cases without the `-f` switch. The previous example will work with any positive number for `-v`. To learn more about using the multi-instance feature with the DiscoverHistory tool, see [Multi-instance feature support for the builder script with the DiscoverHistory tool](https://github.com/open-edge-insights/eii-tools/blob/master/DiscoverHistory/README.md#multi-instance-feature-support-for-the-builder-script-with-the-discoverhistory-tool).
  > - If you are running the multi-instance config for the first time, it is recommended to not change the default config.json file and docker-compose.yml file in the VideoIngestion and VideoAnalytics directories.
  > - If you are not running the multi-instance config for the first time, the existing config.json and docker-compose.yml files in the `build/multi_instance` directory will be used to generate the consolidated eii-config.json and docker-compose files.
  > - The docker-compose.yml files present withn the `build/multi_instance` directory will have the updated service_name, container_name, hostname, AppName, ports and secrets for that respective instance.
  > - The config.json file in the `build/multi_instance` directory will have the updated Name, Type, Topics, Endpoint, PublisherAppname, ServerAppName and AllowedClients for the interfaces section and incremented rtsp port number for the config section of that respective instance.
  > - Ensure that all the containers are down before running the multi-instance configuration. Run `docker-compose down` before running `builder.py` for multi-instance configuration.

### Generate benchmarking configs using builder

Use the `-d` or the `override_directory` flag to provide a different set of `docker-compose.yml` and `config.json` files other than the existing files in every service directory. The `-d` or the `override_directory` flag indicates to search for the required set of files within a directory provided by the flag.

For example, to pick files from a directory named benchmarking, you can run the following command:

  ```sh
     python3 builder.py -d benchmarking
  ```

> **Note**
>
> - If you use the override directory feature of the builder then include all the 3 files mentioned in the previous example. If you do not include a file in the override directory then the builder will omit that service in the final config that is generated.
> - Adding the `AppName` of the subscriber or client container in the `subscriber_list of builder_config.json` allows you to spawn a single subscriber or client container that is subscribing or receiving on multiple publishers or server containers.
> - Multiple containers specified by the `-v` flag is spawned for services that are not mentioned in the `subscriber_list`. For example, if you run builder with `–v 3` option and `Visualizer` is not added in the `subscriber_list` of `builder_config.json` then 3 instances of Visualizer are spawned. Each instance subscribes to 3 VideoAnalytics services. If Visualizer is added in the `subscriber_list` of `builder_config.json`, a single Visualizer instance subscribing to 3 multiple VideoAnalytics is spawned.

### Add EII services

This section provides information about adding a new service, subscribing to the [VideoAnalytics](https://github.com/open-edge-insights/video-analytics), and publishing it on a new port.

Add a service to the EII stack as a new directory in the [IEdgeInsights](./) directory. The builder registers and runs any service present in its own directory in the [IEdgeInsights](./) directory. The directory should contain the following:

- A `docker-compose.yml` file to deploy the service as a docker container. The `AppName` is present in the `environment` section in the `docker-compose.yml` file. Before adding the `AppName` to the main `build/eii_config.json`, it is appended to the `config` and `interfaces` as `/AppName/config` and `/AppName/interfaces`.
- A `config.json` file that contains the required config for the service to run after it is deployed. The `config.json` mainly consists of the following:
  - A `config` section, which includes the configuration-related parameters that are required to run the application.
  - An `interfaces` section, which includes the configuration of how the service interacts with other services of the EII stack.

> **NOTE**
> For more information on adding new EII services, refer to the EII sample apps at [Samples](https://github.com/open-edge-insights/eii-samples/blob/master/README.md) written in C++, python, and Golang using the EII core libraries.

The following example shows how to write the **config.json** for any new service, subscribe to **VideoAnalytics**, and publish on a new port:

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

The `config.json` file consists of the following key and values:

- value of the `config` key is the config required by the service to run.
- value of the `interfaces` key is the config required by the service to interact with other services of EII stack over the EII message bus.
- the `Subscribers` value in the `interfaces` section denotes that this service should act as a subscriber to the stream being published by the value specified by `PublisherAppName` on the endpoint mentioned in value specified by `EndPoint` on topics specified in value of `Topic` key.
- the `Publishers` value in the `interfaces` section denotes that this service publishes a stream of data after obtaining and processing it from `VideoAnalytics`. The stream is published on the endpoint mentioned in value of `EndPoint` key on topics mentioned in the value of `Topics` key.
- the services mentioned in the value of `AllowedClients` are the only clients that can subscribe to the published stream, if it is published securely over the EIIMessageBus.

> **Note**
>
> - Similar to the interface keys, EII services can also have "Servers" and "Clients" interface keys. For more information, refer [config.json](https://github.com/open-edge-insights/video-ingestion/blob/master/config.json) of the `VideoIngestion` service and [config.json](https://github.com/open-edge-insights/eii-tools/blob/master/SWTriggerUtility/config.json) of SWTriggerUtility tool.
> - For more information on the `interfaces` key responsible for the EII MessageBus endpoint configuration, refer [common/libs/ConfigMgr/README.md#interfaces](common/libs/ConfigMgr/README.md#interfaces).
> For more details on the Etcd secrets configuration, refer [Etcd_Secrets_Configuration](./Etcd_Secrets_Configuration.md).

### Distribute the EII container images

The EII services are available as pre-built container images in the Docker Hub at https://hub.docker.com/u/openedgeinsights. To access the services that are not available in the Docker Hub, build from source before running the `docker-compose up -d` command.

For example:

```sh
# Update the DOCKER_REGISTRY value in [WORKDIR]/IEdgeInsights/build/.env as DOCKER_RESISTRY=<docker_registry> (Make sure `docker login <docker_registry>` to the docker reigstry works)
cd [WORKDIR]/IEdgeInsights/build
# Base images that needs to be built
docker-compose -f docker-compose-build.yml build ia_eiibase
docker-compose -f docker-compose-build.yml build ia_common
# Assuming here that the `python3 builder.py` step has been executed and ia_kapacitor
# Service exists in the generated compose files
docker-compose -f docker-compose-build.yml build ia_kapacitor
docker-compose up -d
# Push all the applicable EII images to <docker_registry>. Ensure to use the same DOCKER_REGISTRY value on the deployment machine while deployment
docker-compose -f docker-compose-push.yml push
```

The list of pre-built container images that are accessible at https://hub.docker.com/u/openedgeinsights is as follows:

- **Provisioning images**
  - openedgeinsights/ia_configmgr_agent
- **Common EII images applicable for video and timeseries use cases**
  - openedgeinsights/ia_etcd_ui
  - openedgeinsights/ia_influxdbconnector
  - openedgeinsights/ia_rest_export
  - openedgeinsights/ia_opcua_export
- **Video pipeline images**
  - openedgeinsights/ia_video_ingestion
  - openedgeinsights/ia_video_analytics
  - openedgeinsights/ia_web_visualizer
  - openedgeinsights/ia_visualizer
  - openedgeinsights/ia_imagestore
  - openedgeinsights/ia_azure_bridge
  - openedgeinsights/ia_azure_simple_subscriber
- **Timeseries pipeline images**
  - openedgeinsights/ia_grafana

> **Note**
> Additionally, we have `openedgeinsights/ia_edgeinsights_src` image available at https://hub.docker.com/u/openedgeinsights which consists of source code of the GPL/LGPL/AGPL components of the EII stack.

## List of EII services

Based on requirement, you can include or exclude the following EII services in the `docker-compose` file:

- Common EII services
  - [EtcdUI](https://github.com/open-edge-insights/eii-etcd-ui/blob/master/README.md)
  - [InfluxDBConnector](https://github.com/open-edge-insights/eii-influxdb-connector/blob/master/README.md)
  - [OpcuaExport](https://github.com/open-edge-insights/eii-opcua-export/blob/master/README.md) - Optional service to read from the VideoAnalytics container to publish data to opcua clients.
  - [RestDataExport](https://github.com/open-edge-insights/eii-rest-data-export/blob/master/README.md) - Optional service to read the metadata and image blob from the InfluxDBConnector and ImageStore services respectively.
- Video related services
  - [VideoIngestion](https://github.com/open-edge-insights/video-ingestion/blob/master/README.md)
  - [VideoAnalytics](https://github.com/open-edge-insights/video-analytics/blob/master/README.md)
  - [Visualizer](https://github.com/open-edge-insights/video-native-visualizer/blob/master/README.md)
  - [WebVisualizer](https://github.com/open-edge-insights/video-web-visualizer/blob/master/README.md)
  - [ImageStore](https://github.com/open-edge-insights/video-imagestore/blob/master/README.md)
  - [AzureBridge](https://github.com/open-edge-insights/eii-azure-bridge/blob/master/README.md)
  - [FactoryControlApp](https://github.com/open-edge-insights/eii-factoryctrl/blob/master/README.md) - Optional service to read from the VideoAnalytics container if one wants to control the light based on defective/non-defective data
- Timeseries-related services
  - [Telegraf](https://github.com/open-edge-insights/ts-telegraf/blob/master/README.md)
  - [Kapacitor](https://github.com/open-edge-insights/ts-kapacitor/blob/master/README.md)
  - [Grafana](https://github.com/open-edge-insights/ts-grafana/blob/master/README.md)
  - [ZMQ Broker](https://github.com/open-edge-insights/eii-zmq-broker/blob/master/README.md)

## Task 4: Build and run the EII video and timeseries use cases

  > **Note**
  >
  > - For running the EII services in the IPC mode, ensure that the same user is mentioned in the publisher and subscriber services.
  > - If the publisher service is running as root such as `VI`, `VA`, then the subscriber service should also run as root. For example, in the `docker-compose.yml`file, if you have specified `user: ${EII_UID}` in the publisher service, then specify the same `user: ${EII_UID}` in the subscriber service. If you have not specified a user in the publisher service then don't specify the user in the subscriber service.
  > - If services needs to be running in multiple nodes in the TCP mode of communication, msgbus subscribers, and clients of `AppName` are required to configure the "EndPoint" in `config.json` with the `HOST_IP` and the `PORT` under `Subscribers/Publishers` or `Clients/Servers` interfaces section.
  > - Ensure that the port is being exposed in the `docker-compose.yml` of the respective `AppName`.
  > For example, if the `"EndPoint": <HOST_IP>:65012` is configured in the `config.json` file, then expose the port `65012` in the `docker-compose.yml` file of the `ia_video_ingestion` service.

  ```yaml
    ia_video_ingestion:
      ...
      ports:
        - 65012:65012
  ```

Run all the following EII build and commands from the `[WORKDIR]/IEdgeInsights/build/` directory.

EII supports the following use cases to run the services mentioned in the `docker_compose.yml` file. Refer to Task 2 to generate the docker_compose.yml file based on a specific use case. For more information and configuration, refer to the `[WORK_DIR]/IEdgeInsights/README` file.

## Build EII stack

> **NOTE**
>
> - This is an optional step, if you want to use the EII pre-built container images and don't want to build from source. For more details, refer: [Distribution of EII container images](#distribution-of-eii-container-images)
> - Base EII services like `ia_eiibase`, `ia_video_common`, and so on, are required only at the build time and not at the runtime.

Run the following command to build all EII services in the `build/docker-compose-build.yml` along with the base EII services.

```sh
docker-compose -f docker-compose-build.yml build
```

If any of the services fails during the build then run the following command to build the service again:

```sh
docker-compose -f docker-compose-build.yml build --no-cache <service name>
```

## Run EII services

> **NOTE**
>
> If the images tagged with `EII_VERSION` label as in the [build/.env](build/.env) do not exist locally in the system but are available in the Docker Hub the images will be pulled during the `docker-compose up`command.

### EII Provisioning

The EII provisioning is taken care by the `ia_configmgr_agent` service which gets lauched as part of the EII stack.
For more details on the ConfigMgr Agent component, please refer: [https://gitlab.devtools.intel.com/Indu/edge-insights-industrial/eii-configmgr-agent/-/blob/feature/pumpkincreek/README.md](https://gitlab.devtools.intel.com/Indu/edge-insights-industrial/eii-configmgr-agent/-/blob/feature/pumpkincreek/README.md)

#### Start EII in Dev mode

> **Note**
> 1. By default, EII is provisioned in the secure mode.
>    It is recommended to not use EII in the Dev mode in a production environment because all 
>    the security feaures are disabled in the Dev mode.
> 2. By default, EII empty certificates folder ([WORKDIR]/IEdgeInsights/build/Certificates]) 
>    will be created in DEV mode. This behavior is because of docker bind mounts but it is not
>    an issue.

Starting EII in the Dev mode eases the development phase for System Integrators (SI). In the Dev mode, all components communicate over non-encrypted channels. To enable the Dev mode, set the environment variable `DEV_MODE` to `true` in the `[WORK_DIR]/IEdgeInsights/build/.env` file. The default value of this variable is `false`.

To provision EII in the developer mode, complete the following steps:

- Step 1. Update DEV_MODE=true in build/.env.
- Step 2. Re-run the build/builder.py to regenerate the consolidated files

#### Start EII in Profiling mode

The Profiling mode is used for collecting the performance statistics in EII. In this mode, each EII component makes a record of the time needed for processing any single frame. These statistics are collected in the visualizer where System Integrtors (SI) can see the end-to-end processing time and the end-to-end average time for individual frames.

To enable the Profiling mode, in the `[WORK_DIR]/IEdgeInsights/build/.env` file, set the environment variable `PROFILING` to `true`.

Use the following command to run all the EII services in the `docker-compose.yml` file

```sh
xhost +
docker-compose up -d ia_configmgr_agent # workaround for now, we are exploring on getting this and the sleep avoided
sleep 30 # If any failures like configmgr data store client certs or msgbus certs failures, please increase this time to a higher value
docker-compose up -d
```

If the run is successful then the Visualizer UI is displayed with results of video analytics for all video use cases.

## Push the required EII images to docker registry

> **NOTE**
> By default, if `DOCKER_REGISTRY` is empty in [build/.env](build/.env) then the images are published to hub.docker.com. Ensure to remove `openedgeinsights/` org from the image names while pushing to Docker Hub as repository or image names with multiple slashes are not supported. This limitation doesn't exist in other docker registries like Azure Container Registry(ACR), Harbor registry, and so on.

Run the following command to push all the EII service docker images in the `build/docker-compose-push.yml`. Ensure to update the DOCKER_REGISTRY value in [.env](build/.env) file.

```sh
docker-compose -f docker-compose-push.yml push
```

## Video pipeline analytics

This section provides more information about working with video pipeline.

### Enable camera-based video ingestion

For detailed description on configuring different types of cameras and filter algorithms, refer to the [VideoIngestion/README.md](https://github.com/open-edge-insights/video-ingestion/blob/master/README.md).

### Use video accelerators in ingestion and analytics containers

EII supports running inference on `CPU`, `GPU`, `MYRIAD (NCS2)`, and `HDDL` devices by accepting the `device` value ("CPU"|"GPU"|"MYRIAD"|"HDDL"),
part of the `udf` object configuration in the `udfs` key. The `device` field in the UDF config of `udfs` key in the `VideoIngestion` and `VideoAnalytics`
configs can be updated at runtime via [EtcdUI](https://github.com/open-edge-insights/eii-etcd-ui) interface, the `VideoIngestion` and `VideoAnalytics`
services will auto-restart.

For more details on the UDF config, refer [common/udfs/README.md](https://github.com/open-edge-insights/video-common/blob/master/udfs/README.md).

> **Note**
> There is an initial delay of upto ~30s while running inference on `GPU` (only for the first frame) as dynamically certain packages get created during runtime.

### To run on USB devices

For actual deployment in case USB camera is required then mount the device node of the USB camera for `ia_video_ingestion` service. When multiple USB cameras are connected to host m/c the required camera should be identified with the device node and mounted.

For example, mount the two USB cameras connected to the host machine with device node as `video0` and `video1`

```yaml
  ia_video_ingestion:
    ...
    devices:
      - "/dev/dri"
      - "/dev/video0:/dev/video0"
      - "/dev/video1:/dev/video1"
```

> **Note**
> /dev/dri is required for graphic drivers.

### To run on MYRIAD devices

To run inference on `MYRIAD` device `root` user permissions needs to be used at runtime.

To enable root user at runtime in `ia_video_ingestion` or any of the custom UDF services based on `ia_video_ingestion`, set `RUN_AS_USER` env variable to `root` in the respective `docker-compose.yml` file.

For example refer the below snip:


  ```yaml
  ia_video_ingestion:
    ...
    environment:
    ...
      # Set RUN_AS_USER env variable to root.
      RUN_AS_USER: "root"
      # RUN_AS_USER env variable can be used to run VideoIngestion service with the specified user privileges.
    ...
  ```

To enable root user at runtime in `ia_video_analytics` or any of the custom UDF services based on `ia_video_analytics`, set `user: root` in the respective `docker-compose.yml` file.

For example refer the below snip:

  ```yaml
    ia_video_analytics:
      ...
      user: root
   ```

> Note: In IPC mode when publisher(e.g. ia_video_ingestion or ia_video_analytics) is running as root then the subscriber(e.g. ia_visualizer) should also run as root.

#### Troubleshooting issues for MYRIAD(NCS2) devices

If the `NC_ERROR` occurs during device initialization of NCS2 stick then use the following workaround. Replug the device for the init, if the NCS2 devices fails to initialize during running EII. To check if initialization is successful, run ***dmesg*** and ***lsusb*** as follows:

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

- If you notice `global mutex initialization failed` during device initialization of NCS2 stick, then refer to the following link: <https://www.intel.com/content/www/us/en/support/articles/000033390/boards-and-kits.html>

- For VPU troubleshooting, refer the following link: <https://docs.openvinotoolkit.org/2021.4/openvino_docs_install_guides_installing_openvino_linux_ivad_vpu.html#troubleshooting>

### To run on HDDL devices

Complete the following steps to run inference on HDDL devices:

1. Download the full package for OpenVINO toolkit for Linux version "2021 4.2 LTS"
    (`OPENVINO_IMAGE_VERSION` used in [build/.env](build/.env)) from the official website
    (<https://software.intel.com/en-us/openvino-toolkit/choose-download/free-download-linux>).

2. Refer to the following links for OpenVINO to install and run the HDDL daemon on host.

- OpenVINO install: <https://docs.openvinotoolkit.org/2021.4/_docs_install_guides_installing_openvino_linux.html#install-openvino>
- HDDL daemon setup: <https://docs.openvinotoolkit.org/2021.4/_docs_install_guides_installing_openvino_linux_ivad_vpu.html>

> Note: OpenVINO 2021.4 installation creates a symbolic link to the latest installation with filename as `openvino_2021` instead of `openvino`. You can create a symbolic link with filename as `openvino` to the latest installation as follows:
   >
   > ```sh
   >  cd /opt/intel
   >  sudo ln -s <OpenVINO latest installation> openvino
   > ```
   >
   > Example: sudo ln -s openvino_2021.4.752 openvino
   >
   > Uninstall the older versions of OpenVINO, if it is installed on the host system.

  3. Run the following command to run the HDDL Daemon image after the setup is complete. The HDDL Daemon should run in a different terminal or in the background on the host system where inference performed.

      ```sh
      source /opt/intel/openvino/bin/setupvars.sh
      $HDDL_INSTALL_DIR/bin/hddldaemon
       ```

  4. Change the ownership of HDDL files on the host system.
      Before running inference with EII, change the ownership of the Hddl files to the value of `EII_USER_NAME` key set in [build/.env](build/.env).
      Refer the following command to set the ownership of hddl files on the host system.`EII_USER_NAME=eiiuser` in [build/.env](build/.env).

      ```sh
      sudo chown -R eiiuser /var/tmp/hddl_*
      ```

  - For actual deployment you can choose to mount only the required devices for services using OpenVINO with HDDL (`ia_video_analytics` or `ia_video_ingestion`) in `docker-compose.yml` file.

  For example, mount only the Graphics and HDDL ion device for the `ia_video_anaytics` service

      ```sh
        ia_video_analytics:
          ...
          devices:
                  - "/dev/dri"
                  - "/dev/ion:/dev/ion"
      ```

> Note: HDDL Plugin can have ION driver compatibility issues with certain Linux kernel version. Refer [OpenVINO-Release-Notes](https://www.intel.com/content/www/us/en/developer/articles/release-notes/openvino-relnotes.html) for verifying the supported kernel version. ION driver needs to be succesfully installed in order to run inference on HDDL device.

#### Troubleshooting issues for HDDL devices

- Check if the HDDL Daemon started on the host machine to verify if it is using the libraries of the correct OpenVINO version used in [build/.env](build/.env). Enable the `device_snapshot_mode` to `full` in $HDDL_INSTALL_DIR/config/hddl_service.config on the host machine to get the complete snapshot of the HDDL device.

- For VPU troubleshooting refer the following link:
<https://docs.openvinotoolkit.org/2021.4/openvino_docs_install_guides_installing_openvino_linux_ivad_vpu.html#troubleshooting>

- Refer OpenVINO 2021.4 release notes from the following link for new features and changes from the previous versions.
<https://software.intel.com/content/www/us/en/develop/articles/openvino-relnotes.html>

- Refer OpenVINO website in the below link to skim through known issues, limitations and troubleshooting
<https://docs.openvinotoolkit.org/2021.4/index.html>

### To run on Intel(R) Processor Graphics (GPU/iGPU)

To run inference on `GPU` device `root` user permissions needs to be used at runtime.

To enable root user at runtime in `ia_video_ingestion` or any of the custom UDF services based on `ia_video_ingestion`, set `RUN_AS_USER` env variable to `root` in the respective `docker-compose.yml` file.

For example refer the below snip:

  ```yaml
  ia_video_ingestion:
    ...
    environment:
    ...
      # Set RUN_AS_USER env variable to root.
      RUN_AS_USER: "root"
      # RUN_AS_USER env variable can be used to run VideoIngestion service with the specified user privileges.
    ...
  ```

To enable root user at runtime in `ia_video_analytics` or any of the custom UDF services based on `ia_video_analytics`, set `user: root` in the respective `docker-compose.yml` file.

For example refer the below snip:

  ```yaml
    ia_video_analytics:
      ...
      user: root
   ```

  > **Note**
  > In IPC mode when publisher(e.g. ia_video_ingestion or ia_video_analytics) is running as root then the subscriber(e.g. ia_visualizer) should also run as root.
  > The below step is required only for the 11th gen Intel Processors

  Upgrade the kernel version to 5.8 and install the required drivers from the following OpenVINO link:
  <https://docs.openvinotoolkit.org/latest/openvino_docs_install_guides_installing_openvino_linux.html#additional-GPU-steps>

## Custom User Defined Functions

EII supports the following custom User Defined Functions (UDFs):

1. Build or run custom UDFs as standalone applications

   For running custom udfs as standalone application one must download the video-custom-udfs repo and refer [CustomUdfs/README.md](https://github.com/open-edge-insights/video-custom-udfs/blob/master/README.md)

2. Build or run custom UDFs in VI or VA

   For running custom UDFs either in VI or VA, refer to [VideoIngestion/docs/custom_udfs_doc.md](https://github.com/open-edge-insights/video-ingestion/blob/master/docs/custom_udfs_doc.md)

## Time-series analytics

For time-series data, a sample analytics flow uses Telegraf for ingestion, Influx DB for storage and Kapacitor for classification. This is demonstrated with an MQTT based ingestion of sample temperature sensor data and analytics with a Kapacitor UDF which does threshold detection on the input values.

The services mentioned in [build/usecases/time-series.yml](build/usecases/time-series.yml) will be available in the consolidated `docker-compose.yml` and consolidated `build/eii_config.json` of the EII stack for timeseries use case when built via `builder.py` as called out in previous steps.

This will enable building of Telegraf and the Kapacitor based analytics containers.
More details on enabling this mode can be referred from [Kapacitor/README.md](https://github.com/open-edge-insights/ts-kapacitor/blob/master/README.md)

The sample temperature sensor can be simulated using the [tools/mqtt/README.md](https://github.com/open-edge-insights/eii-tools/blob/master/mqtt/README.md) application.

## EII multi node cluster deployment

### With k8s orchestrator

One of the below options could be tried out:

- [`Recommended`] For deploying through ansible playbook on multiple nodes automatically, please refer [build/ansible/README.md](build/ansible/README.md##deploying-eii-using-helm-in-kubernetes-k8s-environment)
- Please refer [build/helm-eii/README.md](build/helm-eii/README.md) on using helm charts to provision the node and deploy EII services

## EII tools

EII stack has below set of tools which run as containers too:

- Benchmarking
  - [Video Benchmarking](https://github.com/open-edge-insights/eii-tools/blob/master/Benchmarking/video-benchmarking-tool/README.md)
  - [Time-series Benchmarking](https://github.com/open-edge-insights/eii-tools/blob/master/Benchmarking/time-series-benchmarking-tool/README.md)
- [DiscoverHistory](https://github.com/open-edge-insights/eii-tools/blob/master/DiscoverHistory/README.md)
- [EmbPublisher](https://github.com/open-edge-insights/eii-tools/blob/master/EmbPublisher/README.md)
- [EmbSubscriber](https://github.com/open-edge-insights/eii-tools/blob/master/EmbSubscriber/README.md)
- [GigEConfig](https://github.com/open-edge-insights/eii-tools/blob/master/GigEConfig/README.md)
- [HttpTestServer](https://github.com/open-edge-insights/eii-tools/blob/master/HttpTestServer/README.md)
- [JupyterNotebook](https://github.com/open-edge-insights/eii-tools/blob/master/JupyterNotebook/README.md)
- [mqtt](https://github.com/open-edge-insights/eii-tools/blob/master/mqtt/README.md)
- [SWTriggerUtility](https://github.com/open-edge-insights/eii-tools/blob/master/SWTriggerUtility/README.md)
- [TimeSeriesProfiler](https://github.com/open-edge-insights/eii-tools/blob/master/TimeSeriesProfiler/README.md)
- [VideoProfiler](https://github.com/open-edge-insights/eii-tools/blob/master/VideoProfiler/README.md)

## EII Uninstaller

The uninstaller script automates the removal of all the EII Docker configuration installed on a system. This uninstaller will perform the following tasks:

1. **Stops and removes all EII running and stopped containers**
2. **Removes all EII docker volumes**
3. **Removes all EII docker images \[Optional\]**
4. **Removes all EII install directory**

Run the following commmand from the `[WORKDIR]/IEdgeInsights/build/` directory

```sh
./eii_uninstaller.sh -h
```

Usage: ./eii_uninstaller.sh [-h] [-d]

This script uninstalls previous EII version.
Where:
    -h show the help
    -d triggers the deletion of docker images (by default it will not trigger)

Example:

- Run the following command to delete the EII containers and volumes:

  ```sh
      ./eii_uninstaller.sh
  ```

- Run the following command to delete the EII containers, volumes, and images

  ```sh
    export EII_VERSION=2.4
    ./eii_uninstaller.sh -d
  ```

The commands in the example will delete version 2.4 EII containers, volumes, and all the docker images.

## Debugging options

1. To check if all the EII images are built successfully, use cmd: `docker images|grep ia` and
   to check if all containers are running, use cmd: `docker ps` (`one should see all the dependency containers and EII containers up and running`). If you see issues where the build is failing due to non-reachability to Internet, please ensure you have correctly configured proxy settings and restarted docker service.

2. `docker ps` should list all the enabled containers which are included in docker-compose.yml

3. To verify if the default video pipeline with EII is working fine i.e., from video ingestion->video analytics->visualizer, please check the visualizer UI

4. `/opt/intel/eii` root directory gets created - This is the installation path for EII:
     - `data/` - stores the backup data for persistent imagestore and influxdb
     - `sockets/` - stores the IPC ZMQ socket files

---

> Note
>
>- Few useful docker-compose and docker commands:
> - `docker-compose -f docker-compose-build.yml build` - builds all the service containers. To build a single service container, use `docker-compose -f docker-compose-build.yml build [serv_cont_name]`
> - `docker-compose down` - stops and removes the service containers
> - `docker-compose up -d` - brings up the service containers by picking the changes done in `docker-compose.yml`
> - `docker ps` - check running containers
> - `docker ps -a` - check running and stopped containers
> - `docker stop $(docker ps -a -q)` - stops all the containers
> - `docker rm $(docker ps -a -q)` - removes all the containers. Useful when you run into issue of already container is in use.
> - [docker compose cli](https://docs.docker.com/compose/reference/overview/)
> - [docker compose reference](https://docs.docker.com/compose/compose-file/)
> - [docker cli](https://docs.docker.com/engine/reference/commandline/cli/#configuration-files)
>
>- If you want to run the docker images separately i.e, one by one, run the command `docker-compose run --no-deps [service_cont_name]` Eg: `docker-compose run --name ia_video_ingestion --no-deps      ia_video_ingestion` to run VI container and the switch `--no-deps` will not bring up it's dependencies mentioned in the docker-compose file. If the container is not launching, there could be some issue with entrypoint program which could be overrided by providing this extra switch `--entrypoint /bin/bash` before the service container name in the docker-compose run command above, this would let one inside the container and run the actual entrypoint program from the container's terminal to rootcause the issue. If the container is running and one wants to get inside, use cmd: `docker-compose exec [service_cont_name] /bin/bash` or `docker exec -it [cont_name] /bin/bash`
>
>- Best way to check logs of containers is to use command: `docker logs -f [cont_name]`. If one wants to see all the docker-compose service container logs at once, then just run
   `docker-compose logs -f`

---

## Troubleshooting guide

1. Please refer to [TROUBLESHOOT.md](./TROUBLESHOOT.md) guide for any troubleshooting tips related to EII configuration and installation
2. If any issues are observed w.r.t the python package installation then manually install the python packages as shown below :

   **Note**: It is highly recommended that you use a python virtual environment to install the python packages, so that the system python installation doesn't get altered. Details on setting up and using python virtual environment can be found here: <https://www.geeksforgeeks.org/python-virtual-environment/>

    ```sh
    cd [WORKDIR]/IEdgeInsights/build
    # Install requirements for builder.py
    pip3 install -r requirements.txt
    ```
