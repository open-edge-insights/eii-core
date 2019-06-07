#! /bin/bash

# Automation of Bare Metal execution of VideoAnalytics
# Change DEV_MODE=true in docker_setup/.env file

./install_openvino.sh
./install.sh
source ./setenv.sh
source /opt/intel/openvino/bin/setupvars.sh
docker stop ia_video_analytics
python3.6 VideoAnalytics.py --config \
                ../../docker_setup/config/algo_config/factory_pcbdemo.json \
                --log-dir ./ --log-name videoanalytics.log


# Usage
# sudo ./va_baremetal_setup.sh
