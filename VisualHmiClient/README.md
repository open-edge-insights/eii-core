# VisualHMIClient Module:

VisualHMIClient is a datasync app which takes classifier results and posts the same to VisualHmi Server.

VisualHMI starts subscribing on databus topic (`classifier_results`) to get the classifier results from the databus server. Based on the Classifier results, VisualHMICLient converts the data to VisualHMI standard as a json and post the data to VisualHMI REST API endpoint.

Also, VisualHMIClient gets the image blob from the gRPC interface `GetBlob(imgHandle)` from DataAgent module and persist the image in VisualHMI local filesystem.

> **Note**:
> * VisualHMI runs only on python3.6 as our OPCUA Databus Client have support for python3.6 only at present.

## Pre-requisites:

* Please make sure that the below libraries availability. (either in `ElephantTrunkArch` or yourfolder)
  * DataBusAbstraction Library  (files under `DataBusAbstraction/py` in our `ElephantTrunkArch` repo)
  * GRPC Client wrapper (`client.py` and protobuff files:
  * For using python DatabusAbstraction py library, run the below steps:
    ```sh
    cd <repo>/DataBusAbstraction/py/test
    make build
    ```
    This generates `open62541W.so` in `<repo>/DataBusAbstraction/py/test` folder and also copies the same to `<repo>/DataBusAbstraction/py`.
    This is very much needed for all opcua clients who use python DataBusAbstraction APIs

**VisualHMIClient Can be run two Modes**
    * 1. Production Mode - Docker Based Containers
    * 2. BareMetal Setup - For Development Machines

## Setting Up VisualHMI in EdgeServer - Production Mode

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

## Steps to run VisualHMI (RefinementApp) module

* Running VisualHMI in production mode as Docker Container:
    * Installing & Starting Docker Daemon Service in CentOS
      ```sh
        sudo yum install docker-ce
        sudo systemctl enable docker
        sudo systemctl start docker
      ```
      > **Note**:
      > Set DockerDaemon & Container Proxy if required.
      > Please Refer ElephantTrunkArch/docker_setup/README.md for Proxy configuration

    * Building Docker Container

        * Go to `/root/ElephantTrunkArch`  Directory. **From ElephantTrunkArch dir follow below Steps**

        ```sh
          docker build -f VisualHmiClient/Dockerfile -t visual_hmi .
        ```
    * Running VisualHmiEtaDataSync App as Container
        ```sh
          docker run -v /root/ElephantTrunkArch/Certificates:/eta/VisualHmiClient/Certificates -v /root/ElephantTrunkArch/VisualHmiClient/config.json:/eta/VisualHmiClient/config.json -v /root/saved_images:/root/saved_images --privileged=true --network host --name visualhmi -itd --restart always visual_hmi
        ```
        > **Note**:
        Please add --env no_proxy=localhost,<ETA_RUNNING_MACHINE_IP_ADDRESS> before --restart always for running Behind Proxy Env
        > Please make sure you have given required information in config.json
        > Don't change mounted volumes directory. If you want to change make sure config.json also updated
        > Docker run will consider the local config.json as in VisualHmiClient/config.json
        > Please make sure your config.json is updated
        > If you want to change the config.json again No need to build again. As the host /root/ElephantTrunkArch/VisualHMIClient/config.json is mounted to docker container.

    * Stopping VisualHmi Container.
    ```sh
      docker stop visualhmi
    ```
    * Restarting VisualHmi Container.
    ```sh
      docker restart visualhmi
    ```
    * For VisualHmiContainer Logs
    ```sh
      docker logs -f visualhmi
    ```
> **Note**:
> `VisualHmiEtaDataSync` setup Ends by here.
> End of Setup.


> **Note**:
> Below Execution is not Recommnedded for Production Scenario:

## Setting Up VisualHMI in Development EdgeServer - BareMetal Setup

* Set your `PYTHONPATH` based on where libraries are placed
    For Eg:
    If the pwd is `ElephantTrunkArch` and it is under home which /root/:
    	export PYTHONPATH=$PYTHONPATH:~/ElephantTrunkArch:ElephantTrunkArch/DataBusAbstraction/py/:ElephantTrunkArch/DataAgent/da_grpc/protobuff
    else:
    	set your `PYTHONPATH` appropriately
* Install VisualHmiClient dependencies:

  ```sh
    pip3.6 install -r requirements.txt
  ```

* Running VisualHMI on Host (BareMetal - Not Recommnedded for Factory):
  ```sh
    python3.6 VisualHmiEtaDataSync.py -local <path to store image locally>
  ```

* Running VisualHMI without HMI Backend (For Testing)

  ```sh
    python3.6 VisualHmiEtaDataSync.py -local <path to store image locally>
  ```

> **Note**:
> Currently VisualHMI cannot be exited directly. Please use following command to terminate the VisualHMI App

```sh
  ps -ef | grep VisualHmiEtaDataSync.py | grep -v grep | awk {'print$2'} | xargs kill
```

>**Removing All Images Data from VisualHmi**
>Remove All Stored Images

  ```sh
    rm -r ~/saved_images/*
  ```
>Get into Postgre Db Console

  ```sh
    psql ali ali
  ```
>Execute Below Commands Inside Postgre Console

  ```sh
    DELETE FROM defect *;
    DELETE FROM image *;
    DELETE FROM part *;
  ```
  > **Note**:
  > The Removing Images Part is advised if you are doing pre-production Mode Validation.
  > Typically the above Scenario should not be handled. As it deletes all the Images.
