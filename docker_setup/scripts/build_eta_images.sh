#!/bin/bash

source ./init.sh

echo "Copying the docker_setup/resolv.conf to /etc/resolv.conf..."
cp -f ../resolv.conf /etc/resolv.conf

echo "Restarting docker daemon to pick the resolv.conf changes..."
service docker restart

#Build all base ETA images
echo "0.1 Building pybase image..."
# create soft link .dockerignore
ln -fs  $diPathFromRoot/.dockerignore.base  $rootDir/.dockerignore

# build py base image
docker build -f $dockerfileDir/Dockerfile.pybase -t ia/pybase:1.0 $rootDir

echo "0.2 Building gobase image..."
# build go base image
docker build -f $dockerfileDir/Dockerfile.gobase -t ia/gobase:1.0 $rootDir

echo "0.3 Building gopybase image..."
# build py & go base image
docker build -f $dockerfileDir/Dockerfile.gopybase -t ia/gopybase:1.0 $rootDir

echo "1. Building DataAgent image..."
# create soft link .dockerignore
ln -fs  $diPathFromRoot/.dockerignore.da  $rootDir/.dockerignore
# build DataAgent docker image
docker build -f $dockerfileDir/Dockerfile.da -t ia/data_agent:1.0 $rootDir

echo "2. Building DataAnalytics/Classifier image..."
# create a soft link .dockerignore
ln -fs  $diPathFromRoot/.dockerignore.classifier  $rootDir/.dockerignore
# build classifier/kapacitor docker image
docker build -f  $dockerfileDir/Dockerfile.classifier -t ia/data_analytics:1.0 $rootDir
rm -rf  $rootDir/kapacitor $rootDir/kapacitord

echo "3. Building NATS client image..."
# create soft link .dockerignore
ln -fs  $diPathFromRoot/.dockerignore.natsClient  $rootDir/.dockerignore
# build NATS client docker image
docker build -f  $dockerfileDir/Dockerfile.natsClient -t ia/nats_client:1.0 $rootDir
rm -rf  $rootDir/natsClient-go

echo "4. Building VideoIngestion image..."
# create a soft link .dockerignore
ln -fs  $diPathFromRoot/.dockerignore.vi  $rootDir/.dockerignore
# build video ingestion docker image
docker build -f $dockerfileDir/Dockerfile.vi -t ia/video_ingestion:1.0  $rootDir