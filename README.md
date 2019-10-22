Edge Insights Software (EIS) is the framework for enabling smart manufacturing with visual and point defect inspections.

# Contents:

1. [Minimum System Requirements](#minimum-system-requirements)

2. [Docker pre-requisites](#docker-pre-requisities)

3. [EIS Pre-requisites](#eis-pre-requisites)

4. [Provision EIS](#provision-eis)

5. [Build / Run EIS PCB Demo Example](#build-and-run-eis-pcb-demo-example)

6. [Etcd Secrets and MessageBus Endpoint Configuration](#etcd-secrets-and-msgbus-configuration)

7. [Enable camera based Video Ingestion](#enable-camera-based-video-ingestion)

8. [Using video accelerators](#using-video-accelerators)

9. [Time-series Analytics](#time-series-analytics)

10. [DiscoveryCreek](#DiscoveryCreek)

11. [EIS multi node cluster provision and deployment using Turtlecreek](#eis-multi-node-cluster-provision-and-deployment-using-turtlecreek)

12. [Debugging options](#debugging-options)



# Minimum System Requirements

EIS software will run on the below mentioned Intel platforms:

```
BDW (Broadwell)
SKL (Skylake)
BXT (Broxton) / APL (Apollo Lake)
KBL (Kaby Lake)
CFL (Coffee Lake)
WHL (Whiskey Lake)
CML (Comet Lake)
ICL (Ice Lake)
```

For performing Video Analytics, a 16GB of RAM is recommended.
For time-series ingestion and analytics, a 2GB RAM is sufficient.
The EIS is validated on Ubuntu 18.04 and though it can run on other platforms supporting docker, it is not recommended.


# Docker Pre-requisities

1. Install latest docker cli/docker daemon by following [https://docs.docker.com/install/linux/docker-ce/ubuntu/#install-docker-ce](https://docs.docker.com/install/linux/docker-ce/ubuntu/#install-docker-ce). Follow **Install using the repository** and **Install Docker CE (follow first 2 steps)** sections there. Also, follow the manage docker as a non-root user section at [https://docs.docker.com/install/linux/linux-postinstall/](https://docs.docker.com/install/linux/linux-postinstall/) to run docker without sudo

2. Please follow the below steps only if the node/system on which the docker setup is tried out is running behind a HTTP proxy server. If that's not the case, this step can be skipped.

    * Configure proxy settings for docker client to connect to internet and for containers to access internet by following [https://docs.docker.com/network/proxy/](https://docs.docker.com/network/proxy/). Sample proxy config which can be copied to ~/.docker/config.json is provided below.

        ```
        {
            "proxies":
                {
                    "default":
                    {
                        "httpProxy": "http://proxy.iind.intel.com:911",
                        "httpsProxy": "http://proxy.iind.intel.com:911",
                        "noProxy": "127.0.0.1,localhost,*.intel.com"
                    }
                }
        }
        ```

    * Configure proxy settings for docker daemon by following the steps at [https://docs.docker.com/config/daemon/systemd/#httphttps-proxy](https://docs.docker.com/config/daemon/systemd/#httphttps-proxy). Use the values for http proxy and https proxy as used in previous step.

    * The correct DNS servers need to be updated to the /etc/resolv.conf

        ```
        A. Ubuntu 16.04 and earlier

            For Ubuntu 16.04 and earlier, /etc/resolv.conf was dynamically generated by NetworkManager.

            Comment out the line dns=dnsmasq (with a #) in /etc/NetworkManager/NetworkManager.conf

            Restart the NetworkManager to regenerate /etc/resolv.conf :
            sudo systemctl restart network-manager

            Verify on the host: cat /etc/resolv.conf

        B. Ubuntu 18.04 and later

            Ubuntu 18.04 changed to use systemd-resolved to generate /etc/resolv.conf. Now by default it uses a local DNS cache 127.0.0.53. That will not work inside a container, so Docker will default to Google's 8.8.8.8 DNS server, which may break for people behind a firewall.

            /etc/resolv.conf is actually a symlink (ls -l /etc/resolv.conf) which points to /run/systemd/resolve/stub-resolv.conf (127.0.0.53) by default in Ubuntu 18.04.

            Just change the symlink to point to /run/systemd/resolve/resolv.conf, which lists the real DNS servers:
            sudo ln -sf /run/systemd/resolve/resolv.conf /etc/resolv.conf

            Verify on the host: cat /etc/resolv.conf
        ```

3. Install `docker-compose` tool by following this [https://docs.docker.com/compose/install/#install-compose](https://docs.docker.com/compose/install/#install-compose)

4. **Optional:** For enabling full security, make sure host machine and docker daemon are configured with below security recommendations. [docker_setup/docker_security_recommendation.md](docker_setup/docker_security_recommendation.md)

5. **Optional:** If one wishes to enable log rotation for docker containers

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

* Download the full package for OpenVINO toolkit for Linux version "2019 R3" from the official website
  (https://software.intel.com/en-us/openvino-toolkit/choose-download/free-download-linux), extract it and copy the
  directory `l_openvino_toolkit_xxxxx/` inside `[repo]/common/openvino`.

   > **NOTE**: Make sure there is always one `l_openvino_toolkit_xxxxx/` folder under [repo]/common/openvino folder as 
   > we are adding `l_openvino_toolkit_*` into Dockerfile which could result in  build failure of 
   > openvino base container if there are multiple openvino sdk's in there (especially the old ones)

# Provision EIS

<b>`By default EIS is provisioned in Secure mode`</b>.

Follow below steps to provision EIS. Porvisioning must be done before deploying EIS on any node. It will start ETCD as a container and load it with configuration required to run EIS for single node or multi node cluster set up.

Please follow below steps to provision EIS in Developer mode. Developer mode will have all security disabled.

* Please update DEV_MODE=true in [docker_setup/.env](docker_setup/.env) to provision EIS in Developer mode.
* <b>Please comment secrets section for all services in [docker_setup/docker-compose.yml](../docker-compose.yml)</b>

Following actions will be performed as part of Provisioning

 * Loading inital ETCD values from json file located at [docker_setup/provision/config/etcd_pre_load.json](docker_setup/provision/config/etcd_pre_load.json).
 * For Secure mode, Generating ZMQ secret/public keys for each app and putting them in ETCD.
 * Generating required X509 certs and putting them in etcd.

Below script starts `etcd` as a container and provision EIS. Please pass docker-compose file as argument, against which provisioning will be done.
```
$ sudo ./provision_eis.sh <path_to_eis_docker_compose_file>

eq. $ sudo ./provision_eis.sh ../docker-compose.yml

```


# Build and Run EIS PCB Demo Example

 Please incldue following services in [docker-compose.yml](docker_setup/docker-compose.yml) for PCB Demo example

* For streaming use case only:

    > ia_video_ingestion, ia_video_analytics, ia_visualizer

* For historical use case only:

    > ia_video_ingestion, ia_video_analytics, ia_imagestore, ia_influxdbconnector

  > **NOTE**: Use [DiscoverHistory](tools/DiscoverHistory/README.md) to view the historical data from ia_imagestore
  > and ia_influxdbconnector.

* For streaming and historical use case

    > ia_video_ingestion, ia_video_analytics, ia_visualizer, ia_imagestore, ia_influxdbconnector

> **Note:**
> * All EIS build and run commands are to be executed from the [repo]/docker_setup/ directory.
> * If `ia_visualizer` service is enabled in the [docker-compose.yml](docker_setup/docker-compose.yml) file, please
>   run command `$ xhost +` in the terminal before starting EIS stack, this is a one time configuration. This is 
>   needed by `ia_visualizer` service to render the UI

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

   Please note that the first time build of EIS containers take ~70 minutes.

   A successful run will open Visualizer UI with results of video analytics.

# Etcd Secrets and MessageBus Endpoint Configuration

Etcd Secrets and MessageBus endpoint configurations are done to establish the data path
and configuration of various EIS containers.

Every service in [docker_setup/docker-compose.yml](docker_setup/docker-compose.yml)
is a
* messagebus client if it needs to send or receive data over EISMessageBus
* etcd client if it needs to get data from etcd distributed key store

For more details, visit [Etcd_Secrets_and_MsgBus_Endpoint_Configuration](./Etcd_Secrets_and_MsgBus_Endpoint_Configuration.md)

# Enable camera based Video Ingestion

EIS supports various cameras like Basler (GiGE), RTSP and USB camera. The video ingestion pipeline is enabled using 'gstreamer' which ingests the frames from the camera. The Video Ingestion application accepts a user-defined filter algorithm to do pre-processing on the frames before it is ingested into the DBs and inturn to the Analytics container.

All the changes related to camera type are made in the Etcd ingestor configuration values and sample ingestor configurations are provided in [VideoIngestion/README.md](VideoIngestion/README.md) for reference.

For detailed description on configuring different types of cameras and  filter algorithms, refer to the [VideoIngestion/README.md](VideoIngestion/README.md).

# Using video accelerators

EIS supports running inference on `CPU`, `GPU`, `Myriad` and `HDDL` devices by accepting device type (“CPU”|”GPU”|”MYRIAD”|”HDDL”) which is part of the classifier configuration in Etcd. For more details, check [VideoAnalytics/README.md#classifier-config](VideoAnalytics/README.md#classifier-config)

> Note:
> To run on HDDL devices, make sure to uncomment the below section of code in [VideoAnalytics/va_classifier_start.sh](VideoAnalytics/va_classifier_start.sh).

    ```sh
    #Uncomment these lines if you are using HDDL
    #$HDDL_INSTALL_DIR/bin/hddldaemon &
    #sleep 20
    ```

# Time-series Analytics

For time-series data, a sample analytics flow uses Telegraf for ingestion, Influx DB for storage and Kapacitor for classification. This is demonstrated with an MQTT based ingestion of sample temperature sensor data and analytics with a Kapacitor UDF which does threshold detection on the input values.

For enabling this, different set of containers need to be built in EIS and it can be selected by modifying the docker-compose file.

Please incldue following services in [docker-compose.yml](docker_setup/docker-compose.yml) for Time series analytics example.

> **Note:**
> * If `ia_visualizer` service is enabled in the [docker-compose.yml](docker_setup/docker-compose.yml) file, please
>   run command `$ xhost +` in the terminal before starting EIS stack, this is a one time configuration. This is 
>   needed by `ia_visualizer` service to render the UI

> ia_telegraf, ia_influxdbconnector, ia_data_analytics, ia_visualizer


This will enable building of Telegraf and the Kapacitor based analytics containers.
More details on enabling this mode can be referred from [TimeSeriesAnalytics/README.md](TimeSeriesAnalytics/README.md)

The sample temperature sensor can be simulated using the [tools/mqtt-temp-sensor](tools/mqtt-temp-sensor) application.

# DiscoveryCreek

DiscoveryCreek is a machine learning based anomaly detection engine.

For enabling DiscoveryCreek, please include the following services in the [docker-compose.yml](docker_setup/docker-compose.yml) file:

> ia_telegraf, ia_influxdbconnector, ia_dc, ia_visualizer

More details on enabling DiscoveryCreek based analytics can be referred at [DiscoveryCreek/README.md](DiscoveryCreek/README.md)

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
8. [Telegraf](Telegraf/README.md)
9. [TimeSeriesAnalytics](TimeSeriesAnalytics/README.md)
10. [EtcdUI](EtcdUI/README.md)
11. [DiscoveryCreek](DiscoveryCreek/README.md)


# EIS multi node cluster provision and deployment using Turtlecreek

By default EIS is provisioned with Single node cluster. In order to deploy EIS on multiple nodes using docker registry, provision ETCD cluster and 
remote managibility using turtlecreek, please follow [docker_setup/deploy/README.md](docker_setup/deploy/README.md)


# Debugging options

1. To check if all the EIS images are built successfully, use cmd: `docker images|grep ia` and
   to check if all containers are running, use cmd: `docker ps` (`one should see all the dependency containers and EIS containers up and running`). If you see issues where the build is failing due to non-reachability to Internet, please ensure you have correctly configured proxy settings and restarted docker service. Even after doing this, if you are running into the same issue, please add below instrcutions to all the dockerfiles in `docker_setup\dockerfiles` at the top after the LABEL instruction and retry the building EIS images:

    ```sh
    ENV http_proxy http://proxy.iind.intel.com:911
    ENV https_proxy http://proxy.iind.intel.com:911
    ```

2. `docker ps` should list all the containers which are included in docker-compose.yml

3. To verify if the data pipeline withing EIS is working fine i.e., from ingestion -> classification -> publishing classifed metadata onto the databus, then check the logs of `ia_video_analytics` container using cmd: `docker logs -f ia_video_analytics`. One should see, publish messages like `Publishing topic: [topic_name]`

5. `/opt/intel/eis` root directory gets created - This is the installation path for EIS:
     * `data/` - stores the backup data for persistent imagestore and influxdb

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

3. For debug purpose, it becomes essential to send dev team the logs of the build/run scripts to rootcause the issue effectively. This is     where the `tee` command comes to rescue.
4. 
4. Best way to check logs of containers is to use command: `docker logs -f [cont_name]`. If one wants to see all the docker-compose service container logs at once, then just run
   `docker-compose logs -f`

---

