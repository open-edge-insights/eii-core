#!/bin/bash

source setenv.sh

echo "Creating external dist_libs package..."
mkdir -p $etaRootDir/dist_libs
dist_libs=$etaRootDir/dist_libs

echo "Creating DataAgentClient & DataBusAbstraction library folders"
mkdir -p $dist_libs/DataAgentClient
mkdir -p $dist_libs/DataAgentClient/client
mkdir -p $dist_libs/DataAgentClient/protobuff
mkdir -p $dist_libs/DataAgentClient/test
mkdir -p $dist_libs/DataBusAbstraction

echo "Copying DataAgent library files..."
mkdir -p $dist_libs/DataAgentClient/client/py
cp -rf $rootDir/DataAgent/da_grpc/client/py/__init__.py $dist_libs/DataAgentClient/client/py/
cp -rf $rootDir/DataAgent/da_grpc/client/py/client.py $dist_libs/DataAgentClient/client/py/

mkdir -p $dist_libs/DataAgentClient/client/cpp
cp -rf $rootDir/DataAgent/da_grpc/client/cpp/. $dist_libs/DataAgentClient/client/cpp/

mkdir -p $dist_libs/DataAgentClient/protobuff/py
cp -rf $rootDir/DataAgent/da_grpc/protobuff/py/__init__.py $dist_libs/DataAgentClient/protobuff/py/
cp -rf $rootDir/DataAgent/da_grpc/protobuff/py/da_pb2_grpc.py $dist_libs/DataAgentClient/protobuff/py/
cp -rf $rootDir/DataAgent/da_grpc/protobuff/py/da_pb2.py $dist_libs/DataAgentClient/protobuff/py/

mkdir -p $dist_libs/DataAgentClient/protobuff/cpp
cp -rf $rootDir/DataAgent/da_grpc/protobuff/cpp/. $dist_libs/DataAgentClient/protobuff/cpp/
cp -rf $rootDir/DataAgent/da_grpc/protobuff/da.proto $dist_libs/DataAgentClient/protobuff/

mkdir -p $dist_libs/DataAgentClient/test/py
mkdir -p $dist_libs/DataAgentClient/test/py/examples
cp -rf $rootDir/DataAgent/da_grpc/test/py/examples/grpc_example.py $dist_libs/DataAgentClient/test/py/examples/
cp -rf $rootDir/DataAgent/da_grpc/test/py/examples/README.md $dist_libs/DataAgentClient/test/py/examples/
cp -rf $rootDir/DataAgent/da_grpc/test/py/client_test.py $dist_libs/DataAgentClient/test/py/client_test.py

mkdir -p $dist_libs/DataAgentClient/test/cpp
mkdir -p $dist_libs/DataAgentClient/test/cpp/examples
cp -rf $rootDir/DataAgent/da_grpc/test/cpp/examples/. $dist_libs/DataAgentClient/test/cpp/examples/
cp -rf $rootDir/DataAgent/da_grpc/test/cpp/Makefile $dist_libs/DataAgentClient/test/cpp/examples/

cp $rootDir/DataAgent/__init__.py $dist_libs/DataAgentClient/
cp $rootDir/DataAgent/__init__.py $dist_libs/DataAgentClient/client/
cp $rootDir/DataAgent/__init__.py $dist_libs/DataAgentClient/client/py/
cp $rootDir/DataAgent/__init__.py $dist_libs/DataAgentClient/protobuff/
cp $rootDir/DataAgent/__init__.py $dist_libs/DataAgentClient/protobuff/py/

echo "Copying DataBusAbstraction library files..."
mkdir -p $dist_libs/DataBusAbstraction/py
mkdir -p $dist_libs/DataBusAbstraction/py/test
mkdir -p $dist_libs/DataBusAbstraction/py/test/examples
cp -rf $rootDir/DataBusAbstraction/py/databus_requirements.txt $dist_libs/DataBusAbstraction/py/
cp -rf $rootDir/DataBusAbstraction/py/DataBus*.py $dist_libs/DataBusAbstraction/py/
cp -rf $rootDir/DataBusAbstraction/py/test/examples/databus_client.py $dist_libs/DataBusAbstraction/py/test/examples
cp -rf $rootDir/DataBusAbstraction/py/test/examples/README.md $dist_libs/DataBusAbstraction/py/test/examples

echo "Client distribution package created at $dist_libs."