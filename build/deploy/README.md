# Multi-node EIS Provisioning & Deployment

Perform the below steps  to achieve provisioning & deployment on multiple nodes

[Step 1 Provision the Master node](#step-1-provision-the-master-node)

[Step 2 Set up Docker Registry URL then Build and Push Images](#step-2-set-up-docker-registry-url-then-build-and-push-images)

[Step 3 Choosing the EIS services to run on worker node](#step-3-choosing-the-eis-services-to-run-on-worker-node)

[Step 4 Creating the required environment on worker node](#step-4-creating-the-required-environment-on-worker-node)

[Step 5 Creating eis bundle for worker node](#step-5-creating-eis-bundle-for-worker-node)

[Step 6 EIS-Multinode deployment with TurtleCreek](#step-6-eis-multinode-deployment-with-turtlecreek)

[Step 7 EIS-Multinode deployment without TurtleCreek](#step-7-eis-multinode-deployment-without-turtlecreek)

# Step 1 Provision the Master node

> **Pre-requisite**:
> Please follow the EIS Pre-requisites before Provisioning.
> [EIS Pre-requisites](../../README.md#eis-pre-requisites)

> **NOTE**:
> * EIS services can run on master as well as worker nodes
> * Master node should have the entire repo/source code present
> * Master node is the primary administative node and has following attributes:
>   1. Generating required certificates and secrets.
>   2. Loading Initial ETCD values.
>   3. Generating bundles to provision new nodes.
>   4. Generating bundles to deploy EIS services on new/worker nodes.


For running EIS in multi node, we have to identify one node to run ETCD server (this node is called as `master` node). For a master node, ETCD_NAME in [build/.env](../.env) must be set to `master`. Rest other nodes are `Worker` nodes which doesn't run ETCD server, instead all the worker nodes remotely connect to the ETCD server running on the `Master` node only.

Provision the Master node using the below command,

        ```
        $ cd [WORK_DIR]/IEdgeInsights/build/provision
        $ sudo ./provision_eis.sh <path_to_eis_docker_compose_file>

        eq. $ sudo ./provision_eis.sh ../docker-compose.yml

        ```
    This creates the ETCD server (Container ia_etcd) on the master edge node.

# Step 2 Set up Docker Registry URL then Build and Push Images
EIS Deployment on multiple node must be done using a docker registry.

Follow below steps:

* Please update docker registry url in DOCKER_REGISTRY variable in  [build/.env](../.env) on any node(master/worker). Please use full registry URL with a traliling /

* Building EIS images and pushing the same to docker registry.

      ```sh
      docker-compose build
      docker-compose push

      ```

> **NOTE**: Please copy only build folder on node on which EIS is being launched through docker-registry and make sure all build dependencies are commented/removed form docker compose file before executing below commands.
> **NOTE**: Above commenting/removing build dependencies is not required if entire EIS repo is present on the node on which EIS is being launched through registry.

# Step 3 Choosing the EIS services to run on worker node.

>Note: This deployment bundle generated can be used to provision and also to deploy EIS stack on new node

1. This software works only on python3+ version.
2. Please update the [config.json](./config.json) file

      ```
        {
            "docker_compose_file_version": "<docker_compose_file_version which is compose file version supported by deploying docker engine>",

            "exclude_services": [list of services which you want exclude for your deployement]
            "include_services": [list of services needed in final deployment docker-compose.yml]

        }
      ```
    ***Note***: 
    > 1. Please validate the json.

    > 2. Please ensure that you have updated the DOCKER_REGISTRY in [build/.env](../.env) file

    > 3. Ensure "ia_etcd_ui" service is not added as part of "include_services" in [config.json](./config.json). EtcdUI would run only in master node and it can be accessed from worker nodes at: http://[master_node_ip]:7070/etcdkeeper.<br/>
    > Follow [EtcdUI/README](../../EtcdUI/README.md) for more inofrmation.
# Step 4 Creating the required environment on worker node

```
    # commands to be executed on master node.
    $ sudo vim .env

    Now change the value of following fields in the .env of the worker node.

    ETCD_HOST=<IP address of master node>
    DOCKER_REGISTRY=<Docker registry details>
    $ cd build/deploy
    $ sudo python3.6 generate_eis_bundle.py -p

    This will generate the 'eis_provisioning.tar.gz'.
    This bundle has only artifact required to create environment like user/directory/..
    Do a manual copy of this bundle on worker node. And then follow below commands
    on worker node.
```

```
    # commands to be executed on worker node.
    $ tar -xvzf eis_provisioning.tar.gz
    $ cd eis_provisioning/provision/
    $ sudo ./provision_eis.sh
```

# Step 5 Creating eis bundle for worker node
> **NOTE**: Before proceeding this step, please make sure, you have followed steps 1-4.

```
    # commands to be executed on master node.
    $ sudo vim .env

    Now change the value of following fields in the .env of the worker node.

    ETCD_HOST=<IP address of master node>
    DOCKER_REGISTRY=<Docker registry details>
    $ cd deploy
    $ sudo python3.6 generate_eis_bundle.py

    This will generate the .tar.gz which has all the required artifacts by which eis services 
    can be started on worker node.
```
***Note***:

    1. Default Values of Bundle Name is "eis_bundle.tar.gz" and the tag name is "eis_bundle"

    2. Using *-t* options you can give your custom tag name which will be used as same name for bundle generation.

    3. Default values for docker-compose yml path is "../docker-compose.yml"

For more help:

    $sudo python3.6 generate_eis_bundle.py


    usage: generate_eis_bundle.py [-h] [-f COMPOSE_FILE_PATH] [-t BUNDLE_TAG_NAME]

    EIS Bundle Generator: This Utility helps to Generate the bundle to deploy EIS.

    optional arguments:
        -h, --help            show this help message and exit
        -t BUNDLE_TAG_NAME    Tag Name used for Bundle Generation (default:eis_bundle)


Now this bundle can be used to deploy an eis on worker node. This bundle has all the required artifacts to start the eis
services on worker node.

# Step 6 EIS-Multinode deployment with TurtleCreek

## Thingsboard/Telit/Azure - TurtleCreek:

EIS deployment will only be done on the node where TurtleCreek is installed & provisioned via Thingsboard/Telit/Azure portal.

1. Before proceeding this step, please make sure, you have followed steps 1-5.

2. For TurtleCreek Agent installation and deployment through TurtleCreek, please refer TurtleCreek repo README.

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

# Step 7 EIS-Multinode deployment without TurtleCreek

> **NOTE**: Before proceeding this step, please make sure, you have followed steps 1-5. Please make sure to copy and untar the above bundle on a secure location having root only access as it contains secrets.

```
    $ sudo tar -xvzf <eis_bundle gz generated in step 3>
    $ cd eis_bundle
    $ docker-compose up -d

    eq.

    $ sudo tar -xvzf eis_bundle.tar.gz
    $ cd eis_bundle
    $ docker-compose up -d

```
