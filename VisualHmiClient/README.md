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

* Login as root User `sudo su`

* Clone or Copy `ElephantTrunkArch` Src under `/root/` Directory

* Configure Databus & VisualHMI Server Details
  ```sh
    vi /root/ElephantTrunkArch/VisualHMIClient/config.json
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

  * Building Docker Container

      * Go to `/root/ElephantTrunkArch`  Directory. **From ElephantTrunkArch dir follow below Steps**

      ```sh
      cp docker_setup/dockerignores/.dockerignore .
      docker build -f VisualHmiClient/Dockerfile -t visual_hmi .
      ```
  * Running VisualHmiClient App as Container (**current working dir should be the repo path**)
      ```sh
      docker run -v $PWD/cert-tool/Certificates:/eta/cert-tool/Certificates \
                 -v $PWD/VisualHmiClient/config.json:/eta/VisualHmiClient/config.json \ 
                 -v /root/saved_images:/root/saved_images --privileged=true \
                 --network host --name visualhmi \
                 -itd --restart always visual_hmi
      ```
    > **Note**:
    > * Please add --env no_proxy=localhost,<ETA_RUNNING_MACHINE_IP_ADDRESS> before --restart 
    >   always for running Behind Proxy Env
    > * Please make sure you have given required information in [config.json](config.json)
    > * Don't change mounted volumes directory. If you want to change make sure config.json also 
    >   updated
    > * Docker run will consider the local `config.json` as in `VisualHmiClient/config.json`
    >   Please make sure your config.json is updated
    > * If you want to change the config.json again No need to build again. As the host 
    >   /root/ElephantTrunkArch/VisualHMIClient/config.json is mounted to docker container.

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

Please follow the [VisualHMI EdgeServer Setup guide](https://github.intel.com/ElephantTrunkArch/ElephantTrunkArch/wiki/VisualHmi-EdgeServer-Setup) to setup VisualHmi backend and frontend apps (Only needed if one wants to see the classified images in a Web UI with defective areas highlighted which cannot be seen on the images that are stored on disk in the EdgeServer at the location dumped by VisualHmiClient).

> **Note**: There is a dockerized version of VisualHmi available for refinement backend/frontend   
>           apps. We should have a startup script to bring up the VisualHmi stack (VisualHmiClient
>           and VisualHmi refinement app backend/frontend apps) for easier
>           configuration/installation.