# EII provision and deployment  


For deployment of EII, helm charts are provided for both provision and deployment.

> **Note**:
> Same procedure has to be followed for single or multi node.

## Pre requisites

For preparing the necessary files required for the provision and deployment, user needs to execute the build and provision steps on an Ubuntu 18.04 machine.
Follow the Docker pre-requisites, EII Pre-requisites, Provision EII and Build and Run EII mentioned in [README.md](../../README.md) on the Ubuntu dev machine.

  > **Note**:
 
  > Provisioning step can be done for a normal single node deployment without k8s. This step is executed only for getting the Certificates generated.

  > K8s installation on single or multi node should be done as pre-requisite to continue the following deployment.

  > For helm installation refer [helm website](https://helm.sh/docs/intro/install/).

## Update the helm charts directory

1. Copy the docker-compose.yml, eii_config.json into the eii-provision helm chart.
  ```sh
  $ cd [WORKDIR]/IEdgeInsights/build
  $ cp docker-compose.yml provision/config/eii_config.json helm-eii/eii-provision/
  ```
2. To generate only Certificates by provisioning. 
  ```sh
  $ cd [WORKDIR]/IEdgeInsights/build/provision
  $ sudo ./provision.sh ../docker-compose.yml --generate_certs
  ``` 
3. Copy the Certificates generated by provisioning process to the eii-provision helm chart.
  ```sh
  $ cd [WORKDIR]/IEdgeInsights/build
  $ sudo chmod -R 755 provision/Certificates/
  $ cp -a provision/Certificates/ helm-eii/eii-provision/
  ```
  > **Note**:
  > The Certificates/ directory contains sensitive information. So post the installation of eii-provision helm chart, it is recommended to delete the Certificates from it.

## Provision and deploy in the kubernetes node.

Copy the helm charts in helm-eii/ directory to the node.

1. Install provision helm chart
  ```sh
  $ cd [WORKDIR]/IEdgeInsights/build/helm-eii/
  $ helm install eii-provision eii-provision/
  ```

  Verify the pod is in running state:
  ```sh
  $ kubectl get pods

  NAME                       READY   STATUS    RESTARTS   AGE
  ia-etcd-58866469b9-dl66k   2/2     Running   0          8s
  ```
2. Run builder to copy templates file to eii-deploy/templates directory and generate consolidated values.yaml file for eii-services:
  ```sh
  $ python3 builder.py
  ```
> **Note**:
> If user wants to include only a certain number of services in the EII stack, he can opt to provide the -f or yml_file flag of builder.py to allow only the services provided in the yml file mentioned with the -f or yml_file.
> Example:
>```sh
>  $ python3 builder.py -f usecases/video-streaming.yml
>  ```
>
> If user wants to generate boiler plate config for multiple stream use cases, he can do so by using the -v or video_pipeline_instances flag of builder.
> Example:
>```sh
>  $ python3 builder.py -v 3 -f usecases/video-streaming.yml
>  ```
3. Install deploy helm chart
  ```sh
  $ cd [WORKDIR]/IEdgeInsights/build/helm-eii/
  $ helm install eii-deploy eii-deploy/
  ```

  Verify all the pod are running:
  ```sh
  $ kubectl get pods

  NAME                                          READY   STATUS    RESTARTS   AGE
  deployment-etcd-ui-6c7c6cd769-rwqm6           1/1     Running   0          11s
  deployment-video-analytics-546574f474-mt7wp   1/1     Running   0          11s
  deployment-video-ingestion-979dd8998-9mzkh    1/1     Running   0          11s
  deployment-webvisualizer-6c9d56694b-4qhnw     1/1     Running   0          11s
  ia-etcd-58866469b9-dl66k                      2/2     Running   0          2m26s
  ```

The EII is now successfully deployed.

## Provision and deploy mode in times switching between dev and prodmode 

1. Set the DEV_MODE as "true/false" in  [.env](../.env) depending on dev or prod mode.

2. Run builder to copy templates file to eii-deploy/templates directory and generate consolidated values.yaml file for eii-services:
  ```sh
  $ python3 builder.py
  ```
2. Remove the etcd storage directory
  ```sh
  $sudo rm -rf /opt/intel/eii/data/*
  ```

Do helm install of provision and deploy charts as per previous section.

> **Note**:
> Please wait for all the pods terminated successfully, In times of re-deploy helm chart for eii-provision and eii-deploy

## Steps to enable Accelarators
>**Note**:
> `nodeSelector` is the simplest recommended form of node selection constraint.
> `nodeSelector` is a field of PodSpec. It specifies a map of key-value pairs. For the pod to be eligible to run on a node, the node must have each of the indicated key-value pairs as labels (it can have additional labels as well). The most common usage is one key-value pair.

1. Setting the `label` for a particular node
  ```sh
  $ kubectl label nodes <node-name> <label-key>=<label-value>
  ```

2. For HDDL/NCS2 dependenecies follow the steps for setting `labels`.

  * For HDDL

  ```sh
  $ kubectl label nodes <node-name> hddl=true
  ```

  * For NCS2

  ```sh
  kubectl label nodes <node-name> ncs2=true
  ```

>**Note**: Here the node-name is your worker node machine hostname

3. Open the `[WORKDIR]/IEdgeInsights/VideoIngestion/helm/values.yaml` or `[WORKDIR]/IEdgeInsights/VideoAnalytics/helm/values.yaml` file.

4. Based on your workload preference. Add hddl or ncs2 to accelerator in values.yaml of video-ingestion or video-analytics. 

  * For HDDL

  ```yml
  config:
    video_ingestion:
       .
       .
       .
      accelerator: "hddl"
      .
      .
      .
  ```
  * For NCS2

  ```yml
  config:
    video_ingestion:
       .
       .
       .
      accelerator: "ncs2"
      .
      .
      .
  ```

5. set device as "MYRIAD" in case of ncs2 and as HDDL in case of hddl in the [VA config](../../VideoAnalytics/config.json)
  
  * In case of ncs2.

  ```yml
  "udfs": [{
    .
    .
    .
    "device": "MYRIAD"
    }]
  ```

  * In case of hddl.

  ```yml
  "udfs": [{
    .
    .
    .
    "device": "HDDL"
    }]
  ```  

6. Run the `[WORKDIR]/IEdgeInsights/build/builder.py` for generating latest consolidated `deploy` yml file based on your `nodeSelector` changes set in the respective Modules.

  ```sh
  cd [WORKDIR]/IEdgeInsights/build/
  python3 builder.py
  ```

7. Follow the [Deployment Steps](##Provision-and-deploy-in-the-kubernetes-node)

8. Verify the respecitve workloads are running based on the `nodeSelector` constraints.

## Steps for Enabling GiGE Camera with helm

**Note:** For more information on `Multus` please refer this git https://github.com/intel/multus-cni
 Skip installing multus if it is already installed.

1. Prequisites
  For enabling gige camera with helm. Helm pod networks should be enabled `Multus` Network Interface
  to attach host system network interface access by the pods for connected camera access.

  **Note**: Please follow the below steps & make sure `dhcp daemon` is running fine.If there is an error on `macvlan` container creation on accessing the socket or if socket was not running. Please execute the below steps again

  ```sh
  $ sudo rm -f /run/cni/dhcp.sock
  $ cd /opt/cni/bin
  $ sudo ./dhcp daemon
  ```
 
  ### Setting up Multus CNI and Enabling it.
  * Multus CNI is a container network interface (CNI) plugin for Kubernetes that enables attaching multiple network interfaces to pods. Typically, in Kubernetes each pod only has one network interface (apart from a loopback) -- with Multus you can create a multi-homed pod that has multiple interfaces. This is accomplished by Multus acting as a "meta-plugin", a CNI plugin that can call multiple other CNI plugins.

  1. Get the name of the `ethernet` interface in which gige camera & host system connected
    **Note**: Identify the network interface name by following command

  ```sh
  $ ifconfig
  ```

  2. Execute the Following Script with Identified `ethernet` interface name as Argument for `Multus Network Setup`
    **Note:** Pass the `interface` name without `quotes`

  ```sh
  $ cd [WORKDIR]/IEdgeInsights/build/helm-eii/gige_setup
  $ sudo -E sh ./multus_setup.sh <interface_name>
  ```

  3. Set gige_camera to true in values.yaml

  ```sh
  $ vi [WORKDIR]/IEdgeInsights/VideoIngestion/helm/values.yaml
  .
  .
  .
  gige_camera: true
  .
  .
  .      
  ```

  4. Follow the [Deployment Steps](##Provision-and-deploy-in-the-kubernetes-node)
     
  5. Verify `pod`ip & `host` ip are same as per Configured `Ethernet` interface by using below command.

  ```sh
  $ kubectl -n eii exec -it <pod_name> -- ip -d address      
  ```

## Accessing Web Visualizer and EtcdUI
  Environment EtcdUI & WebVisualizer will be running in Following ports. 
  * EtcdUI
     * `https://master-nodeip:30010/`
  * WebVisualizer
     * PROD Mode --  `https://master-nodeip:30007/`
     * DEV Mode -- `http://master-nodeip:30009/`


