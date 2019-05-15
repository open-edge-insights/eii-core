# VideoAnalytics:

This module helps to validate/run the classifier algos without using kapacitor in baremetal environment.
It uses StreamSubLib to subscribe to influxDB and gets the point data from measurement **stream1**,
call the classifier algos based on the input config and persists the classified result to influxDB measurement **classifier_results**.

# Running App

1) Follow Build & Installation process of IEI at:
   https://gitlab.devtools.intel.com/Indu/IEdgeInsights/IEdgeInsights/blob/master/docker_setup/README.md

   For running VideoAnalytics in bare-metal, the DEV_MODE should be set to "true" in docker_setup/.env file.

2) Run the following script for the bare metal execution of VideoAnalytics.
   ```
   cd DataAnalytics/VideoAnalytics/
   python3 va_baremetal_setup.py
   ```

   Note: If you are running in Ubuntu 18.04, the install of OpenVino will fail. You will have to manually install the drivers by following instructions in the below link:
   https://github.com/intel/compute-runtime/releases

3) Start the visualizer app, it will start showing the processed images
