# VideoAnalytics:

This module helps to validate/run the classifier algos without using kapacitor in baremetal environment.
It uses StreamSubLib to subscribe to influxDB, gets frame from ImageStore, runs the classifier algos
based on the input config and persists the classified result to influxDB measurement.

## Running App

1. Follow Build & Installation process of IEI at:
   [IEI README.md](../../README.md)

   For running VideoAnalytics in bare-metal, the DEV_MODE should be set to "true" in [.env](../../docker_setup/.env) file.

2. Install OpenVino:
   Download the right version of OpenVINO and extract inside the VideAnalytics directory as per the [IEI README.md](../../README.md).

3. Run the following script for the bare metal execution of VideoAnalytics.

   ```
    sudo -E ./va_baremetal_setup.sh
   ```
