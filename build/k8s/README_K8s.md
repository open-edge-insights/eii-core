# This README is not complete yet. It is just to help reviewer to understand the steps to perform provisoning done by K8.

# EIS setup with Kubernetes for single node.
# This README is for setting up EIS with kubernetes in a single node setup for now.
1. Install Kubernetes by running below command . 
   sudo ./k8s_install.sh
2. Add 'HOST_IP' and 'ETCD_HOST' in build/.env file.
3. Update ETCD_CLIENT_PORT and ETCD_PEER_PORT to 8379 and 8380 respectively in build/.env file.
3. Update EIS_LOCAL_PATH, PROVISION_MODE as k8s in provision/.env file
4. Run provisoning by running below command.
      'sudo ./provision_eis.sh ../docker-compose.yml'
5. Run the create_deploy_yml.sh script to get deploy_yml folder with env variables substituted in yml files.
6. Do not make any change in yml files present in deploy_yml folder as it will be overwritten.
7. If any change is needed, please modify in ymls present in k8s directory and run create_deploy_yml.sh to generate final ymls.
8. Run below commands:
   kubectl -n kube-eis get pods [ to check if any pod for the partciular apps exist]
   if pods and service exists, run below command to delete them else run command mentioned in Section b
   Section a. kubectl -n kube-eis delete (appname.yml)
   E.g. Kubectl -n kube-eis delete sample_publisher_subscriber.yml
   And then run below command to get pods and services running.
   Section b. kubectl -n kube-eis apply (appname.yml)
   E.g. kubectl -n kube-eis apply sample_publisher_subscriber.yml
9. Web Visualizer can be accessed through https://IPaddress:30007
   E.g. https://10.223.109.135:30007/
