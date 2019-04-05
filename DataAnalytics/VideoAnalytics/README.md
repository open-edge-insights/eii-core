# VideoAnalytics:

This module helps to validate/run the classifier algos without using kapacitor in baremetal environment.
It uses StreamSubLib to subscribe to influxDB and gets the point data from measurement **stream1**,
call the classifier algos based on the input config and persists the classified result to influxDB measurement **classifier_results**.

# Running App

1) Edit the file cert-tool/config.json . Add the following entry in the "Certificate" list

   {
      "pcbdemo": {
        "server_alt_name": "<SYSTEM_HOSTNAME>"
      }
   }
   Note: SYSTEM_HOSTNAME has to be replaced with the host name.
   
   Follow Build & Installation process of IEI at:
   https://gitlab.devtools.intel.com/Indu/IEdgeInsights/IEdgeInsights/blob/master/docker_setup/README.md
      

2) Install OpenVino:
   Download the full package for OpenVINO toolkit for Linux version "2018 R5" from the official website (https://software.intel.com/en-us/openvino-toolkit/choose-download/free-download-linux) and extract it inside IEdgeInsights/DataAnalytics/VideoAnalytics. Post this step a directory named l_openvino_toolkit_xxxxx/ will be present inside VideoAnalytics directory.
   and extract inside DataAnalytics dir(Download and extract are optional if done in first steps)
   Run the following script
   cd DataAnalytics/VideoAnalytics/
   sudo ./install_openvino.sh

3) Install python dependencies and set environment variable:
    sudo ./install.sh <systemp_ip>
    source ./setenv.sh <systemp_ip>

4) Run app:
    python3.6 PCBDemo.py --config ../../docker_setup/config/factory.json  --log-dir /home/pcbdemo --log-name pcbdemo.log

    
5) Validation:
  Stop the ia_data_analytics container
  # docker stop ia_data_analytics
  Start the visualizer app, it will start showing the processed images
  
      