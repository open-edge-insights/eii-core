#!/bin/bash -x

#  Initialize OpenVINO environment
source /opt/intel/openvino/bin/setupvars.sh

# Set PythonPATH
export PYTHONPATH="../../:../../ImageStore/protobuff:../../ImageStore/protobuff/py/:../../DataAgent/da_grpc/protobuff/py:../../DataAgent/da_grpc/protobuff/py/pb_internal:../../algos/dpm/classification/:/opt/intel/openvino_2019.1.133/python/python3.6/ubuntu16/:../VideoAnalytics/"

# Set required environment variables
shared_key=`docker exec -it ia_data_agent "env" | grep SHARED_KEY | cut -d = -f 2 | tr -d '\r'`
shared_nonce=`docker exec -it ia_data_agent "env" | grep SHARED_NONCE | cut -d = -f 2 |tr -d '\r'`
export SHARED_KEY="$shared_key"
export SHARED_NONCE="$shared_nonce"
export INFLUX_SERVER=localhost
export DEV_MODE=false
export PROFILING=false
