# EIS setup with Kubernetes for single node.This Readme is for singlenode deployment only.

> **Note**: Follow below link for Multinode deployment.

[README_Multinode_deployment.md](README_Multinode_deployment.md)

1. Install Kubernetes by running below command . 
    ```
    $ sudo ./k8s_install.sh
    ```
2. Add 'HOST_IP' and 'ETCD_HOST' in build/.env file.
3. Update ETCD_CLIENT_PORT and ETCD_PEER_PORT to 8379 and 8380 respectively in build/.env file.
3. Update EIS_LOCAL_PATH, PROVISION_MODE as k8s in provision/.env file
4. Run provisoning by running below command.
    ```
    $ sudo ./provision_eis.sh ../docker-compose.yml
    ```
5. Run the create_deploy_yml.sh script to get deploy_yml folder with env variables substituted in yml files.
6. Do not make any change in yml files present in deploy_yml folder as it will be overwritten.
7. If any change is needed, please modify in ymls present in k8s directory and run create_deploy_yml.sh to generate final ymls.
8. Run below commands[ to check if any pod for the partciular apps exist]:
```
    $ kubectl -n kube-eis get pods
```
   If pods and service exists, run below command to delete them else run command mentioned in 'b.'
  ```
 a. kubectl -n kube-eis delete (appname.yml)
    E.g. Kubectl -n kube-eis delete ia_video_ingestion.yml
  ```
And then run below command to get pods and services running.
```
b. kubectl -n kube-eis apply (appname.yml)
  E.g. kubectl -n kube-eis apply -f ia_video_ingestion.yml
```
9. Web Visualizer can be accessed through https://IPaddress:30007
```
 E.g. https://10.223.109.135:30007/
```
10. In order to run video use case in dev mode.It is needed to comment the secrets in .yml files .
E.g. To comment below in ia_video_ingestion.yml if need to run in DEV mode:
  ```
  volumeMounts:
    - name: "ca-cert"
      mountPath: /run/secrets/ca_etcd
    - name: "video-ingest-cert"
      mountPath: /run/secrets/etcd_VideoIngestion_cert
    - name: "video-ingest-key"
      mountPath: /run/secrets/etcd_VideoIngestion_key
  ```
11. Follow below link for Basler camera connection.
[README_Basler_Nw.md](README_Basler_Nw.md)
