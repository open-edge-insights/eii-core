# Please follow Step 1,2,3 and 4a or 4b below to deploy EIS on multiple nodes.

# Step 1. Set up Docker Registry URL, Build and Push Images
EIS Deployment on multiple node must be done using a docker registry. 

Follow below steps:

* Please update docker registry url in DOCKER_REGISTRY variable in [docker_setup/.env](docker_setup/.env). Please use full registry URL with a traliling /

* Building EIS images and pushing the same to docker registry.

      ```sh
      docker-compose build
      docker-compose push

      ```

> **NOTE**: Please copy only docker_setup folder on node on which EIS is being launched through docker-registry and make sure all build dependencies are commented/removed form docker compose file before executing below commands.
> **NOTE**: Above commenting/removing build dependencies is not required if entire EIS repo is present on the node on which EIS is being launched through registry.


# Step 2. Add New node to EIS cluster

EIS uses ETCD for clustering. When EIS is deployed on a cluster having multiple node, ETCD will make sure all EIS services configurations and secrets are distributed across all nodes of the cluster.

By default EIS starts with single node cluster. 

For running EIS in multi node cluster mode, we have to indetify one master node. For a master node, ETCD_NAME in [docker_setup/.env](../.env) must be set to `master`. 

> **NOTE**: Master node is the primary administative node, which is used for following 
> * Generating Required Certificates and Secrets.
> * Loading Initial ETCD values.
> * Adding new node to ETCD Cluster.
> * Building and Pushing Docker images to registry.
> * Generating bundles to provision new nodes.
> * Generating bundles to deploy EIS services on new nodes.
> * Master node should have the entire repo present.

## Please execute Step 2.1 and Step 2.2 below to Add New Node to EIS Cluster.

> **NOTE**:  
> * Steps 'a' to be executed on `master` node'.
> * Step 'b' to be executed on 'new node' which will join EIS cluster.
> * Please execute below commands in directory [repo]/docker_setup/provision.
> * For environment under proxy, please make sure IP addresses or IP range for all nodes in a cluster are appended to no_proxy in [docker_setup/.env](../.env) for `master` node.



### Step 2.1 To be performed on Master node:

> **NOTE**: if EIS provisioning is not done on this node, please perform EIS provisioning first using below command. If EIS is already provisioned and running, then below step is not required.

        ```
        $ sudo ./provision_eis.sh <path_to_eis_docker_compose_file>

        eq. $ sudo ./provision_eis.sh ../docker-compose.yml

        ```
* <b>Please execute below command in working directory docker_setup/provision to join new node to cluster.</b>

```

 sudo ./etcd_cluster_add.sh <NodeNAme> <IP Address of New Node>
  
  e.q. sudo ./etcd_cluster_add.sh node2 10.223.109.170

```

> **NOTE**: Please note that `< NodeName >` and `< IP Address of the New Node >` above, must not be part of existing node list of the cluster. (Please use command `./etcd_cluster_info.sh` to see existing node list of the cluster.)

* Once the above command is executed successfully, a message should come that node is added to cluster. 
* Also it will create a `<NodeName>_provision.tar.gz` file in docker_setup/provision/ folder. This file needs to be copied to New Node.

> **NOTE**: Please make sure to copy and untar the above bundle on a secure location having root only access as it container secrets.

```

  ./scp <NodeName>_provision.tar.gz user@<IP Address of New Node>:~
  
  e.q. scp ./node2_provision.tar.gz user@10.223.109.170:~

```

### Step 2.2 To be performed on New node:


> **NOTE** EIS should not be running or provisioned on new node.  Before provisioning, below steps needs to be performed.

 * ` <NodeName>_provision.tar.gz` is copied as per step 1 above from Existing Node.
 
 * Once above file is copied, Provision EIS using below command and it will join new node to existing cluster.
 
```
    $ sudo tar -xvf <NodeName>_provision.tar.gz
    $ cd <NodeName>_provision/provision
    $ sudo ./provision_eis.sh

    eq. 
    
    $ sudo tar -xvf node2_provision.tar.gz
    $ cd node2_provision/provision
    $ sudo ./provision_eis.sh

```
* Once provisioning is done, please verify that New Node is added to cluster by below command.
```
    $  ./etcd_cluster_info.sh

```

# Step 3. Deployement Bundle Generation for new node

1. This software works only on python3+ version.
2. Please update the [docker_setup/deploy/config.json](./config.json) file

      ```
        { 
            "docker_compose_file_version": "<docker_compose_file_version which is compose file version supported by deploying docker engine>",
            
            "exclude_services": [list of services which you want exclude for your deployement]
            "include_services": [list of services needed in final deployment docker-compose.yml]

        }        
      ```
    ***Note***: Please validate the json.
    Also Please ensure that you have updated the DOCKER_REGISTRY in [docker_setup/.env](../.env) file

3. Follow the below commands for EIS bundle Generation.
    ```
    sudo python3.6 generate_eis_bundle.py
    ```
4. EIS Bundle **eis_bundle.tar.gz** will be generated under the same directory.



**Note** :

    Default Values of Bundle Name is "eis_bundle.tar.gz" and the tag name is "eis_bundle"

    Using *-t* options you can give your custom tag name which will be used as same name for bundle generation.

    Default values for docker-compose yml path is "../docker-compose.yml"

   


For more help:
        ```
            sudo python3.6 generate_eis_bundle.py
        ```


    usage: generate_eis_bundle.py [-h] [-f COMPOSE_FILE_PATH] [-t BUNDLE_TAG_NAME]

    EIS Bundle Generator: This Utility helps to Generate the bundle to deploy EIS.

    optional arguments:
        -h, --help            show this help message and exit
        -t BUNDLE_TAG_NAME    Tag Name used for Bundle Generation (default:eis_bundle)

    

# Step 4a. EIS - Multinode deployment with TurtleCreek

Edge Insights Software (EIS) Deployment Bundle Generation for TurtleCreek Agent which will be deployed via Thingsboard/Telit portal.This Utility helps you to generate deployment bundle for EIS Software deployment via Thingsboard/Telit Portal

## Thingsboard/Telit - TurtleCreek:

EIS Deployment will be done only TurtleCreek Installed & Provisioned Device via Thingsboard/Telit portal. 

1. For TurtleCreek Agent installtion, provisioning and deploying the bundle generated using Step 3 above, please refer TurtleCreek repo Readme.

        
    While deploying EIS Software Stack via Telit-TurtleCreek. Visualizer UI will not pop up because of display not attached to docker container of Visualizer.

    To Overcome. goto IEdgeInsights/docker_setup 
    -->  Stop the running ia_visualizer container with it's name or container id using "docker ps" command.
         ```sh
         docker rm -f ia_visualizer
         ```

    -->  Start the ia_visualizer again. 
        ```sh
        docker-compose up ia_visualizer
        ```


# Step 4b. EIS - Multinode deployment without TurtleCreek.

Once EIS bundle is generated using Step 3, copy the bundle tar.gz to new node and follow below commands
> **NOTE**: Please make sure to copy and untar the above bundle on a secure location having root only access as it container secrets.

```
    $ sudo tar -xvf <eis_bundle gz generated in step 3>
    $ cd docker_setup
    $ docker-compose up

    eq. 
    
    $ sudo tar -xvf eis_bundle.tar.gz
    $ cd docker_setup
    $ docker-compose up

```
