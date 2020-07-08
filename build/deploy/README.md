# Multi-node EIS Provisioning & Deployment

The provisioning where ETCD server runs on one edge(master) node & rest other nodes are worker nodes which doesn't run ETCD server, instead all the worker nodes remotely connect to the ETCD server running on the Master edge node only.

Perform the below steps  to achieve provisioning & deployment on multiple nodes

[Step 1 Provision the Master node](#step-1-provision-the-master-node)

[Step 2 Set up Docker Registry URL then Build and Push Images](#step-2-set-up-docker-registry-url-then-build-and-push-images)


[Step 3 Deployment of EIS generated bundle on new node](#step-3-deployment-of-eis-generated-bundle-on-new-node)

[Step 4 EIS-Multi-node Provisioning](#step-4-eis-multi-node-provisioning)

[Step 5a EIS-Multinode deployment with TurtleCreek](#step-5a-eis-multinode-deployment-with-turtlecreek)

[Step 5b EIS-Multinode deployment without TurtleCreek](#step-5b-eis-multinode-deployment-without-turtlecreek)

[Step 6 Generate EIS bundle for CSL worker node provisioning](#step-6-generate-eis-bundle-for-csl-worker-node-provisioning)

# Step 1 Provision the Master node

**NOTE** Please follow the EIS Pre-requisites before Provisioning.
        [EIS Pre-requisites](../../README.md#eis-pre-requisites)

For running EIS in multi node, we have to identify one master node to run ETCD server on one node. For a master node, ETCD_NAME in [build/.env](../.env) must be set to `master`.

> **NOTE**: Master node is the primary administative node, which is used for following
> * Generating Required Certificates and Secrets.
> * Loading Initial ETCD values.
> * Generating bundles to provision new nodes.
> * Generating bundles to deploy EIS services on new nodes.
> * Master node should have the entire repo present.

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

# Step 3 Deployment of EIS generated bundle on new node

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
    ***Note***: Please validate the json.
    Also Please ensure that you have updated the DOCKER_REGISTRY in [build/.env](../.env) file

3. Follow the below commands for EIS bundle Generation.
    ```
    sudo python3.6 generate_eis_bundle.py
    ```
4. EIS Bundle **eis_bundle.tar.gz** will be generated under the same directory.

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


# Step 4 EIS-Multi-node Provisioning

##  step a : installing the provisioning bundle on worker node.

```
    # commands to be executed on master node.
    $ cd build/deploy
    $ sudo python3.6 generate_eis_bundle.py -p

    This will generate the 'eis_provisioning.tar.gz'.
    Do a manual copy of this bundle on worker node. And then follow below commands
    on worker node.

```

```
    # commands to be executed on worker node.
    $ tar -xvzf eis_provisioning.tar.gz
    $ cd eis_provisioning/provision/
    $ sudo ./provision_eis.sh
```

##  step b : installing the eis bundle on worker node.

```
    # commands to be executed on master node.
    $ sudo vim .env

    Now change the value of following fields in the .env of the worker node.

    ETCD_NAME=<any name other than `master`>
    ETCD_HOST=<IP address of master node>
    DOCKER_REGISTRY=<Docker registry details>
    $ cd deploy
    $ sudo python3.6 generate_eis_bundle.py

    This will generate the .tar.gz
```
Now this bundle can be used to deploy an eis on worker node.

# Step 5a EIS-Multinode deployment with TurtleCreek

Edge Insights Software (EIS) Deployment Bundle Generation for TurtleCreek Agent which will be deployed via Thingsboard/Telit/Azure portal.This Utility helps you to generate deployment bundle for EIS Software deployment via Thingsboard/Telit/Azure Portal

## Thingsboard/Telit/Azure - TurtleCreek:

EIS deployment will only be done the on node where TurtleCreek is installed & provisioned via Thingsboard/Telit/Azure portal.

1. Please follow above steps 1-4 for bundle generation for master/worker node.

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

# Step 5b EIS-Multinode deployment without TurtleCreek

Once EIS bundle is generated using Step 3, copy the bundle tar.gz to new node and follow below commands
> **NOTE**: Please make sure to copy and untar the above bundle on a secure location having root only access as it container secrets. Please follow above steps 1-4 for bundle generation for master/worker node.

```
    $ sudo tar -xvzf <eis_bundle gz generated in step 3>
    $ cd build
    $ docker-compose up -d

    eq.

    $ sudo tar -xvzf eis_bundle.tar.gz
    $ cd build
    $ docker-compose up -d

```

# Step 6 Generate EIS bundle for CSL worker node provisioning

* Set `PROVISION_MODE=csl` in `build/provision/.env` file.
* Generate EIS bundle for CSL worker node provisioning.
    ```sh
        sudo python3 ./generate_eis_bundle.py -t eis_csl_worker_node_setup
    ```
* Copy the `eis_csl_worker_node_setup.tar.gz` file to your provisioning machine.
    ```sh
        $ sudo scp <eis_csl_worker_node_setup.tar.gz> <any-directory_on-worker-Filesystem>
        $ sudo tar -xvf <eis_csl_worker_node_setup.tar.gz>
        $ cd <eis_csl_worker_node_setup>
    ```
* Provision the EIS in CSL Worker Client Node.
    ```sh
        $ cd provision
        $ sudo ./provision_eis.sh
    ```