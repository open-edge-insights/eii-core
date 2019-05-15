# Automation of Bare Metal execution of VideoAnalytics
# Change DEV_MODE=true in docker_setup/.env file

import subprocess

subprocess.call("sudo ./install_openvino.sh", shell=True)
subprocess.call("sudo ./install.sh", shell=True)
subprocess.Popen("source ./setenv.sh", shell=True, executable="/bin/bash")
subprocess.Popen("source /opt/intel/openvino/bin/setupvars.sh",
                 shell=True, executable="/bin/bash")
subprocess.call("docker stop ia_video_analytics", shell=True)
subprocess.call("python3.6 VideoAnalytics.py --config \
                ../../docker_setup/config/algo_config/factory_pcbdemo.json \
                --log-dir ./ --log-name videoanalytics.log", shell=True)

# Usage
# python3 va_baremetal_setup.py
