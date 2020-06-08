 # README for configuring Basler camera to k8s network.

 Multus CNI enables attaching multiple network interfaces to pods in Kubernetes. Install it.
 Skip step 2 & 3 if multus is already installed and execute others.
 1. go to build/k8s folder.
 2. git clone https://github.com/intel/multus-cni.git && cd multus-cni
 3. cat ./images/multus-daemonset.yml | kubectl apply -f -
 4. kubectl create -f net_attach_def.yml
 5. Check ifconfig of the machine where k8s pods are going to be deployed. Find out the ethernet interface name against IP address of machine. 
 Edit macvlan.yml ->Replace 'master' under 'config' with specific ethernet interface name.
  e.g. "master": "eno1"[en01 is ethernet interface name here,]
 6. kubectl create -f macvlan.yml. 
 7. Update the pod definition - ia_video_ingestion.yml with below lines
 '''
   'annotations:
      k8s.v1.cni.cncf.io/networks: macvlan-conf '
    Under metadata below namespace.
    E.g. 
      namespace: kube-eis
      annotations:
        k8s.v1.cni.cncf.io/networks: macvlan-conf
'''
 8. Run below command to check pod status
 '''
 kubectl -n kube-eis describe pods ia-video-ingestion
 '''
 If encountered any error regarding dhcp (error adding container to network "macvlan-conf": delegateAdd: error invoking DelegateAdd - "macvlan": error in getting result from AddNetwork: error dialing DHCP daemon: dial unix /run/cni/dhcp.sock: connect: connection refused, failed to clean up sandbox container)

 Run below commands: 
'''
 a. sudo rm -f /run/cni/dhcp.sock
 b. cd /opt/cni/bin. 
 c. sudo ./dhcp daemon
 '''