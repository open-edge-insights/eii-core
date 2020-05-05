#!/bin/bash -e

# Copyright (c) 2020 Intel Corporation.

# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

# Usage: run as sudo ./k8_install.sh
# This script will install kubernetes on the system.
UBUNTU_VERSION=18.04
K8S_VERSION=1.11.3-00
node_type=master

# Update all installed packages.
echo "Updating all installed packages"
apt-get update
apt-get upgrade

# if you get an error similar to
# '[ERROR Swap]: running with swap on is not supported. Please disable swap', disable swap:
echo "Running with swap on is not supported so disabling swap"
swapoff -a

# install some utils
echo "Installing utils"
apt-get install -y apt-transport-https ca-certificates curl software-properties-common

# Install Docker
echo "Installing Docker"
curl -fsSL https://download.docker.com/linux/ubuntu/gpg | apt-key add -

add-apt-repository "deb [arch=amd64] https://download.docker.com/linux/ubuntu bionic stable"

apt-get update
apt-get install -y docker.io

# Install Network File system client
echo "Installing Network File system client"
apt-get install -y nfs-common

# Enable docker service
echo "Enabling docker service"
systemctl enable docker.service

# Update the apt source list
echo "Updating the apt source list"
curl -s https://packages.cloud.google.com/apt/doc/apt-key.gpg | apt-key add -
add-apt-repository "deb [arch=amd64] http://apt.kubernetes.io/ kubernetes-xenial main"

# Install K8s components
echo "Installing K8s components"
apt-get update
apt install -y kubelet kubeadm kubectl

apt-mark hold kubelet kubeadm kubectl

# Initialize the k8s cluster
echo "Initialize the K8s cluster"
kubeadm init --pod-network-cidr=10.244.0.0/16

# Create .kube file if it does not exists
echo "Create .kube file if it does not exists"
mkdir -p $HOME/.kube

# Move Kubernetes config file if it exists
if [ -f $HOME/.kube/config ]; then
    mv $HOME/.kube/config $HOME/.kube/config.back
fi

cp -f /etc/kubernetes/admin.conf $HOME/.kube/config
chown $(id -u):$(id -g) $HOME/.kube/config

# if you are using a single node which acts as both a master and a worker
# untaint the node so that pods will get scheduled:
echo "Tainting nodes, untaint the nodes in case of single node so that pods will get scheduled"
kubectl taint nodes --all node-role.kubernetes.io/master-

# Install Flannel network
echo "Installing Flannel network"
kubectl apply -f https://raw.githubusercontent.com/coreos/flannel/v0.10.0/Documentation/kube-flannel.yml
echo "Getting list of nodes, Status should show Ready"
kubectl get nodes # Should show Ready
echo "Displaying Tainted nodes"
kubectl describe nodes | grep Taint # Should show <none>
echo "Done."
