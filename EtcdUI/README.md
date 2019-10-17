# `ETCD UI Service`

* Open your browser and enter the address: http://127.0.0.1:7070/etcdkeeper
* Click on the version of the title to select the version of ETCD. The default is V3. Reopening will remember your choice.
* Right click on the tree node to add or delete.
* For secure mode, authentication is required. User name and password needs to be entered in the dialogue box.
* Username is 'root' and default password is located at ETCD_ROOT_PASSWORD key under environment section in [docker_setup/provision/dep/docker-compose-provision.override.prod.yml](../docker_setup/provision/dep/docker-compose-provision.override.prod.yml).

---
**NOTE**:
1. If ETCD_ROOT_PASSWORD is changed, EIS must to be provisioned again. Please follow below command in < EIS Repo >/docker_setup/provision folder to reprovision EIS.

        ```
        $ sudo ./provision_eis.sh <path_to_eis_docker_compose_file>

        eq. $ sudo ./provision_eis.sh ../docker-compose.yml

        ```
2. Only VideoIngestion and VideoAnalytics based services will have watch for any changes. Any changes done to those keys will be reflected at runtime in EIS.
3. For changes done to any other keys, EIS stack needs to be restarted for it to take effect. Please execute below command in working directory docker_setup/ to restart EIS.
    ```
    $ docker-compose down
    $ docker-compose up
    
    ```
---
