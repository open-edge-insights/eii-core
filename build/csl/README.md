# EIS Orchestration using CSL

## CSL Setup

* For installing CSL Manager, CSL server and CSL client

    * Please refer orchestrator repo.

      > **Note**: Installation of CSL is a pre-requisite before EIS Provisioning in following steps.

## Provisioning EIS with CSL

> **Note**: EIS Deployment with CSL can be done only in **PROD** mode.

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
        ```


## Deploying VideoIngestion,VideoAnalytics & WebVisualizer of EIS in CSL.

> **NOTE**:
> For registering module manifest with CSL Software Module Repository **csladm** utility is needed. Please copy the module spec json files to the machine where you are having **csladm** utilty.
> It was advisable to use `csladm` utility in CSL Manager installed node.
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
## Deploying Telegraf, InfluxDbConnector, Kapacitor & Grafana of EIS TimeSeries use-case in CSL.

> **NOTE**:
> For registering module manifest with CSL Software Module Repository **csladm** utility is needed. Please copy the module spec json files to the machine where you are having **csladm** utilty.
> It was advisable to use `csladm` utility in CSL Manager installed node.
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
     $   whitelisted_mounts=/dev
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

* Update the Appspec Execution Environment as needed by individual modules.

* Open CSLManager in your browser.

* Click on **Submit New App** button, which pop's up a window to paste the Appspec.

    * Copy the Appspec of EIS-CSL from 
        [build/csl/csldeploy.json](../csl/csldeploy.json)
        and paste it in Window & Submit.

    * Verify the logs of deployed application status.


> **NOTE**
> Running CASL in Multiple Node is "Work In Progress"