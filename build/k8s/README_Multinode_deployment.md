# EIS setup with Kubernetes for multi node setup of single master and worker nodes.

## Run below set of commands on all machines.
#### Add gpg key and Docker for the docker repository.
```
$ curl -fsSL https://download.docker.com/linux/ubuntu/gpg | sudo apt-key add -
$ sudo add-apt-repository "deb [arch=amd64] https://download.docker.com/linux/ubuntu bionic stable"
```
#### Add gpg key for Kubernetes, and then add the repository.
```
$ curl -s https://packages.cloud.google.com/apt/doc/apt-key.gpg | sudo apt-key add -
$ cat << EOF | sudo tee /etc/apt/sources.list.d/kubernetes.list
deb https://apt.kubernetes.io/ kubernetes-xenial main
EOF
```
#### Update the packages and Install Docker, kubelet, kubeadm, and kubectl
```
$ sudo apt-get update
$ sudo apt-get install -y docker-ce kubelet kubeadm kubectl
$ sudo apt-mark hold docker-ce kubelet kubeadm kubectl
```
## Initialize the cluster now 
Run below command on master node.
```
$ sudo kubeadm init --pod-network-cidr=10.244.0.0/16 [a. store the kubeadm join command which comes as output of this command]
$ mkdir -p $HOME/.kube
$ sudo cp -i /etc/kubernetes/admin.conf $HOME/.kube/config
$ sudo chown $(id -u):$(id -g) $HOME/.kube/config
```
#### Apply flannel CNI. Flannel is network overlay, so nodes can communicate with each other.
```
$ kubectl apply -f https://raw.githubusercontent.com/coreos/flannel/master/Documentation/kube-flannel.yml
```
## Run below commands on worker node.
```
$ sudo kubeadm join [your unique string from the kubeadm init command on master node in step 2.]
```
## Now that the nodes are successfully joined, run first kubectl command to view cluster node status.
```
$ kubectl get nodes
```
## Provisioning the master. All steps to perform on master node.
   * Add 'HOST_IP' and 'ETCD_HOST' in build/.env file.
   * Update ETCD_CLIENT_PORT and ETCD_PEER_PORT to 8379 and 8380 respectively in build/.env file.
   * Update EIS_LOCAL_PATH, PROVISION_MODE as k8s in provision/.env file
   * Add master node name in etcd_prodmode.yml or etcd_devmode.yml to run etcd always on master node.
     E.g as below:
     spec:
     nodeName: master-node
   * Run deployment.sh script after this change.
   * Run below command 
      ```
      $ sudo ./provision_eis.sh ../docker-compose.yml
      ```
   * Run the deployment.sh script to get deploy_yml folder with env variables substituted in yml files. It will use yml files suffixed with deployment to create deployments.
   * Do not make any change in yml files present in deploy_yml folder as it will be overwritten.
   * If any change is needed, please modify in deployment ymls present in k8s directory and run deployment.sh to generate final ymls.
   * Run below command to see all deployment details:
   ```
   $ kubectl -n kube-eis get all
   ```
> **Note**: In order to run video use case in dev mode. It is needed to comment the secrets in respective deployment.yml files . E.g. Comment below in video_ingestion_deployment.yml if need to run in DEV mode:
   ```
   volumeMounts:
     - name: "ca-cert"
       mountPath: /run/secrets/ca_etcd
     - name: "video-ingest-cert"
       mountPath: /run/secrets/etcd_VideoIngestion_cert
     - name: "video-ingest-key"
       mountPath: /run/secrets/etcd_VideoIngestion_key
   ```
> **Note** Follow below link for Basler camera connection. Network settings need to be done on all nodes.

> Note that in case of deployments use video_ingestion_deployment.yml file instead of pod file ia_video_ingestion.yml
> [README_Basler_Nw.md](README_Basler_Nw.md).