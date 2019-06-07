Edge Insights Software (EIS) is the framework for enabling smart manufacturing with visual and point defect inspections.

# Contents:

1. [Minimum System Requirements](#minimum-system-requirements)

2. [Docker pre-requisites](#docker-pre-requisities)

3. [EIS Pre-requisites](#eis-pre-requisites)

4. [Run EIS PCB Demo in Developer Mode](#run-eis-pcb-demo-in-developer-mode)

5. [Visualize the results](#visualize-the-results)

6. [Enable security (Production Mode)](#enable-security-production-mode)

7. [Enable camera based Video Ingestion](#enable-camera-based-video-ingestion)

8. [Using video accelerators](#using-video-accelerators)

9. [Time-series Analytics](#time-series-analytics)

10. [Install the EIS software to the production machine](#install-the-eis-software-to-the-production-machine)

11. [Usage of Docker Registry](#usage-of-docker-registry)

12. [Factory control app](#factory-control-app)

13. [Debugging options](#debugging-options)


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

4. **Optional:** For enabling full security, make sure host machine and docker daemon are configured with below security recommendations. [docker_security_recommendation.md](docker_security_recommendation.md)

# EIS Pre-Requisites

The section assumes the EIS software is already downloaded from the release package or from git.

* Download the full package for OpenVINO toolkit for Linux version "2019 R1.0.1" from the official website
(https://software.intel.com/en-us/openvino-toolkit/choose-download/free-download-linux) and extract it inside `IEdgeInsights/DataAnalytics/VideoAnalytics`. Post this step a directory named `l_openvino_toolkit_xxxxx/` will be present inside VideoAnalytics directory.

   > **NOTE**: Make sure there is always one `l_openvino_toolkit_xxxxx/` folder under IEdgeInsights/DataAnalytics/
   > VideoAnalytics folder as we are adding `l_openvino_toolkit_*` into Dockerfile which could result in
   > build failure of VideoAnalytics container if there are multiple openvino sdk's in there especially the old ones.


# Run EIS PCB Demo in Developer Mode

   A simple way to start EIS and see it in actiion is to enable the developer mode which disables all security and the provisioning steps.

   The master configuration for EIS is availalbe in the file [docker_setup/.env](docker_setup/.env).

   To enable Development mode, **DEV_MODE** variable in .env need to be set to true.
   
   ```sh
   # DEV_MODE if set `true` allows one to run IEI in non-secure mode and provides additional UX/DX etc.,
   DEV_MODE=true
   ```

   Next step is to build and run the EIS stack, which is achieved by the below command which does the build and run:

   > Note: All EIS build and run commands are to be executed from the IEdgeInsights/docker_setup/ directory.

   ```sh
   sudo make build run | tee build_run.txt
   ```

   The build and run steps can be split into two as well like below:
   ```sh
   sudo make build
   sudo make run
   ```

   Please note that the first time build of EIS containers take ~70 minutes.

   Once EIS starts executing, the logs can be seen by the below command:
   ```sh
   tail -f /opt/intel/iei/logs/consolidatedLogs/iei.log
   ```

   A successful run will give logs like below:
   ```sh
   Publishing topic: stream1_results
   Publishing topic: stream1_results
   Publishing topic: stream1_results
   Publishing topic: stream1_results
   .....
   ```

   We now have the analytics results published over OPC-UA and the corresponding image frames stored in the ImageStore.

# Visualize the results

For visualizing the results of the video analytics, the  [tools/visualizer](tools/visualizer) app is available in the repository. This is a sample app which uses the OPC-UA client for receiving the analytics results and the GRPC client for receiving the image frames and do a simple visualization. This demonstrates the usage of the `DataBusAbstraction` and `ImageStore` client libraries.

For running the visualizer follow the [tools/visualizer/README.md](tools/visualizer/README.md).

> Note: Use the developer mode option in Visualizer to run without security if the EIS is started in Dev mode.

# Enable security (Production Mode)

* Follow below steps to generate certificates, provision and build/start IEI.

   1. Certificates generation:

      Follow [cert-tool/README.md](cert-tool/README.md) to generate the required certificates/keys.

   2. Provision the secrets to Vault (**present working dir - `<IEdgeInsights>/docker_setup/`**)

      Run the script:

      ```sh
      sudo make provision CERT_PATH=<PATH_TO_CERTIFICATES_DIRECTORY> | tee provision_startup.txt

      E.g. sudo make provision CERT_PATH=../cert-tool/Certificates/
      ```

      This will take the inputs from [docker_setup/config/provision_config.json](docker_setup/config/provision_config.json) & read the cert-tool generated Certificates and save it securely by storing it in the Hashicorp Vault.
      It is responsibility of the Admin to remove the source directory wherein certificates exist.

      ---
      **Note**:
      If the admin wants to update the secrets in the vault, a re-provisioning step needs to be done like below:

      ```sh
      <Take back up of image store, influx or any other data if it is needed in future because provisioning steps deletes all of it>

      <Update the new values into provision_config.json & custom certificates directory if necessary>

      sudo make provision CERT_PATH=<PATH_TO_CERTIFICATES_DIRECTORY> | tee provision_startup.txt
      ```

      ---

EIS can work with a TPM hardware where the vault keys can be saved securely in the TPM. For using this mode, provide the right value for TPM_ENABLE in [.env](.env) file. This configuration should be used when the EIS is expected to leverage TPM for storing vault specific secret credentials. If one sets it to false, certain credentials are stored in file system.
    1. `true` -  for enabling the tpm to store credentials
    2. `false` - for disabling the tpm

    **NOTE**: Please use `TPM_ENABLE=true` only on systems where TPM hardware is present OR TPM is enabled using PTT Firmware in the BIOS.


# Enable camera based Video Ingestion

EIS supports various cameras like Basler (GiGE), RTSP and USB camera. The video ingestion pipeline is enabled using 'gstreamer' which ingests the frames from the camera. The Video Ingestion application accepts a user-defined trigger / filter algorithm to do pre-processing on the frames before it is ingested into the DBs and inturn to the Analytics container.

All the changes related to camera type are made in the factory_*.json files and samples are provided in [docker_setup/config/algo_config/](docker_setup/config/algo_config/) directory.

For detailed description on configuring different types of cameras and trigger algorithms, refer to the [VideoIngestion/README.md](VideoIngestion/README.md).

# Using video accelerators

EIS supports running inference on `CPU`, `GPU`, `Myriad` and `HDDL` devices as well by accepting device type (“CPU”|”GPU”|”MYRIAD”|”HDDL”) from factory json files in [algo_config](../algos/algo_config/) folder.
For running a sample with accelerators, use the factory_pcbdemo_myriad.json or factory_pcbdemo_hddl.json in the .env file. Note that it uses a different set of model files which are FP16 based.

> Note:
> To run on HDDL devices, make sure to uncomment the below section of code in [DataAnalytics/VideoAnalytics/va_classifier_start.sh](../DataAnalytics/VideoAnalytics/va_classifier_start.sh).

    ```sh
    #Uncomment these lines if you are using HDDL
    #$HDDL_INSTALL_DIR/bin/hddldaemon &
    #sleep 20
    ```

# Time-series Analytics

For time-series data, a sample analytics flow uses Telegraf for ingestion, Influx DB for storage and Kapacitor for classification. This is demonstrated with an MQTT based ingestion of sample temperature sensor data and analytics with a Kapacitor UDF which does threshold detection on the input values.

For enabling this, different set of containers need to be built in EIS and it can be selected by modifying the `IEI_SERVICES` field of .env file to services_pointdata.json. This will enable building of Telegraf and the Kapacitor based analytics containers.
More details on enabling this mode can be referred from [DataAnalytics/PointDataAnalytics/README.md](DataAnalytics/PointDataAnalytics/README.md)

The sample temperature sensor can be simulated using the [tools/mqtt-temp-sensor](tools/mqtt-temp-sensor) application.


# Install the EIS software to the production machine

For factory deployments, the EIS software is installed as a systemd service so that it comes up automatically on machine boot and starts the analytics process. For doing this, we would be installing `EIS` containers from `iei.service`. **The command below will unsinstall any previous version of `iei.service` (systemd service in ubuntu) and installs new one. This step does not need a provisioning step to be executed first.**

      ```sh
      sudo make install CERT_PATH=<PATH_TO_CERTIFICATES_DIRECTORY> | tee make_install.txt

      E.g. sudo make install CERT_PATH=../cert-tool/Certificates/
      ```

Post installation, IEI can be started / stopped using commands:

      ```sh
      sudo systemctl stop iei
      sudo systemctl start iei
      ```
# Usage of Docker Registry

This is an `optional` step where in IEI user wants to build and run EIS images using a DOCKER REGISTRY. This is very helpful
when one wants to pull and deploy docker images directly from a docker registry instead of building in every system.

Follow below steps:

* Configuring private docker registry

   Please refer to [official docker documentation](https://docs.docker.com/registry/deploying/) for setting up Docker Registry.

      **NOTE**:
      To access docker registry from the host system where docker push/pull operations are performed, please follow below commands:

         ```sh
         <Add docker registry system ip address to NO_PROXY in http-proxy.conf and https-proxy.conf files at `/etc/systemd/system/docker.service.d`>

         <If using insecure http registry, please follow the steps called out in the section `Deploy a plain HTTP registry` at [Insecure registry access](https://docs.docker.com/registry/insecure/)>

         sudo systemctl daemon-reload
         sudo systemctl restart docker

         ```


* In running in Production mode, follow [cert-tool/README.md](../cert-tool/README.md) to generate the required certificates/keys first before installing from registry.

* Building EIS and dist_libs images and pushing the same to docker registry (`Entire IEdgeInsights folder needs to be present on the system from where images are pushed to registry`)

      ```sh
      sudo make build push DOCKER_REGISTRY=<IP ADDRESS or URL>

      Eg: sudo make build push DOCKER_REGISTRY=ip_address:5000
      ```

* Pulling EIS images from configured docker registry and do a run without install.
Note: To pull and run from a DOCKER REGISTRY, only [docker_setup](../ docker_setup/) folder needs to be copied to target system.

   In case of DEV_MODE as true in .env file (Developer mode)

      ```sh
      sudo make pull run  DOCKER_REGISTRY=<IP ADDRESS or URL> | tee compose_startup.txt
      ```

   In case of DEV_MODE as false in .env file (Secure mode)

      ```sh
      sudo make pull provision run CERT_PATH=<PATH_TO_CERTIFICATES_DIRECTORY> DOCKER_REGISTRY=<IP ADDRESS or URL> | tee compose_startup.txt
      ```


* Pulling EIS images from configured docker registry and install for factory deployment
   `iei.service`. **The command below will unsinstall any previous version of `iei.service` (systemd service in ubuntu) and installs new one. This step does not need a provisioning step to be executed first.**

      ```sh
      sudo make install-registry CERT_PATH=<PATH_TO_CERTIFICATES_DIRECTORY>  DOCKER_REGISTRY=<IP ADDRESS or URL>

      Eg: sudo make install-registry CERT_PATH=../cert-tool/Certificates  DOCKER_REGISTRY=ip_address:5000
      ```

* Pulling `ia_dist_libs` image from docker registry and setup `/opt/intel/iei/dist_libs` client external libs distribution package

      ```sh
      sudo make distlibs-registry DOCKER_REGISTRY=<IP ADDRESS or URL>

      Eg: sudo make distlibs-registry DOCKER_REGISTRY=ip_address:5000
      ```


# Factory control app

The factory control app is a sample provided to demonstrate the implementation of a closed loop factory control mechanism which can be used for triggering alarm lights / control devices on a factory floor when a defect is detected in the visual inspection.
Follow [FactoryControlApp/README.md](../FactoryControlApp/README.md) for configuring the sample implementation. For using this app, the services_*.json file need to be updated to include this app in the build and run process.

# Debugging options

1. To check if all the IEI images are built successfully, use cmd: `docker images|grep ia` and
   to check if all containers are running, use cmd: `docker ps` (`one should see all the dependency containers and IEI containers up and running`). If you see issues where the build is failing due to non-reachability to Internet, please ensure you have correctly configured proxy settings and restarted docker service. Even after doing this, if you are running into the same issue, please add below instrcutions to all the dockerfiles in `docker_setup\dockerfiles` at the top after the LABEL instruction and retry the building IEI images:

    ```sh
    ENV http_proxy http://proxy.iind.intel.com:911
    ENV https_proxy http://proxy.iind.intel.com:911
    ```

2. `docker ps` should list the below containers in IEI stack:

   In Video analytics mode:
   - ia_video_ingestion
   - ia_imagestore
   - ia_video_analytics
   - ia_data_agent
   - ia_logrotate

   In time series data analytics mode:
   - ia_telegraf
   - ia_data_analytics
   - ia_data_agent
   - ia_logrotate

    **Note**: If any of the above containers are not listed, always use cmd: `sudo tail -f /opt/intel/iei/logs/consolidatedLogs/iei.log` to find out the reason for container failure

3. To verify if the data pipeline withing IEI is working fine i.e., from ingestion -> classification -> publishing classifed metadata onto the databus, then check the logs of `ia_data_agent` container using cmd: `docker logs -f ia_data_agent`. One should see, publish messages like `Publishing topic: [topic_name]`

4. To verify the E2E data flow working between IEI running on ECN (Edge Compute Node) and `iei-simple-visualizer` app running on the same node or on a diff node, check if the classified images and their respective metadata is been received in the `iei-simple-visualizer` container. 

5. `/opt/intel/iei` root directory gets created - This is the installation path for IEI:
     * `config/` - all the IEI configs reside here.
     * `logs/` - all the IEI logs reside here.
     * `dist_libs/` - is the client external libs distribution package
        * `DataAgentClient` -
            * cpp - consists of gRPC cpp client wrappers, protobuff files and test programs
            * py - consists of gRPC py client wrappers, protobuff files and test programs
        * `DataBusAbstraction` -
            * c - consists of opcua C client wrappers and test programs
            * py - consists of opcua py client wrappers and test programs
     * `secret_store/` - This is the vault's persistent storage wherein IEI secrets are stored in encrypted fashion. This directory is recreated                       on every provision step.
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

   **NOTE**: Now that we are encrypting the grpc internal secrets, please don't forget to run the command `source ./set_shared_key_nonce_env_vars.sh` as this sets the key and nonce ENVs needed for           the `docker-compose.yml` file
3. For debug purpose, it becomes essential to send dev team the logs of the build/run scripts to rootcause the issue effectively. This is     where the `tee` command comes to rescue.
4. Best way to check logs of containers is to use command: `docker logs -f [cont_name]`. If one wants to see all the docker-compose service container logs at once, then just run
   `docker-compose logs -f`
5. Run these commands to build & start the client tests container:
   `sudo ./client_tests_startup.sh`
   `docker-compose -f client-tests-compose.yml run --entrypoint /bin/bash ia_client_tests`
---
