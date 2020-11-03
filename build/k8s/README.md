# EIS Orchestration using k8s (a.k.a) Kuberenetes Orchestrator

EIS Orchestration using k8s Orchestrator.

1. [k8s Setup](#k8s-setup)

2. [Provisioning EIS with k8s](#provisioning-eis-with-k8s)

3. [Deploying EIS Application with K8s](#deploying-eis-application-with-k8s)

4. [Steps for Enabling Basler Camera with k8s](#steps-for-enabling-basler-camera-with-k8s)

5. [Steps for Multinode Deployment](#steps-for-multinode-deployment)

6. [Steps to enable Accelarators](#steps-to-enable-accelarators)
## K8s Setup

  > **Note**:
  > Please follow the standard proxy settings if your installation machines are behind proxy environment.
  > For orchestrator deployment in a proxy environment, make sure all communicating machines IP addresses
  >are in `no_proxy` list both in `docker` & `system` environment files as follows.
  > 1.  /etc/environment
  > 2.  /etc/systemd/system/docker.service.d/http-proxy.conf
  > 3.  ~/.docker/config.json

  > `K8s installation should be done as pre-requisite to continue the following deployment`
## Provisioning EIS with k8s
  > 1. EIS Deployment with k8s can be done in **PROD** & **DEV** mode.
  > 2. For running EIS in multi node, we have to identify one master node.For a master node `ETCD_NAME` in [build/.env](../.env) must be set to `master`.
  > 3. Please follow the [EIS Pre-requisites](../../README.md#eis-pre-requisites) before k8s Provisioning.

  * Please Update following Environment Variables in [build/.env](../../build/.env)
    * HOST_IP           =     [masternodeip]
    * ETCD_HOST         =     [masternodeip]
    * ETCD_CLIENT_PORT  =     8379
    * ETCD_PEER_PORT    =     8380

  * Please Update following Environment Variables in [build/provision/.env](../../build/provision/.env)
    * EIS_LOCAL_PATH    =     [EIS Source path]
    * PROVISION_MODE    =     k8s

### Provisioning `EIS` with `k8s` in `DEV` mode
> **Note:** `k8s_eis_deploy.yml` file be generated in `build/k8s` directory by `eisbuilder`.
> Once 'DEV_MODE` is updated. We should re-run `eisbuilder` to generate
>updated `build/k8s/k8s_eis_deploy.yml` file.
>   Please follow the [EIS Pre-requisites](../../README.md#eis-pre-requisites)

  * Please Update `DEV_MODE=true` in [build/.env](../../build/.env) file.

  * Provision EIS with K8s. 
    ```sh
    $ sudo ./provision_eis.sh <path_to_eis_docker_compose_file>

    eq. $ sudo ./provision_eis.sh ../docker-compose.yml
    ```
### Provisioning `EIS` with `k8s` in `PROD` mode
> **Note:** `k8s_eis_deploy.yml` file be generated in `build/k8s` directory by `eisbuilder`.
> Once 'DEV_MODE` is updated. We should re-run `eisbuilder` to generate
>updated `build/k8s/k8s_eis_deploy.yml` file.
> Please follow the [EIS Pre-requisites](../../README.md#eis-pre-requisites)
  
  * Please Update `DEV_MODE=false` in [build/.env](../../build/.env) file.

  * Provision EIS with K8s.
    ```sh
    $ sudo ./provision_eis.sh <path_to_eis_docker_compose_file>

    eq. $ sudo ./provision_eis.sh ../docker-compose.yml
    ```
## Steps for Multinode Deployment
 > 1. `K8s Master node & Worker(s) node Setup should be done as pre-requisite to continue the following deployment`
 > 2. `This step is not required for **Single Node** deployment`
  * Master Node provisoning: Follow steps mentioned in below links.
    1. [Change for Master Provision for Multinode Deployment](###change-for-master-provision-for-multinode-deployment)
    2. [Provisioning EIS with k8s](#provisioning-eis-with-k8s)
  * Worker Node provisoning: Follow steps mentioned in below links.
     1. [EIS Provision Bundle creation for k8s worker node](#EIS-Provision-Bundle-creation-for-k8s-worker-node)
     2. [EIS k8s Worker Node provisioning for MultiNode deployment](#EIS-k8s-Worker-Node-provisioning-for-MultiNode-deployment)

### Change for Master Provision for Multinode Deployment.
 * Get the master node name by running below command.
    ```sh
    $ kubectl get nodes
    ```
      It will display NAME along with ROLES. Copy the complete `name` of Node with ROLES as `master`.
      ```
      E.g.
      $ kubectl get nodes
      NAME     STATUS          ROLES    AGE   VERSION
      coolm    Ready           master   67d   v1.18.6
      ```
      For example, copied `coolm` from above.
  * Add copied master node name(e.g. coolm) with label `nodeName under spec of Pod ia-etcd as nodeName:` in below yaml based on DEV mode.
    * If DEV mode =TRUE  [build/provision/dep/k8s/etcd_devmode.yml](../../build/provision/dep/k8s/etcd_devmode.yml)
    * IF DEV mode=FALSE  [build/provision/dep/k8s/etcd_prodmode.yml](../../build/provision/dep/k8s/etcd_prodmode.yml)

    This will enable etcd to run always on master node.

    For example, added  `nodeName:coolm` in etcd_devmode.yaml or etcd_prodmode.yaml which was copied above.
    ```
     E.g.
     spec:
       restartPolicy: OnFailure
       nodeName: coolm
     ```
### EIS Provision Bundle creation for k8s worker node.
> 1. This step is not required for **Single Node** deployment
> 2. Provisioning bundle will be generated on k8s `Master node`.
> 3. Make Sure ETCD_NAME=[any name other than `master`] in [build/.env](../build/.env) of Master node.
> 4. Run all steps on Master node.
  * Set `PROVISION_MODE=k8s` in [build/provision/.env](../build/provision/.env) file.
  * Go to `$[WORK_DIR]/IEdgeInsights/build/deploy` directory
  * Generate Provisioning bundle for k8s worker node provisioning.

    ```sh
      $ sudo python3 generate_eis_bundle.py -p
    ```
### EIS `k8s Worker Node` provisioning for MultiNode deployment
>**Note**:
> 1. This should be executed in k8s worker nodes in ***Multi node
>    scenario*** only.
> 2. This step is not required for **Single Node** deployment
> 3. Provisioning bundle generated on k8s Master node will be copied to worker node.
  * Copy the `eis_provisioning.tar.gz` file from k8s master node to worker node(s).

      ```sh
          $ sudo scp <eis_provisioning.tar.gz> <any-directory_on-worker-Filesystem>
          $ sudo tar -xvf <eis_provisioning.tar.gz>
          $ cd <eis_provisioning>
      ```
  * Provision the EIS in k8s Worker Node.
      ```sh
          $ cd provision
          $ sudo ./provision_eis.sh
      ```
## Steps for Enabling Basler Camera with k8s
   > **Note:** For more information on `Multus` please refer this git https://github.com/intel/multus-cni
   > Skip installing multus if it is already installed and execute others.

   *  Prequisites
      For enabling basler camera with k8s. K8s pod networks should be enabled `Multus` Network Interface
      to attach host system network interface access by the pods for connected camera access.

      >**Note**: Please follow the below steps & make sure `dhcp daemon` is running fine.If there is an error on `macvlan` container creation on accessing the socket or if socket was not running. Please execute the below steps again
      > ```sh
      >   $ sudo rm -f /run/cni/dhcp.sock
      >   $ cd /opt/cni/bin
      >   $ sudo ./dhcp daemon
      > ```      

      ### Setting up Multus CNI and Enabling it.
      * Multus CNI is a container network interface (CNI) plugin for Kubernetes that enables attaching multiple network interfaces to pods. Typically, in Kubernetes each pod only has one network interface (apart from a loopback) -- with Multus you can create a multi-homed pod that has multiple interfaces. This is accomplished by Multus acting as a "meta-plugin", a CNI plugin that can call multiple other CNI plugins.

      * Get the name of the `ethernet` interface in which basler camera & host system connected
        >**Note**: Identify the network interface name by following command
          ```sh
          $ ifconfig
          ```
      * Execute the Following Script with Identified `ethernet` interface name as Argument for `Multus Network Setup`
        > **Note:** Pass the `interface` name without `quotes`
        ```sh
        $ sudo -E sh ./k8s_multus_setup.sh <interface_name>
        ```

      * Update `macvlan` network config `VideoIngestion` deployment `yml` file in `VideoIngestion/k8s-service.yml`

      ```yml
            annotations:
                k8s.v1.cni.cncf.io/networks: macvlan-conf
      ```

      Eg.
      ```yml
        apiVersion: apps/v1
        kind: Deployment
        metadata:
          labels:
            app: video
          name: deployment-video-ingestion
          namespace: eis
          annotations:
            k8s.v1.cni.cncf.io/networks: macvlan-conf
      ```

      * Follow the [Deployment Steps](#deploying-eis-application-with-k8s)
     
      * Verify `pod`ip & `host` ip are same as per Configured `Ethernet` interface by using below command.

          ```sh
            $ kubectl exec -it <pod_name> -- ip -d address
          ```

## Deploying EIS Application with K8s
  > **Note:** `eis-k8s-deploy.yml` file be generated in `build/k8s` directory by `eisbuilder`.

  * Goto `build/k8s` directory.
    ```sh
    $ cd build/k8s
    ```
  * Deploy `k8s` using `kubectl` utility by following command
    ```sh
    $  kubectl apply -f ./eis-k8s-deploy.yml
    ```
  * Make sure the pods are `Running` fine as per deploy yml file.
    ```sh
    $  kubectl -n eis get pods
    ```
   
## Steps to enable Accelarators
  >**Note**:
  > `nodeSelector` is the simplest recommended form of node selection constraint.
  > `nodeSelector` is a field of PodSpec. It specifies a map of key-value pairs. For the pod to be eligible to run on a node, the node must have each of the indicated key-value pairs as labels (it can have additional labels as well). The most common usage is one key-value pair.

  * Setting the `label` for a particular node
    ```sh
    $ kubectl label nodes <node-name> <label-key>=<label-value>
    ```

  * For HDDL/NCS2 dependenecies follow the steps for setting `labels`.
    * For HDDL
        ```sh
        kubectl label nodes <node-name> hddl=true
        ```
    * For NCS2
        ```sh
        kubectl label nodes <node-name> ncs2=true
        ```
    **Note** Here the node-name is your worker node machine hostname

  * Updating the `yml` File.

    * Open the `build/k8s/eis-k8s-deploy.yml` file.

    * Based on your
      workload preference. Add a `nodeSelector` field to your `pod` configuration for `VideoIngestion` or `VideoAnalytics` Module(s) `k8s-service.yml` yml file. 
        ```sh
        $   spec:
              nodeSelector:
                <key>: <value>
        ```
      * For HDDL
          ```sh
          .
          .
          .
           spec:
              nodeSelector:
              hddl: "true"
              containers:
                - name: ia-video-ingestion
                image: ia_video_ingestion:2.4
          .
          .
          .
          ```
      * For NCS2
          ```sh
          .
          .
          .
           spec:
              nodeSelector:
              ncs2: "true"
              containers:
                - name: ia-video-ingestion
                image: ia_video_ingestion:2.4
          .
          .
          .
          ```
    * Run the `build/eis_builder.py` for generating latest consolidated `deploy` yml file based on your `nodeSelector` changes set in the respective Modules `k8s-service.yml` files.
     ```sh
        cd build/
        python3 eis_builder.py
     ```

    * Deploy the `latest` generated `yml` file
    ```sh
        cd build/k8s/
        kubectl apply -f ./eis-k8s-deploy.yml
    ```

    * Verify the respecitve workloads are running based on the `nodeSelector` constraints.