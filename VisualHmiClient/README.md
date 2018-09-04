# VisualHMIClient Module:

VisualHMIClient is a datasync app which takes classifier results and posts the same to VisualHmi Server.

VisualHMI starts subscribing on databus topic (`classifier_results`) to get the classifier results from the databus server. Based on the Classifier results, VisualHMICLient converts the data to VisualHMI standard as a json and post the data to VisualHMI REST API endpoint.

Also, VisualHMIClient gets the image blob from the gRPC interface `GetBlob(imgHandle)` from DataAgent module and persist the image in VisualHMI local filesystem.

> **Note**:
> * VisualHMI runs only on Python2.7 as our OPCUA Databus Client have support for python2.7 only at present.

## Pre-requisites:

* Please make sure that the below libraries availability. (either in `ElephantTrunkArch` or yourfolder)
  * DataBusAbstraction Library  (files under `DataBusAbstraction/py` in our `ElephantTrunkArch` repo)
  * GRPC Client wrapper (`client.py` and protobuff files:

* Set your `PYTHONPATH` based on where libraries are placed
    For Eg:
    If the pwd is `ElephantTrunkArch` and it is under home:
    	echo PYTHONPATH=$PYTHONPATH:~/ElephantTrunkArch:ElephantTrunkArch/DataBusAbstraction/py/:ElephantTrunkArch/DataAgent/da_grpc/protobuff
    else:
    	set your `PYTHONPATH` appropriately
* Install VisualHmiClient dependencies:

  ```sh
    pip2.7 install -r requirements.txt
  ```

* Configure Databus & VisualHMI Server Details using `config.json`

## Steps to run VisualHMI (RefinementApp) module


VisualHMIClient Can be run two Modes

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

        * Go to `ElephantTrunkArch` root Directory. **From ElephantTrunkArch dir follow below Steps**

        ```sh
          docker build -f VisualHmiClient/Dockerfile -t visual_hmi .
        ```
    * Running VisualHmiEtaDataSync App as Container
        ```sh
          docker run -v "$(pwd)"/VisualHmiClient/config.json:eta/VisualHmiClient/config.json: -v /root/saved_images:/root/saved_images --privileged=true --network host --name visualhmi -itd --restart always visual_hmi
        ```
        > **Note**:
        > Please make sure you have given required information in config.json
        > Don't change mounted volumes directory. If you want to change make sure config.json also updated
        > Docker run will consider the local config.json as in VisualHmiClient/config.json
        > Please make sure your config.json is updated

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
> Below Execution is not Recommnedded for Production Scenario:

* Running VisualHMI on Host (BareMetal - Not Recommnedded for Factory):
  ```sh
    python2.7 VisualHmiEtaDataSync.py -local <path to store image locally>
  ```

* Running VisualHMI without HMI Backend (For Testing)

  ```sh
    python2.7 VisualHmiEtaDataSync.py -local <path to store image locally>
  ```

> **Note**:
> Currently VisualHMI cannot be exited directly. Please use following command to terminate the VisualHMI App
> ```sh
  ps -ef | grep VisualHmiEtaDataSync.py | grep -v grep | awk {'print$2'} | xargs kill
> ```

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
