#!/bin/bash -e

# Copyright (c) 2021 Intel Corporation.

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

# Script to run to create K8 ymls with env variables substituted to actual values.
# This script on running, substitutes the environment variables present in ymls 
# and creates yml files with actual values in deploy_yml folder.

bold=`tput bold`
clear=`tput sgr0`
blink=`tput blink`
f_y=`tput setaf 3`
f_w=`tput setaf 7`
f_g=`tput setaf 2`

echo "Setting Up Multus Network"

if [ -z "$1" ]
then 
    echo "${bold}${f_w}Pass ${f_y}'Ethernet'${f_w} Interface for Multus Configuration as Argument${clear}"
    exit 1
else
    echo "${bold}Multus Setup using network configuration with ${f_y}${bold} '$1' ${f_w}Ethernet Interface${clear}"
    cat macvlan.yml | sed '/"master":/ s/"master":[^,]*/"master": "'"$1"'"/' > macvlan-deploy.yml
fi

# Cloning the Multus GitHub repository
echo "${f_w}Cloning the Multus GitHub repository${clear}"

if [ -d "multus-cni" ]
then
    echo "Removing existing Dir"
    rm -rf multus-cni
fi


git clone https://github.com/intel/multus-cni.git && cd multus-cni && \
    git checkout -b  v3.6 tags/v3.6


# Creating Multus Daemon
echo "${f_w}${bold}Creating Multus Daemon${clear}"
cat ./images/multus-daemonset.yml | kubectl delete -f - --ignore-not-found
cat ./images/multus-daemonset.yml | kubectl apply -f -

# Attaching Network Definitions for mapping with Multus Network
echo "${f_w}${bold}Attaching Network Definitions for mapping with Multus Network${clear}"
cd ..
kubectl delete -f ./net-attach-def.yml --ignore-not-found
kubectl create -f ./net-attach-def.yml
sleep 5 
# Launch `macvlan` pod for the defined network.
echo "${f_w}${bold}Launching macvlan pod for the defined network.${clear}"
kubectl delete -f ./macvlan-deploy.yml --ignore-not-found
kubectl create -f ./macvlan-deploy.yml

# Describing Created Config
kubectl describe network-attachment-definitions macvlan-conf -n default | grep 'Config'

echo "${bold}${f_g}${blink}Setup Finished !!!${clear}"

