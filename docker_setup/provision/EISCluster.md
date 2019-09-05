#  EIS Clustering

EIS uses ETCD for clustering. When EIS is deployed on a cluster having multiple node, ETCD will make sure all EIS services configurations and secrets are distributed across all nodes of the cluster.

By default EIS starts with single node cluster. New nodes can be added to EIS cluster using following 2 step process.

> **NOTE**: 
> * Steps 1 to be executed on 'existing node' running EIS cluster.
> * Step 2 to be executed on 'new node' which will join EIS cluster.
> * Please execute below commands in directory [repo]/docker_setup/provision.
> * For environment under proxy, please make sure IP addresses or IP range for all nodes in a cluster are appended to no_proxy in [docker_setup/.env](../.env) for all nodes of cluster.



### Step 1). To be performed on Existing node:

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
* Also it will create a `<NodeName>.env` file in docker_setup/provision/dep folder. This file needs to be copied to docker_setup/provision/dep folder of the New Node.

```

  ./scp <NodeNAme>.env user@<IP Address of New Node>:<EIS repo Path>/docker_setup/provision/dep
  
  e.q. scp ./dep/node2.env user@10.223.109.170:~/IEdgeInsights/docker_setup/provision/dep

```

### Step 2). To be performed on New node:


> **NOTE** EIS should not be running or provisioned on new node.  Before provisioning, below steps needs to be performed.

 * `<NodeName>.env` is copied to docker_setup/provision/dep folder as per step 1 above from Existing Node.
 * Update ETCD_NAME in docker_setup/.env with `NodeName` used in command in step 1 on Existing Node.
 * Make sure to update [docker_setup/docker-compose.yml](../docker-compose.yml) with only the required services to be deployed on new node.
 * Make sure to update [docker_setup/provision/config/etcd_pre_load.json](config/etcd_pre_load.json) on new node with only the required keys for 
services deployed on new node.


 * Once above steps are performed, Provision EIS using below command and it will join new node to existing cluster and do EIS provisioning on new node.

```
    $ sudo ./provision_eis.sh <path_to_eis_docker_compose_file>

    eq. $ sudo ./provision_eis.sh ../docker-compose.yml

```
* Once provisioning is done, please verify that New Node is added to cluster by below command.
```
    $  ./etcd_cluster_info.sh

```
