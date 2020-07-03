 # README for configuring Basler camera to k8s network.

 Multus CNI enables attaching multiple network interfaces to pods in Kubernetes. Install it.
 Skip installing multus if it is already installed and execute others.
 1. Go to build/k8s folder. Run below :
      ```
      $ git clone https://github.com/intel/multus-cni.git && cd multus-cni
      $ cat ./images/multus-daemonset.yml | kubectl apply -f -
      $ kubectl create -f net_attach_def.yml
      ```
 2. Check ifconfig of the machine where k8s pods are going to be deployed. Find out the ethernet interface name against IP address of machine.

  >Edit macvlan.yml ->Replace 'master' under 'config' with specific ethernet interface name.

  >e.g. "master": "eno1"[eno1 is ethernet interface name here]
      ```
      $ kubectl create -f macvlan.yml
      ```
 7. Update the pod definition - ia_video_ingestion.yml with below lines
>
     'annotations:
      k8s.v1.cni.cncf.io/networks: macvlan-conf '
      Under metadata below namespace.
      E.g.
        namespace: kube-eis
        annotations:
        k8s.v1.cni.cncf.io/networks: macvlan-conf
>
 8. Run below command to check pod status
  ```
      $ kubectl -n kube-eis describe pods ia-video-ingestion
  ```
> **NOTE** : If encountered any error regarding dhcp as below:

>(error adding container to network "macvlan-conf": delegateAdd: error invoking DelegateAdd - "macvlan": error in getting result from AddNetwork: error dialing DHCP daemon: dial unix /run/cni/dhcp.sock:
connect: connection refused, failed to clean up sandbox container)

>Run below commands
```
    $ sudo rm -f /run/cni/dhcp.sock
    $ cd /opt/cni/bin
    $ sudo ./dhcp daemon
```
