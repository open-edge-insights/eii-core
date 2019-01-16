# VisualHmiClient Module:

VisualHmiClient is a datasync app (basically a gRPC/OPCUA client) which takes in the classified results from the DataAgent module via gRPC and OPCUA external communication channels and posts the same to [VisualHmi refinement app backend](https://github.intel.com/ElephantTrunkArch/refinement-app-backend) to be rendered in the [VisualHmi UI refinement app](https://github.intel.com/ElephantTrunkArch/refinement-app).

> **Note**:
> * As a OPCUA client, VisualHmiClient app subscribes to `classifier_results` topic from DataAgent
>   module to get the metadata results. These metadata results are converted to json format and 
>   posted to VisualHmi refinement app backend REST API endpoint.
> * As a gRPC client, VisualHmiClient app gets the image blob from the gRPC interface `GetBlob> 
>  (imgHandle)` from DataAgent module and persist the image in VisualHmi local filesystem.
> * VisualHmiClint app runs only on python3.6 as our OPCUA Databus Client have support for 
>   python3.6 only at present.

## Setting up VisualHmi

### Configuration

* Clone `ElephantTrunkArch` repo dir.

* Configure Databus & VisualHMI Server Details
  ```sh
    vi [repo_dir]/VisualHMIClient/config.json
  ```
  Change the Databus Host & Port Details.
  **(Your ETA running machine IP & Opcua Port)**

> **NOTE**:
  If the ETA is running on a node behind a coporate network/proxy server, please set IP address      of the node in the no_proxy/NO_PROXY env variable on the system where you are executing VisualHmiClient app so that the communication doesn't go via the proxy server.
  Eg. `export no_proxy=$no_proxy,<ETA node IP address>`
  If this is not set, one would into gRPC errors like `StatusCode.UNAVIALABLE`      

### Installation 

#### Installing VisualHmiClient app

- **Production Mode - Docker Based Containers (`Preferred` for test/factory system)**

  > **Note**: No need to follow **Pre-requisites** section as the VisualHmiClient docker
  >           image will have all the pre-requisites installed
  
  * Installing & Starting Docker Daemon Service in CentOS
    ```sh
      sudo yum install docker-ce
      sudo systemctl enable docker
      sudo systemctl start docker
    ```
    
  > **Note**:
  > Refer [docker_setup/README.md](../docker_setup/README.md) for docker daemon and container proxy 
  > configuration

  * Building and Running VisualHmiClient as a container (**present working dir - ElephantTrunckArch repo_dir**)

      ```sh
      sudo VisualHmiClient/build_and_run_visualhmiclient.sh ETA_IP_ADDR=[ETA_IP_ADDR] IMG_DIR=[IMG_DIR] LOCAL=[yes|no]

      where ETA_IP_ADDR refers to system's IP on which ETA is running on
            IMG_DIR refers to the image dir where the images are stored on the host
            LOCAL[yes|no] refers to if posting of metadata to VisualHmi backend
      ```
    > **Note**:
    > * Please make sure you have given required information in [config.json](config.json)
    > * Please ensure that the Certificates folder exists in [../cert-tool/Certificates](../cert-tool/Certificates) path
    >   as this is essential to provide the imagestore, opcua and ca certs to VisualHmiClient container via volume mount

  * Follow [VisualHmiCleaner/README.md](VisualHmiCleaner/README.md) to start VisualHmiCleaner    
    docker container utility to clear the classified images stored on the disk & postgresql entries where the classified results metadata is stored.

  * Stopping VisualHmi Container
  ```sh
    docker stop visualhmi
  ```

  * Restarting VisualHmi Container
  ```sh
    docker restart visualhmi
  ```

  * For VisualHmiContainer Logs
  ```sh
    docker logs -f visualhmi
  ```

#### Installing VisualHmi refinement backend and frontend apps

Please follow the [VisualHMI Refinement App Setup](https://github.intel.com/ElephantTrunkArch/HMI-Docker/blob/master/README.md) to setup VisualHmi backend and frontend apps (Only needed if one wants to see the classified images in a Web UI with defective areas highlighted which cannot be seen on the images that are stored on disk in the EdgeServer at the location dumped by VisualHmiClient).
