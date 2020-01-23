# Multi-node EIS Provisioning & Deployment

1. [Multi-node EIS provisioning and deployment with ETCD Clustering](#multi-node-eis-provisioning-and-deployment-with-etcd-clustering)

2. [Multi-node EIS provisioning and deployment without ETCD Clustering](#multi-node-eis-provisioning-and-deployment-without-etcd-clustering)


# Multi-node EIS provisioning and deployment with ETCD Clustering
This method uses the standard ETCD Clustering concept to perform provisioning on multiple nodes. This method is efficient when there are many nodes in the use case.
Perform the below steps to achieve provisioning & deployment on multiple nodes using ETCD Clustering:

[Step 1 Provision the Master node](#step-1-provision-the-master-node)

[Step 2 Set up Docker Registry URL then Build and Push Images](#step-2-set-up-docker-registry-url-then-build-and-push-images)

[ Step 3 Add New node to EIS cluster](#step-3-add-new-node-to-eis-cluster)

[Step 4 Deployement Bundle Generation for new node](#step-4-deployement-bundle-generation-for-new-node)

[ Step 6a EIS-Multinode deployment with TurtleCreek](#step-6a-eis-multinode-deployment-with-turtlecreek)

[Step 6b EIS-Multinode deployment without TurtleCreek](#step-6b-eis-multinode-deployment-without-turtlecreek)

# Multi-node EIS provisioning and deployment without ETCD Clustering
This is called lightweight provisioning because, unlike ETCD clustering where each edge node runs an ETCD server, lightweight provisioning involves only 1 master node & rest other nodes are slave nodes which themselves doesn't run ETCD server. Instead they remotely connect to the ETCD server running on the Master edge node.
Hence, no ETCD Clustering concept is used. So no RAFT protocol overhead is involved here, which speedens up the virtual clustering.
This lightweight virtual clustering is ideal for a small set of 1 MASTER & 3 to 5 SLAVE NODES.

Perform the below steps  to achieve provisioning & deployment on multiple nodes **without** using ETCD Clustering.

[Step 1 Provision the Master node](#step-1-provision-the-master-node)

[Step 2 Set up Docker Registry URL then Build and Push Images](#step-2-set-up-docker-registry-url-then-build-and-push-images)

[Step 4 Deployement Bundle Generation for new node](#step-4-deployement-bundle-generation-for-new-node)

[Step 5 EIS-Multi-node Provisioning without ETCD Clustering](#step-5-eis-multi-node-provisioning-without-etcd-clustering)

[Step 6b EIS-Multinode deployment without TurtleCreek](#step-6b-eis-multinode-deployment-without-turtlecreek)



# Step 1 Provision the Master node
Provision the Master node using the below command.

        ```
        $ cd [EIS_repo]/build/provision
        $ sudo ./provision_eis.sh <path_to_eis_docker_compose_file>

        eq. $ sudo ./provision_eis.sh ../docker-compose.yml

        ```
    This creates the ETCD server (Container ia_etcd) on the master edge node.

> **NOTE**: In case of Multi-node provisioning without ETCD clustering, we do not create anymore etcd servers on any slave other node. All the slave edge nodes connect to etcd server running on master edge node only.

# Step 2 Set up Docker Registry URL then Build and Push Images
EIS Deployment on multiple node must be done using a docker registry.

Follow below steps:

* Please update docker registry url in DOCKER_REGISTRY variable in [build/.env](build/.env). Please use full registry URL with a traliling /

* Building EIS images and pushing the same to docker registry.

      ```sh
      docker-compose build
      docker-compose push

      ```

> **NOTE**: Please copy only build folder on node on which EIS is being launched through docker-registry and make sure all build dependencies are commented/removed form docker compose file before executing below commands.
> **NOTE**: Above commenting/removing build dependencies is not required if entire EIS repo is present on the node on which EIS is being launched through registry.


# Step 3 Add New node to EIS cluster

EIS uses ETCD for clustering. When EIS is deployed on a cluster having multiple node, ETCD will make sure all EIS services configurations and secrets are distributed across all nodes of the cluster.

By default EIS starts with single node cluster.

For running EIS in multi node cluster mode, we have to indetify one master node. For a master node, ETCD_NAME in [build/.env](../.env) must be set to `master`.

> **NOTE**: Master node is the primary administative node, which is used for following
> * Generating Required Certificates and Secrets.
> * Loading Initial ETCD values.
> * Adding new node to ETCD Cluster.
> * Building and Pushing Docker images to registry.
> * Generating bundles to provision new nodes.
> * Generating bundles to deploy EIS services on new nodes.
> * Master node should have the entire repo present.

## Please execute Step 3.1 and Step 3.2 below to Add New Node to EIS Cluster.

> **NOTE**:
> * Steps 'a' to be executed on `master` node'.
> * Step 'b' to be executed on 'new node' which will join EIS cluster.
> * Please execute below commands in directory [repo]/build/provision.
> * For environment under proxy, please make sure IP addresses or IP range for all nodes in a cluster are appended to no_proxy in [build/.env](../.env) for `master` node.



### Step 3.1 To be performed on Master node:

> **NOTE**: if EIS provisioning is not done on this node, please perform EIS provisioning first using below command. If EIS is already provisioned and running, then below step is not required.

        ```
        $ sudo ./provision_eis.sh <path_to_eis_docker_compose_file>

        eq. $ sudo ./provision_eis.sh ../docker-compose.yml

        ```
* <b>Please execute below command in working directory build/provision to join new node to cluster.</b>

```

 sudo ./etcd_cluster_add.sh <NodeNAme> <IP Address of New Node>

  e.q. sudo ./etcd_cluster_add.sh node2 10.223.109.170

```

> **NOTE**: Please note that `< NodeName >` and `< IP Address of the New Node >` above, must not be part of existing node list of the cluster. (Please use command `./etcd_cluster_info.sh` to see existing node list of the cluster.)

* Once the above command is executed successfully, a message should come that node is added to cluster.
* Also it will create a `<NodeName>_provision.tar.gz` file in build/provision/ folder. This file needs to be copied to New Node.

> **NOTE**: Please make sure to copy and untar the above bundle on a secure location having root only access as it container secrets.

```

  ./scp <NodeName>_provision.tar.gz user@<IP Address of New Node>:~

  e.q. scp ./node2_provision.tar.gz user@10.223.109.170:~

```

### Step 3.2 To be performed on New node:


> **NOTE** EIS should not be running or provisioned on new node.  Before provisioning, below steps needs to be performed.

 * ` <NodeName>_provision.tar.gz` is copied as per step 3.1 above from Existing Node.

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

# Step 4 Deployement Bundle Generation for new node

>Note: This deployment bundle is used as both provisioning bundle & deployment bundle in case of "Multi-node provisioning without etcd clustering" (aka Lightweight provisioning).

1. This software works only on python3+ version.
2. Please update the [build/deploy/config.json](./config.json) file

      ```
        {
            "docker_compose_file_version": "<docker_compose_file_version which is compose file version supported by deploying docker engine>",

            "exclude_services": [list of services which you want exclude for your deployement]
            "include_services": [list of services needed in final deployment docker-compose.yml]

        }
      ```
    ***Note***: Please validate the json.
    Also Please ensure that you have updated the DOCKER_REGISTRY in [build/.env](../.env) file

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


# Step 5 EIS-Multi-node Provisioning without ETCD Clustering

A.K.A Lightweight provisioning on multiple nodes, this step is needed only if we are provisioning the slave node with EIS readiness for the first time.

Unzip this bundle to any folder on the slave node file system.
```
    $ sudo scp <eis_bundle.tar.gz> <any-directory_on-slave-Filesystem>
    $sudo tar -xvf <eis_bundle.tar.gz>
    $ls
    $cd <eis_bundle>
    $sudo vim .env

    Now change the value of following field in the .env of the slave node.

    ETCD_HOST=<IP address of master node>

    $cd provision
    $ sudo ./slave_provision_noetcd.sh

```
Now provisioning is done on the slave node without using ETCD Server locally on slave node. Instead all ETCD server calls connect to the remote Master's ETCD server.


# Step 6a EIS-Multinode deployment with TurtleCreek

Edge Insights Software (EIS) Deployment Bundle Generation for TurtleCreek Agent which will be deployed via Thingsboard/Telit/Azure portal.This Utility helps you to generate deployment bundle for EIS Software deployment via Thingsboard/Telit/Azure Portal

## Thingsboard/Telit/Azure - TurtleCreek:

EIS Deployment will be done only TurtleCreek Installed & Provisioned Device via Thingsboard/Telit/Azure portal.

1. For TurtleCreek Agent installtion, provisioning and deploying the bundle generated using Step 4 above, please refer TurtleCreek repo Readme.


    While deploying EIS Software Stack via Telit-TurtleCreek. Visualizer UI will not pop up because of display not attached to docker container of Visualizer.

    To Overcome. goto IEdgeInsights/build
    -->  Stop the running ia_visualizer container with it's name or container id using "docker ps" command.
         ```sh
         docker rm -f ia_visualizer
         ```

    -->  Start the ia_visualizer again.
        ```sh
        docker-compose up ia_visualizer
        ```


# Step 6b EIS-Multinode deployment without TurtleCreek

> Common Step for Deployment using ETCD CLustering & without ETCD Clustering

Once EIS bundle is generated using Step 4, copy the bundle tar.gz to new node and follow below commands
> **NOTE**: Please make sure to copy and untar the above bundle on a secure location having root only access as it container secrets.

```
    $ sudo tar -xvf <eis_bundle gz generated in step 4>
    $ cd build
    $ docker-compose up -d

    eq.

    $ sudo tar -xvf eis_bundle.tar.gz
    $ cd build
    $ docker-compose up -d

```
