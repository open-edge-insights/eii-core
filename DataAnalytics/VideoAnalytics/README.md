# VideoAnalytics:

This module helps to validate/run the classifier algos without using kapacitor in baremetal environment.
It uses StreamSubLib to subscribe to influxDB and gets the point data from measurement **stream1**,
call the classifier algos based on the input config and persists the classified result to influxDB measurement **classifier_results**.

# Running App

1) Follow Build & Installation process of IEI at:
   https://gitlab.devtools.intel.com/Indu/IEdgeInsights/IEdgeInsights/blob/master/docker_setup/README.md

   For running VideoAnalytics in bare-metal, the DEV_MODE should be set to "true" in docker_setup/.env file.

2) Install OpenVino:
   Download the right version of OpenVINO and extract inside the VideAnalytics directory as per the docker_setup/README.md.
   Run the following script:
   ```
   cd DataAnalytics/VideoAnalytics/
   sudo ./install_openvino.sh
   ```

   If you are running in Ubuntu 18.04, the install of OpenVino will fail. You will have to manually install the drivers by following instructions in the below link:
   https://github.com/intel/compute-runtime/releases


3) Install python dependencies and set environment variable:
    ```
    sudo ./install.sh
    source ./setenv.sh
    ```

4) Initialize openvino environment:
   ```
   source /opt/intel/openvino/bin/setupvars.sh
   ```
5) Stop ia_video_analytics container if it is running:
    ```
    # docker stop ia_video_analytics
    ```
6) Run app:
    python3.6 VideoAnalytics.py --config ../../docker_setup/config/algo_config/factory_pcbdemo.json  --log-dir ./ --log-name videoanalytics.log

  Start the visualizer app, it will start showing the processed images
  
      
