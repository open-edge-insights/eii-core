#!/bin/bash

source setenv.sh

echo "Creating external dist_libs package..."
mkdir -p $etaRootDir/dist_libs
dist_libs=$etaRootDir/dist_libs

echo "Creating DataAgent & DataBusAbstraction folders"
mkdir -p $dist_libs/DataAgentClient
mkdir -p $dist_libs/DataAgentClient/client
mkdir -p $dist_libs/DataAgentClient/protobuff
mkdir -p $dist_libs/DataAgentClient/test
mkdir -p $dist_libs/DataBusAbstraction
mkdir -p $dist_libs/Util

echo "Copying source files..."
mkdir -p $dist_libs/DataAgentClient/client/py
cp -rf $rootDir/DataAgent/da_grpc/client/py/. $dist_libs/DataAgentClient/client/py/

mkdir -p $dist_libs/DataAgentClient/client/cpp
cp -rf $rootDir/DataAgent/da_grpc/client/cpp/. $dist_libs/DataAgentClient/client/cpp/

mkdir -p $dist_libs/DataAgentClient/protobuff/py
cp -rf $rootDir/DataAgent/da_grpc/protobuff/py/. $dist_libs/DataAgentClient/protobuff/py/

mkdir -p $dist_libs/DataAgentClient/protobuff/cpp
cp -rf $rootDir/DataAgent/da_grpc/protobuff/cpp/. $dist_libs/DataAgentClient/protobuff/cpp/

mkdir -p $dist_libs/DataAgentClient/test/py
mkdir -p $dist_libs/DataAgentClient/test/py/examples
cp -rf $rootDir/DataAgent/da_grpc/test/py/examples/. $dist_libs/DataAgentClient/test/py/examples/

mkdir -p $dist_libs/DataAgentClient/test/cpp
mkdir -p $dist_libs/DataAgentClient/test/cpp/examples
cp -rf $rootDir/DataAgent/da_grpc/test/cpp/examples/. $dist_libs/DataAgentClient/test/cpp/examples/

mkdir -p $dist_libs/DataBusAbstraction/py
cp -rf $rootDir/DataBusAbstraction/py/. $dist_libs/DataBusAbstraction/py/

cp -rf $rootDir/Util/proxy.py $dist_libs/Util/
cp -rf $rootDir/Util/__init__.py $dist_libs/Util/

echo "Copying package required files..."
cp $rootDir/DataAgent/__init__.py $dist_libs/DataAgentClient/
cp $rootDir/DataAgent/__init__.py $dist_libs/DataAgentClient/client/
cp $rootDir/DataAgent/__init__.py $dist_libs/DataAgentClient/client/py/
cp $rootDir/DataAgent/__init__.py $dist_libs/DataAgentClient/protobuff/
cp $rootDir/DataAgent/__init__.py $dist_libs/DataAgentClient/protobuff/py/

echo "Removing redundant files..."
rm $dist_libs/DataBusAbstraction/py/test/DataBusTest.py

echo "dist_libs package created."