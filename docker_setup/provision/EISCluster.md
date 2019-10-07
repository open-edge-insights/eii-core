#  EIS Clustering

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

## Please execute Step 1 and Step 2 below to Add New Node to EIS Cluster.

> **NOTE**:  
> * Steps 1 to be executed on `master` node'.
> * Step 2 to be executed on 'new node' which will join EIS cluster.
> * Please execute below commands in directory [repo]/docker_setup/provision.
> * For environment under proxy, please make sure IP addresses or IP range for all nodes in a cluster are appended to no_proxy in [docker_setup/.env](../.env) for `master` node.



### Step 1). To be performed on Master node:

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
* Also it will create a `<NodeName>_provision.tar.gz` file in docker_setup/provision/ folder. This file needs to be copied to docker_setup/provision/ folder of the New Node.

```

  ./scp <NodeName>_provision.tar.gz user@<IP Address of New Node>:<EIS repo Path>/docker_setup/provision/
  
  e.q. scp ./node2_provision.tar.gz user@10.223.109.170:~/IEdgeInsights/docker_setup/provision/

```

### Step 2). To be performed on New node:


> **NOTE** EIS should not be running or provisioned on new node.  Before provisioning, below steps needs to be performed.

 * ` <NodeName>_provision.tar.gz` is copied to docker_setup/provision/dep folder as per step 1 above from Existing Node.
 
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
