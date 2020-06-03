# EIS Orchestration using CSL

## CSL Setup

* For installing CSL Manager, CSL server and CSL client

    * Please refer orchestrator repo.

      > **Note**: Installation of CSL is a pre-requisite before EIS Provisioning in following steps.

## Provisioning EIS with CSL

 **Note**: 

1. EIS Deployment with CSL can be done only in **PROD** mode.

2. For running EIS in multi node, we have to identify one master node. For a master node, ETCD_NAME in [build/.env](../.env) must be set to `master`.

Provisioning EIS with CSL is done in 2 ways. 

1.  [EIS Master/Single Node Provisioning in CSL Client Node](#eis-mastersingle-node-provisioning-in-csl-client-node)
    * This is the Mandatory Provisioning step should be done atleast in **1** CSL Client node for deploying EIS.

2.  [EIS Slave node Provisioning in CSL Client Node](#eis-slave-node-provisioning-in-csl-client-node)
    * This Provisioning step should be done when we are deploying EIS in CSL on multiple Client nodes.
    * It should be done after Master Node Provisioning is done in **1** CSL Client node for deploying EIS.
    * Slave provisioning will create only the EIS dependent directories, users & its permissions.
    
    **Note** Slave Provisioning should not be done alone without master node provisioning. 

### EIS Master/Single Node Provisioning in CSL Client Node

To Deploy EIS with CSL. EIS has to provisioned in "csl" mode. Please follow the below steps.

* EIS Should be provisioned properly in Client Machine. Please Select the Client machine where you want provision the eis by following below steps.

* Below script loads values from json file located at [build/provision/config/eis_config.json](../../build/provision/config/eis_config.json) to CSL Datastore based on the 
  generated users & keys of CSL datastore.
    
    * Please Update following Environment Variables in [build/provision/.env](../../build/provision/.env)
      Before your provisioning in CSLMode
        * PROVISION_MODE=csl
        * CSL_MGR_USERNAME         =   [csl manager username]
        * CSL_MGR_PASSWORD         =   [csl manager password]
        * CSL_MGR_IP               =   [csl manager IP]
        * DEFAULT_COMMON_NAME      =   [Etcd username for EtcdUI login]
        * DEFAULT_COMMON_PASSWORD  =   [Etcd password for EtcdUI login]

    * Please Update following Environment Variables in [build/.env](../../build/.env)
        * ETCD_PREFIX=/csl/apps/EIS
        * Under Etcd Client Settings Update ETCD_HOST=<csl manager ip address/virtual ip address>
        * Update ETCD_CLIENT_PORT if needed.

    **NOTE** please make sure your CSL Manger IP address is part of eis_no_proxy for it's Communication.

    * Please follow the EIS Pre-requisites before CSL Provisioning.
        [EIS Pre-requisites](../../README.md#eis-pre-requisites)


        ```sh
        $ sudo ./provision_eis.sh <path_to_eis_docker_compose_file>
    
        eq. $ sudo ./provision_eis.sh ../docker-compose.yml

### EIS Slave node Provisioning in CSL Client Node
>**Note** This should be used in other than EIS master CSL Client nodes on ***Multi node scenario*** only. This is not a primary provisioning step for **Single Node**.
  * This step to be followed from the EIS Slave Client node machines for enabling EIS in mulit nodes of CSL.
  * Pre requisites:
    * EIS Master Node Provisioning should be done any other **1** CSL client node.
  * Please follow the EIS Pre-requisites before CSL Provisioning.
    [EIS Pre-requisites](../../README.md#eis-pre-requisites)
  * Change the value of ETCD_NAME=<any name other than `master`> in [build/.env](../../build/.env).
  * Goto `build/provision` directory.
  * Provisioning EIS Slave node in CSL Client node.
    ```sh
      $ sudo ./provision_eis.sh
    ```
  


## Generating the CSL Appspec & Module Spec.
  * CSL Appspec will be auto-generated under **build/csl** folder as **csl_app_spec.json** while running [eis_builder](../eis_builder.py) as a part of EIS pre-requisites.

## Deploying VideoIngestion, VideoAnalytics, EtcdUI, InfluxDBConnector, ImageStore, WebVisualizer & Visualizer of EIS in CSL.

> **NOTE**:
> For registering module manifest with CSL Software Module Repository **csladm** utility is needed. Please copy the module spec json files to the machine where you are having **csladm** utilty.
> Use the Module specs present in every app's individual folders.
> It is advisable to use `csladm` utility in CSL Manager installed node.
> * For more details please this command:    
>        ```sh
>        $ ./csladm register artifact -h
>       ```

* Load Module spec of VideoIngestion/VideoAnalytics/WebVisualizer modules to CSL manager following commands using CSL admin utility.

    * VideoIngestion
    
      ```sh
      $ ./csladm register artifact --type file  --name videoingestion --version 2.3 --file ./vi_module_spec.json 
      ```
    
    * VideoAnalytics
    
      ```sh
      $ ./csladm register artifact --type file  --name videoanalytics --version 2.3 --file ./va_module_spec.json 
      ```
    
    * WebVisualizer
      
      ```sh
      $ ./csladm register artifact --type file  --name webvisualizer --version 2.3 --file ./webvis_module_spec.json
      ```

    * Visualizer

      ```sh
      $ ./csladm register artifact --type file  --name visualizer --version 2.3 --file ./visualizer_module_spec.json
      ```

    * InfluxDBConnector

      ```sh
      $ ./csladm register artifact --type file  --name influxdbconnector --version 2.3 --file ./influxdbconnector_module_spec.json
      ```

    * ImageStore

      ```sh
      $ ./csladm register artifact --type file  --name imagestore --version 2.3 --file ./imagestore_module_spec.json
      ```

    * EtcdUI

      ```sh
      $ ./csladm register artifact --type file  --name etcdui --version 2.3 --file ./etcd_ui_module_spec.json
      ```

## Deploying Video Streaming without storage in CSL.

* To deploy the Video Streaming application

  * Delete the InfluxDBConnector and ImageStore configurations from the video_deploy.json appspec.

    Config that required to be deleted in the **modules** section,
    ```
        {
            "Name": "InfluxDBConnector",
            "Description": "InfluxDBConnector module manifest",
            "SchemaVersion": "0.2",
            "ManifestFile": "${idx:influxdbconnector:2.3}",
            "RunAsUser": "$EIS_UID",
            "RunAsGroup": "$EIS_UID",
            "Resources": {
                "CPU": 100,
                "MemoryMB": 100
            },
	    "Constraints": {
		"influx": "true"
            },
            "ExecutionEnv": {
                "AppName": "InfluxDBConnector",
                "ETCD_ENDPOINT": "${datastore.endpoint}",
                "CONFIGMGR_CACERT": "${databucket.cacert}",
                "CONFIGMGR_CERT": "${databucket.cert}",
                "CONFIGMGR_KEY": "${databucket.key}",
                "ETCD_PREFIX": "$ETCD_PREFIX",
                "DEV_MODE": "$DEV_MODE",
                "PROFILING_MODE": "false",
                "ZMQ_RECV_HWM": "1000",
                "Clients": "Visualizer",
                "Server": "zmq_tcp,0.0.0.0:${ep.sOutInflux.localport}",
		"SubTopics": "VideoAnalytics/camera1_stream_results",
                "camera1_stream_results_cfg": "zmq_tcp,${ep.influx-va-in.remoteaddress}:${ep.influx-va-in.remoteport}"
            },
            "Endpoints": [
                {
                    "Name": "sOutInflux",
                    "Endtype": "server",
                    "Port": "8675",
		    "DataType": "messages",
                    "Link": "sinfluxout"
                },
                {
                    "Name": "influx-va-in",
                    "Endtype": "client",
                    "DataType": "messages",
                    "Link": "va-is-influx-web-vis"
                }
            ]
        },
        {
            "Name": "ImageStore",
            "Description": "ImageStore module manifest",
            "SchemaVersion": "0.2",
            "ManifestFile": "${idx:imagestore:2.3}",
            "RunAsUser": "$EIS_UID",
            "RunAsGroup": "$EIS_UID",
            "Resources": {
                "CPU": 100,
                "MemoryMB": 100
            },
            "ExecutionEnv": {
                "AppName": "ImageStore",
                "ETCD_ENDPOINT": "${datastore.endpoint}",
                "CONFIGMGR_CACERT": "${databucket.cacert}",
                "CONFIGMGR_CERT": "${databucket.cert}",
                "CONFIGMGR_KEY": "${databucket.key}",
                "ETCD_PREFIX": "$ETCD_PREFIX",
                "DEV_MODE": "$DEV_MODE",
                "PROFILING_MODE": "false",
                "ZMQ_RECV_HWM": "1000",
                "Clients": "Visualizer",
                "Server": "zmq_tcp,0.0.0.0:${ep.out-is-server.localport}",
		"SubTopics": "VideoAnalytics/camera1_stream_results",
                "camera1_stream_results_cfg": "zmq_tcp,${ep.is-va-in.remoteaddress}:${ep.is-va-in.remoteport}"
            },
            "Endpoints": [
                {
                    "Name": "out-is-server",
                    "Endtype": "server",
                    "Port": "5669",
		    "DataType": "messages",
                    "Link": "server-is"
                },
                {
                    "Name": "is-va-in",
                    "Endtype": "client",
                    "DataType": "messages",
                    "Link": "va-is-influx-web-vis"
                }
            ]
	}
    ```

    Config that required to be deleted in the **Links** section,
    ```
	{
            "Name": "sinfluxout"
        },
        {
            "Name": "server-is"
        },
    ```

## Deploying Telegraf, InfluxDbConnector, Kapacitor & Grafana of EIS TimeSeries use-case in CSL.

> **NOTE**:
> For registering module manifest with CSL Software Module Repository **csladm** utility is needed. Please copy the module spec json files to the machine where you are having **csladm** utilty.
> Use the Module specs present in every app's individual folders.
> It is advisable to use `csladm` utility in CSL Manager installed node.
> * For more details please this command:
>        ```sh
>        $ ./csladm register artifact -h
>       ```

* Load Module spec of Telegraf/InfluxDbConnector/Kapacitor/Grafana modules to CSL manager following commands using CSL admin utility.

    * Telegraf
    
      ```sh
      $ ./csladm register artifact --type file  --name telegraf --version 2.3 --file ./telegraf_module_spec.json 
      ```
    
    * InfluxDbConnector
    
      ```sh
      $ ./csladm register artifact --type file  --name influxdbconnector --version 2.3 --file ./influxdbconnector_module_spec.json 
      ```
    
    * Kapacitor
      
      ```sh
      $ ./csladm register artifact --type file  --name kapacitor --version 2.3 --file ./kapacitor_module_spec.json
      ```

    * Grafana

      ```sh
      $ ./csladm register artifact --type file  --name grafana --version 2.3 --file ./grafana_module_spec.json
      `

*  Update the Container Image along with Registry details in Module Spec Files.

    * Build & Update the Docker Repository and Images.

        ```sh
        "RuntimeOptions": {
            "ContainerImage": "<Docker Regsitry>:ia_web_visualizer:2.3"
        }
        ```

* Update the Settings for Mounting External Devices to CSL.

  * Goto **CSL Manager** Machine.
     ```sh
     $   sudo vi /opt/csl/csl-manager/application.properties
     ```   
  * Set **whitelisted_mounts** property value to **/dev** directory.
     ```sh    
     $   whitelisted_mounts=/dev,/var/tmp,/tmp/.X11-unix,/opt/intel/eis/data,/opt/intel/eis/saved_images
     ```
  > Save the file.

  * Restart the **csl-manager** System Service
    ```sh    
    $   sudo systemctl restart csl-manager
    ```
>**Below steps are Mandatory, should be performed in all client nodes**
* Steps to set the **CSL Network Configuration** for Accesseing Basler Camera

  * Create a new Network using **csladm** utility by following command in your ***client machine***
  ```sh
    sudo /opt/csl/csl-node/bin/csladm register network
  ```
  * Enter the network configuration name as follows:
  ```sh
    Network configuration Name: cslhostnetworkinterface
  ```
  * Enter the Host Network Interface Name
  ```sh
    Network configuration default interface [eth0]: <host network interface>
  ```
  * Type ***Y*** and continue.

  >**Note** The host network interace name should be your client machine & basler camera connected interface name.

* Steps to set the Accelarator Node Affinity to CSL Client Nodes.

  * CSL has a node affinity feature and we can restrict particular module/application to deploy on specific node.
  * For HDDL/NCS2 dependenecies follow the steps for setting node affinity.
  * Using CSL API Set the Meta Key & Meta Value on particular node as follows
    * For HDDL
      ```sh
      $ curl -k -H "Content-type: application/json" -u <username> -X PUT "https://csl-mgr-ip/api/v1/nodes/nodename/metadata" -d '{"hddl": "true"}'
      ```
    * For NCS2
      ```sh
      $ curl -k -H "Content-type: application/json" -u <username> -X PUT "https://csl-mgr-ip/api/v1/nodes/nodename/metadata" -d '{"ncs2": "true"}'
      ```
    **Note** Here the nodename is your client machine hostname you can refer the same in CSL Manager Client nodes section.

    * Updating the Appspec
      Select the appspec and update the **Constraints** under Modules section.
      * For HDDL
        In *VideoAnalytics* or *VideoIngestion* based on your application usage Section of Appspec under *Modules* Update the follwing key values:
        ```sh
           "Constraints": {
                "hddl": "true"
            }
        ```
      * For NCS2
        In *VideoAnalytics* or *VideoIngestion* based on your application usage Section of Appspec under *Modules* Update the follwing key values:
        ```sh
           "Constraints": {
                "ncs2": "true"
            },
        ``
      **Note** Make Sure that you have appended properly and validate the json.

>**Below steps are recommended, to use the storage feature and keep the data consistent of Video Streaming and Historical application in multinode setup across multiple deployment**
  * Open the video_deploy.json file.

  * Add the following section in the ImageStore and InfluxDBConnector modules in appspec.
  ```sh
    "Constraints": {
	"key": "value"
    },
  ```
  Adding "Constraints" to modules will tie the Pods of those modules to particular node (which have the same labels (key:value) set).

  * Set the labels in a Node, where InfluxDBConnector and ImageStore will run, as these containers store data in local file system
  it is necessary to fix a Node

  * It can be set during the CSL-Client installation, or it can be updated with the help of API exposed by CSL Manager
  ```sh
     $  curl -k -H "Content-type: applicatio/json" -u user -X PUT "https://csl-manager-host:8443/api/v1/nodes/node/metadata" -d '{"key":"value"}'

     where node: string representation of the client name
  ```

  * To delete the labels from node use the following command
  ```sh
     $  curl -k -H "Content-type: applicatio/json" -u user -X DELETE "https://csl-manager-host:8443/api/v1/nodes/node/metadata" -d '{"key":"value"}'

     where node: string representation of the client name
  ```

  >**Note** If the labels are not set in any Node, and Constraints section is added in the InfluxDBConnector and ImageStore modules in appspec. InfluxDBCOnnector and ImageStore modules will not launch.

* Update the Appspec Execution Environment as needed by individual modules.

* Open CSLManager in your browser.

* Click on **Submit New App** button, which pop's up a window to paste the Appspec.

    * Copy the Appspec of EIS-CSL from 
        [build/csl/csl_app_spec.json.json](../csl/csl_app_spec.json.json)
        and paste it in Window & Submit.

    * Verify the logs of deployed application status.

