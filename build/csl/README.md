# EIS Orchestration using CSL Orchestrator

EIS Orchestration using CSL Orchestrator.

1. [CSL Setup](#csl-setup)

2. [EIS CSL Pre-requisites](#eis-csl-pre-requisites)

3. [Provisioning EIS with CSL](#provisioning-eis-with-csl)

4. [Registering EIS ModuleSpecs with CSL SMR](#registering-eis-modulespecs-with-csl-smr)

5. [Deploying EIS Application with CSL Using CSL Manager UI
](#deploying-eis-application-with-csl-using-csl-manager-ui)

6. [Steps to enable Basler Camera](#steps-to-enable-basler-camera)

7. [Steps to enable Accelarators](#steps-to-enable-accelarators)

8. [Steps to enable FPGA](#steps-to-enable-fpga)

9. [Steps to enable IPC Mode](#steps-to-enable-ipc-mode)

10. [Recomendations for database orchestration](#recomendations-for-database-orchestration)


## CSL Setup

* For installing CSL Manager, CSL server and CSL client

    * Please refer orchestrator repo.

      > **Note**: Installation of CSL is a pre-requisite before EIS Provisioning with CSL in following steps.

## EIS CSL Pre-Requisites
  ### EIS Pre-Requisites
  * Please follow the [EIS Pre-requisites](../../README.md#eis-pre-requisites) steps for generating the appspecs & module specs using EIS Builder.

  ### Update the Settings for Mounting External Devices/Volumes to CSL Client Nodes.
  >**Note** This steps should be done in `cslmanager` machine only.
  * Goto **CSL Manager** Machine.
      ```sh
      $   sudo vi /opt/csl/csl-manager/application.properties
      ```   
  * Set **whitelisted_mounts** property value as follows.
      ```sh    
      $   whitelisted_mounts=/dev,/var/tmp,/tmp/.X11-unix,/opt/intel/eis/data,/opt/intel/eis/saved_images,/opt/Intel/OpenCL/Boards,/opt/altera,/opt/intel/intelFPGA,/opt/intel/openvino
      ```
  > Save the file.

  * Restart the **csl-manager** docker container
    ```sh    
    $   docker restart <csl-manager-containerid>
    ```


 
  ### Update the Container Image details in Module Spec Files.
> **NOTE**:
> For registering module manifest with CSL Software Module Repository **csladm** utility is needed. Please copy the module spec json files to the machine where you are having **csladm** utilty.
> Use the Module specs present in every app's individual folders.
> It is advisable to use `csladm` utility in CSL Manager installed node.
> * For more details please use this command:    
>   ```sh
>        $ ./csladm register artifact -h
>   ```
> * For `csladm` utility we can use Env Variable to Set the CSL Manager & Software Module Repository to Avoid Frequent User Interaction on using this `csladm` utility for every operation.
>    ```sh
>      export CSLADM_USERNAME=<csl manager username>
>      export CSLADM_PASSWORD=<csl manager password>
>      export CSLADM_SMR_USERNAME=<csl smr username>
>      export CSLADM_SMR_PASSWORD=<csl smr password>
>    ```

*  Pre-Requisites
  
    **Note** For Running EIS Modules in a node corresponding module Container Image should be present in the docker registry / SMR.

 
* Updating the Module Spec files based on the Docker Regitry / Software Module Repo.
  > **Note** For EIS If you are using docker image without any registry, no need to change anything in module spec `ContainerImage` Section. by default image name will be there.

  * For Docker Registry
    ```sh
    "RuntimeOptions": {
        "ContainerImage": "${DOCKER_REGISTRY}:${EIS_VERSION}"
    }
    ```

  * For SMR 
    
    **Note** Registering docker image with SMR is optional incase of docker registry setup is not available. SMR can be used as needed.
    * Build & Update the Docker Repository and Images.
      * For Saving Docker Images
        ```sh
          $ docker save imagename:version > name.tar.gz
        ```
      **Note** `name` is the Reference name which you give it for each image while saving and the same used while loading in another machine docker.
      
      * For Registering & Loading the Saved Image with SMR
  >   **Note** Registering docker image with SMR is optional incase of docker registry setup is not available. SMR can be used as needed.
        
    ```sh
          $ ./csladm register artifact --type docker --smr-host <smr_host_ip> --csl-mgr-host <csl_mgr_ip> --file ./name.tar.gz --name <modulename> --version <EIS_VERSION>
    ``` 
    * Update generated Module Manifest file based on SMR registered artifact name and version.
      ```sh
      "RuntimeOptions": {
          "ContainerImage": "${idx:<registered_artifactname>:EIS_VERSION}"
      }
    

## Provisioning EIS with CSL

 **Note**: 

1. EIS Deployment with CSL can be done only in **PROD** mode.

2. For running EIS in multi node, we have to identify one master node. For a master node, ETCD_NAME in [build/.env](../.env) must be set to `master`.

3. Please follow the [EIS Pre-requisites](../../README.md#eis-pre-requisites) before CSL Provisioning.
   

Provisioning EIS with CSL is done in 2 steps. 

1.  [EIS Master Node Provisioning in CSL Client Node](#eis-master-node-provisioning-in-csl-client-node)
    * This is the Mandatory Provisioning step should be done atleast in **1** CSL Client node for deploying EIS.

2.  [EIS Worker Node Provisioning in CSL Client Node](#eis-worker-node-provisioning-in-csl-client-node)
    * This Provisioning step should be done when we are deploying EIS in CSL on multiple Client nodes.
    * It should be done after master Node Provisioning is done in **1** CSL client node for deploying EIS.    
    **Note** EIS CSL worker node provisioning should be done after master node provisioning done. 

### EIS Master Node Provisioning in CSL Client Node

  * To Deploy EIS with CSL, EIS has to be provisioned in "csl" mode.

  * Please Select the client machine where you want provision the `master eis csl node` by following below steps.
    
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
  * Provision EIS CSL Master/Single node.
        ```sh
        $ sudo ./provision_eis.sh <path_to_eis_docker_compose_file>

        eq. $ sudo ./provision_eis.sh ../docker-compose.yml
        ```

### EIS Worker Node Provisioning in CSL Client Node
>**Note** This should be used in other than EIS master CSL Client nodes on ***Multi node scenario*** only. This is not a primary provisioning step for **Single Node**.
  * Pre requisites:
    * EIS Master Node should be provisioned in any other CSL client node.
    **Note:** EIS Master Node provision should done only in *one* system.
    
  * Generate the EIS CSL Worker Node Provisioning Setup Bundle & Provision EIS CSL Worker Client node refer as follows
     *  [EIS CSL Worker Node Provisioning Setup Bundle Generation](../deploy/README.md#step-6-generate-eis-bundle-for-csl-worker-node-provisioning)
  
  **Note** Make Sure ETCD_NAME=<any name other than `master`> in [build/.env](../../build/.env).

## Generating the CSL Appspec & Module Spec.
  * CSL Appspec will be auto-generated under **build/csl** folder as **csl_app_spec.json** while running [eis_builder](../eis_builder.py) as a part of EIS pre-requisites.

## Registering EIS ModuleSpecs with CSL SMR. 
**Note** Module Specs will be generated under `build/csl` directory based on the EIS Repo.

* Please use the following command to register the module spec with CSL manager using CSL admin utility.
    ```sh
    $ ./csladm register artifact --type file --name <modulename> --version <EISVersion> --file ./<module_spec_file>
    ```
    For Eg:
    *   Registering VideoIngestion Module Spec with CSL SMR.
        ```sh
        $ ./csladm register artifact --type file --name videoingestion --version 2.3 --file ./vi_module_spec.json
        ```
  **Note** Please refer the modulename as per the appspec and also confirm both are same for proper reference by csl manager.

## Deploying EIS Application with CSL Using CSL Manager UI
* Update the Appspec Execution Environment as needed by individual modules.
    * In case of time series, update the "MQTT_BROKER_HOST" env of telegraf module in **csl_app_spec.json**. Replace "127.0.0.1" to mosquito broker's IP address, to receive the data in telegraf.

* Open CSLManager in your browser.

* Click on **Submit New App** button, which pop's up a window to paste the Appspec.

* Copy the Appspec of EIS-CSL from 
    [build/csl/csl_app_spec.json.json](../csl/csl_app_spec.json.json)
    and paste it in Window & Submit.

* Verify the logs of deployed application status.

## Steps to enable Basler Camera.
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

  * Update the VideoIngestion Section CSL Appspec `build/csl/csl_app_spec.json` file.
    **Note** The below changes should be appended below the `Resources` key in `VideoIngestion` Module section of appspec.
    ```sh
        "Networks": [
                "cslhostnetworkinterface"
        ]
    ``` 
  * Validate the json and save it.

  >**Note** The host network interace name should be your client machine & basler camera connected interface name.

## Steps to enable Accelarators

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
            }
        ``
      **Note** Make Sure that you have appended properly and validate the json.
      
      * For Deleting the Node Affinity Label Key/Values.
        ```shss
        $  curl -k -H "Content-type: applicatio/json" -u <user> -X DELETE "https://csl-manager-host:8443/api/v1/nodes/<nodename>/metadata" -d '{"key":"value"}'
        ```

## Steps to enable FPGA
  * Pre-Requisites
      * Please setup the FPGA using this [Readme](../../README_fpga.md)

  * Update `/opt/intel/openvino` directory in `VideoIngestion` and `VideoAnalytics` modulespec `TemporaryVolumes` section.

  ```sh
    $ vi buid/csl/deploy/VideoIngestion_module_spec.json
    $ vi buid/csl/deploy/VideoAnalytics_module_spec.json
  ```

  ```sh
    "/opt/intel/openvino:/opt/intel/openvino"
  ```

  for eg:

  ```sh
      "TemporaryVolumes": [
        "/dev:/dev",
        "/var/tmp:/var/tmp",
        "/opt/Intel/OpenCL/Boards:/opt/Intel/OpenCL/Boards",
        "/opt/altera:/opt/altera",
        "/opt/intel/intelFPGA:/opt/intel/intelFPGA",
        "/opt/intel/openvino:/opt/intel/openvino"
      ]
  ```
  
  * Register this Updated ModuleSpecs with SMR for reflecting changes. 
    * For Deleting the existing module spec use following command in *CSL Manager* machine.
    ```sh
    $   ./csladm delete artifact --type file --name <modulepsec_name> --version EIS_VERSION
    ```
    
    For Eg:
    ```sh
    $   ./csladm delete artifact --type file --name videoanalytics --version 2.3
    ```
    
    * Re-Register the updated Module Spec by following this [steps](#registering-eis-modulespecs-with-csl-smr).
        
    



## Steps to enable IPC mode

EIS Services can be enabled as IPC mode for efficient datatransfer between modules on same node. To enable IPC Communication mode between modules please follow the steps.

* Create an IPC/Socket Communication for IPC Mode under the module `Endpoints` section
  ```sh
  "Endpoints": [
     {
       "Name": "moduleAtoB",
       "DataType": "ZeroMQ/UNIX",
       "Endtype": "socket",
       "MountPath": "/sockets"
     }
   ]

  ```
  Use the above Endpoint Name and Socket path for comuunicating to other module.
  
  * Example: Creating IPC Endpoints for VideoIngestion & VideoAnalytics Module.

    * Update the VideoIngestion output `camera1_stream_cfg` key as follows:
      ```sh
          "camera1_stream_cfg": "zmq_ipc,${ep.outputsocket-vi.mountpath}"
      ```
    * Create socket endpoint with name `outputsocket-vi` in `Endpoints` section
      ```sh
          {
              "Name": "outputsocket-vi",
              "Endtype": "socket",
              "DataType": "ZeroMQ/UNIX",
              "MountPath": "/sockets",
              "Link": "vi-va-link"
          }
          
      ```
    * Update the VideoAnalytics input `camera1_stream_cfg` key as follows:
      ```sh
          "camera1_stream_cfg": "zmq_ipc,${ep.inputsocket-va.mountpath}"
      ```

    * Create socket endpoint with name `inputsocket-va` in `Endpoints` section
      ```sh
          {
              "Name": "inputsocket-va",
              "Endtype": "socket",
              "DataType": "ZeroMQ/UNIX",
              "MountPath": "/sockets",
              "Link": "vi-va-link"
          }
      ```
    > **Note** The Above Two Module Endpoints uses the same **MountPath** to share the socket file created by the corresponding module.Also uses the same **Link**.

    * Setting Module Affinity
      * `ModuleAffinity` groups modules such that the modules in the same group run on the same node.
      * Update ModuleAffinity in Appspec Under `RuntimeOptions` as follows
        ```sh
        "ModuleAffinity" : [ [moduleA,moduleB..etc]   ]
        ```
        Eg:
        * Setting Module Affinity for `VideoIngestion` & `VideoAnalytics` modules for running in `same` node.

        ```sh
          "ModuleAffinity" : [ ["VideoIngestion", "VideoAnalytics"]]
        ```
    

## Recomendations for database orchestration

EIS services InfluxDBConnector and ImageStore uses databases which store to disk. In an orchestrated environment, it is recommended to pin down these services to a specific node so that we get consistent data from the running service. It can be achieved by using the node labels and constraints of CSL.

Follow the steps to achieve the database orchestration. 

* Open the video_deploy.json file.

* Add the following section in the ImageStore and InfluxDBConnector modules in appspec.
    ```sh
        "Constraints": {
            "key": "value"
        }
    ```
Adding "Constraints" to modules will tie the Pods of those modules to particular node (which have the same labels (key:value) set).

* Set the labels in a Node, where InfluxDBConnector and ImageStore will run, as these containers store data in local file system
it is necessary to fix a Node

* It can be set during the CSL-Client installation, or it can be updated with the help of API exposed by CSL Manager
```sh
    $  curl -k -H "Content-type: applicatio/json" -u <user> -X PUT "https://csl-manager-host:8443/api/v1/nodes/<nodename>/metadata" -d '{"key":"value"}'

```

* To delete the labels from node use the following command
```sh
    $  curl -k -H "Content-type: applicatio/json" -u <user> -X DELETE "https://csl-manager-host:8443/api/v1/nodes/<nodename>/metadata" -d '{"key":"value"}'
```

>**Note** If the labels are not set in any Node, and Constraints section is added in the InfluxDBConnector and ImageStore modules in appspec. InfluxDBCOnnector and ImageStore modules will not launch.

