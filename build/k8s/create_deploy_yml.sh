#!/bin/bash -e
# Script to run to create K8 ymls with env variables substituted to actual values.
# This script on running, substitutes the environment variables present in ymls in k8s folder
# and creates yml files with actual values in deploy_yml folder.
 set -a
 source ../.env
 source ../provision/.env
 set +a

 mkdir -p deploy_yml

for file in *.yml 
do
 echo "$file"
 envsubst < $file > deploy_yml/$file
done
