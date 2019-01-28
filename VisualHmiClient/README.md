# VisualHmiClient Module:

VisualHmiClient is a datasync app which uses python binding of `ImageStore` gRPC module to fetch the classified images from the `ia_image_store` container and python binding of `DataBusAbstraction` module to get the metadata of the classified messages from the DataAgent module via OPCUA.

> **Note**:
> * As a OPCUA client, VisualHmiClient app subscribes to `classifier_results` topic from DataAgent
>   module to get the metadata results. These metadata results are converted to json format and
>   posted to VisualHmi refinement app backend REST API endpoint.
> * As a gRPC client, VisualHmiClient app gets the image blob from the gRPC interface `Read
>  (imgHandle)` of `ImageStore` module and persist the image in the local filesystem.

## Setting up VisualHmi

### Configuration

* Clone `IEdgeInsights` repo dir.

* Configure Databus & VisualHMI Server Details

  ```sh
    vi [repo_dir]/VisualHMIClient/config.json
  ```

  Change the Databus Host & Port Details.
  **(Your IEI running machine IP & Opcua Port)**

> **NOTE**:
> If the IEI is running on a node behind a coporate network/proxy server, please set IP address      of the node in the no_proxy/NO_PROXY env > >
> variable on the system where you are executing VisualHmiClient app so that the communication doesn't go via the proxy server.
> Eg. `export no_proxy=$no_proxy,<IEI node IP address>`
> If this is not set, one would into gRPC errors like `StatusCode.UNAVIALABLE`

### Installation

#### Installing VisualHmiClient app

* Installing & Starting Docker Daemon Service in CentOS

  ```sh
    sudo yum install docker-ce
    sudo systemctl enable docker
    sudo systemctl start docker
  ```

  > **Note**:
  > Refer [docker_setup/README.md](../docker_setup/README.md) for docker daemon and container proxy
  > configuration

* Building and Running VisualHmiClient as a container (**present working dir - IEdgeInsights repo_dir**)

  ```sh
    sudo VisualHmiClient/build_and_run_visualhmiclient.sh IEI_IP_ADDR=[IEI_IP_ADDR] IMG_DIR=[IMG_DIR] LOCAL=[yes|no]

    where IEI_IP_ADDR refers to system's IP on which IEI is running on
          IMG_DIR refers to the image dir where the images are stored on the host
          LOCAL[yes|no] refers to if posting of metadata to VisualHmi backend
  ```

  > **Note**:
  > * Please make sure you have given required information in [config.json](config.json)
  > * Please ensure that the Certificates folder exists in [../cert-tool/Certificates](../cert-tool/Certificates) path
  >   as this is essential to provide the imagestore, opcua and ca certs to VisualHmiClient container via volume mount

* Follow [VisualHmiCleaner/README.md](VisualHmiCleaner/README.md) to start VisualHmiCleaner
  docker container utility to clear the classified images stored on the disk & postgresql entries where the classified results metadata is stored. If you just want to natively run some script to delete the classified images directory periodically, then you could use
  `sudo ./cleanup_visualhmi_dir.sh [full_path_to_saved_images_dir]`. Ensure that the directory provided here is same as the one that is in the 
  `config.json` as used by VisualHmiClient container.

* Useful docker commands for visualhmi
  * `docker stop visualhmi` - stops the container
  * `docker restart visualhmi` - restart the container
  * `docker logs -f visualhmi` - to fetch logs of the container

#### Installing VisualHmi refinement backend and frontend apps

Please follow the [VisualHMI Refinement App Setup](https://github.intel.com/IEdgeInsights/HMI-Docker/blob/master/README.md) to setup VisualHmi backend and frontend apps (Only needed if one wants to see the classified images in a Web UI with defective areas highlighted which cannot be seen on the images that are stored on disk in the EdgeServer at the location dumped by VisualHmiClient).
