#  EIS Provisioning


Follow below steps to provision EIS. Porvisioning must be done before deploying EIS on any node. It will start ETCD as a container and load it with configuration required to run EIS for single node or multi node cluster set up.
Following actions will be performed as part of Provisioning

 * Loading inital ETCD values from json file.
 * For Secure mode, Generating ZMQ secret/public keys for each app and putting them in ETCD.
 * Generating required X509 certs and putting them in etcd.

    Below script starts `etcd` as a container and provision EIS. Please pass docker-compose file as argument, against which provisioning will be done.
    ```
    $ sudo ./provision_eis.sh <path_to_eis_docker_compose_file>

    eq. $ sudo ./provision_eis.sh ../docker-compose.yml

    ```

* By default EIS is provisioned in Developer mode. Please update DEV_MODE=false in [dep/.env](dep/.env) to provision EIS in Secure mode.

* By default EIS is provisioned for Single node deployment. Please uncommment and place proper values for section ' ETCD optinal Configuration required to run ETCD in cluster mode.'  in [dep/.env](dep/.env) for provisioning ETCD for multiple nodes.'.

* By Default, EIS is provisioned with initial values, for any additonal provisioning, please change ETCD_RESET to false in [dep/.env](dep/.env), if initial values are already set for cluster.
