# EIS Orchestration using k8s (a.k.a) Kuberenetes Orchestrator

EIS Orchestration using k8s Orchestrator.

1. [k8s Setup](#k8s-setup)

2. [Provisioning EIS with k8s](#provisioning-eis-with-k8s)

3. [Deploying EIS Application with K8s](#deploying-eis-application-with-k8s)

4. [Steps for Enabling Basler Camera with k8s](README_Basler_Nw.md)

5. [Steps for Multinode Deployment](README_Multinode_deployment.md)


> ***Note:*** This readme is `W.I.P`. Steps for enabling other EIS features with k8s will be integrated here soon.

## K8s Setup

  > **Note**:
  > Please follow the standard proxy settings if your installation machines are behind proxy environment.
  > For orchestrator deployment in a proxy environment, make sure all communicating machines IP addresses
  >are in `no_proxy` list both in `docker` & `system` environment files as follows.
  > 1.  /etc/environment
  > 2.  /etc/systemd/system/docker.service.d/http-proxy.conf
  > 3.  ~/.docker/config.json

  * Install Kubernetes by running below command .
      ```sh
      $ sudo ./k8s_install.sh
      ```

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
## Deploying EIS Application with K8s
  > **Note:** `k8s_eis_deploy.yml` file be generated in `build/k8s` directory by `eisbuilder`.

  * Goto `build/k8s` directory.
    ```sh
    $ cd build/k8s
    ```
  * Deploy `k8s` using `kubectl` utility by following command
    ```sh
    $  kubectl -n eis apply k8s_eis_deploy.yml
    ```
  * Make sure the pods are `Running` fine as per deploy yml file.
    ```sh
    $  kubectl -n eis get pods
    ```
