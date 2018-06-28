#!/bin/bash

rootDir="../.."
dockerfileDir="../dockerfiles"
diPathFromRoot="docker_setup/dockerignore"

echo "1. Building DataAgent image..."
# create soft link .dockerignore
ln -fs  $diPathFromRoot/.dockerignore.da  $rootDir/.dockerignore
# build DataAgent binary 
go build -o $rootDir/da $rootDir/DataAgent/DataAgent.go 

# build DataAgent docker image
docker build -f $dockerfileDir/Dockerfile.da -t ia/data_agent:1.0 $rootDir
rm -rf  $dockerfileDir/DataAgent

echo "2. Building DataAnalytics/Classifier image..."
# create a soft link .dockerignore
ln -fs  $diPathFromRoot/.dockerignore.classifier  $rootDir/.dockerignore

# pre-requisite - run go get <kapacitor_repo>, this would put kapacitor and kapacitord in $GOPATH/bin
# TODO: go get by default doens't support getting a specific version. Need to git clone and do go build/install
# locally to have a specific version
go get github.com/influxdata/kapacitor/cmd/kapacitor
go get github.com/influxdata/kapacitor/cmd/kapacitord

# create kapacitor directory structure for storing kapacitor agent code
mkdir -p $rootDir/kapacitor
cp -f $GOPATH/bin/kapacitor $rootDir/kapacitor
cp -f $GOPATH/bin/kapacitord  $rootDir/kapacitor

# copying the kapacitor's udf folder from path: $GOPATH/src/github.com/influxdata/kapacitor/udf/agent/py/ 
# to $rootDir/kapacitor folder
cp -rf $GOPATH/src/github.com/influxdata/kapacitor/udf $rootDir/kapacitor

# build classifier/kapacitor docker image
docker build -f  $dockerfileDir/Dockerfile.classifier -t ia/data_analytics:1.0 $rootDir
rm -rf  $rootDir/kapacitor $rootDir/kapacitord

echo "3. Building NATS client image..."
# create soft link .dockerignore
ln -fs  $diPathFromRoot/.dockerignore.natsClient  $rootDir/.dockerignore
# build DataAgent binary 
go build -o  $rootDir/natsClient-go  $rootDir/StreamManager/test/natsClient.go 

# build DataAgent docker image
docker build -f  $dockerfileDir/Dockerfile.natsClient -t ia/nats_client:1.0 $rootDir
rm -rf  $rootDir/natsClient-go

echo "4. Building VideoIngestion image..."
# create a soft link .dockerignore
ln -fs  $diPathFromRoot/.dockerignore.vi  $rootDir/.dockerignore

# build video ingestion docker image
docker build -f $dockerfileDir/Dockerfile.vi -t ia/video_ingestion:1.0  $rootDir