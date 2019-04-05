# PointDataAnalytics:

This module gets the point data from Influxdb via kapacitor's subscription. Telegraf subscribes the simulated temperature sensor data from [mqtt-temp-sensor](https://gitlab.devtools.intel.com/Indu/IEdgeInsights/mqtt-temp-sensor) and stores it in influxdb. Kapacitor gets the data from influxdb and runs the classifier's algorithm to find the temperature which is not in given range. These results are written back to influxdb after the analysis.

## Running DataAnalytics from $GOPATH/src/IEdgeInsights - present working directory

**Pre-requisites:**
1. Configuring ETR agent:
    * Install OpenCV 3 with Python 3 support by running command `sudo -H pip3.6 install opencv-python`
2. Install below DataAnalytics dependencies:
    * python3.6 package by following [this](http://ubuntuhandbook.org/index.php/2017/07/install-python-3-6-1-in-ubuntu-16-04-lts/)
    * **sudo -H pip3.6 install -r classifier_requirements.txt** - installs all dependencies for classifer program

**Note**: For more details on the pre-requisites on ETR, please refer: `agent/README.md`

For running the PointDataAnalytics, the kapacitor UDF setup is required. For more details, refer below links:
* Writing a sample UDF at [anomaly detection](https://docs.influxdata.com/kapacitor/v1.5/guides/anomaly_detection/)
* UDF and kapacitor interaction [here](https://docs.influxdata.com/kapacitor/v1.5/guides/socket_udf/)

The udf is **PointDataAnalytics/point_classifier.go** and the tick script is **PointDataAnalytics/point_classifier.tick**

Follow below step to start PointDataAnalytics module:
1. Kapacitor configuration:
    Use the below config in **docker_setup/config/kapacitor.conf** file for enabling the UDF. Change 
    ```
    [udf]
    [udf.functions]
        [udf.functions.point_classifier]
            socket = "/tmp/point_classifier"
            timeout = "10s"
    ```
    > Note:
    > Pre-requisite for this step: 
    > 1. Clone the a locally maintained [kapacitor repository](https://github.intel.com/ElephantTrunkArch/kapacitor) inside the `IEdgeInsights` folder by obtaining the command from gerrit/teamforge
    > 2. Run commands `./build.py --clean -o $GOPATH/bin` from kapacitor directory to get the bins **kapacitor** and **kapacitord** copied to $GOPATH/bin. 

    ```

## To run the point classifier UDF


Follow below steps to start Point DataAnalytics module:
1. Telegraf configuration:
    Use the below config in **docker_setup/config/telegraf.conf** file for enabling the UDF. Change
    ```
    # # Read metrics from MQTT topic(s)
    [[inputs.mqtt_consumer]]
    #   ## MQTT broker URLs to be used. The format should be scheme://host:port,
    #   ## schema can be tcp, ssl, or ws.
        servers = ["tcp://$HOST_IP:1883"]
    #
    #   ## MQTT QoS, must be 0, 1, or 2
    #   qos = 0
    #   ## Connection timeout for initial connection in seconds
    #   connection_timeout = "30s"
    #
    #   ## Topics to subscribe to
        topics = [
        "temperature/simulated/0",
        ]
        name_override = "point_data"
        data_format = "json"
    #
    #   # if true, messages that can't be delivered while the subscriber is offline
    #   # will be delivered when it comes back (such as on service restart).
    #   # NOTE: if true, client_id MUST be set
        persistent_session = false
    #   # If empty, a random client ID will be generated.
        client_id = ""
    #
    #   ## username and password to connect MQTT server.
        username = ""
        password = ""

2.  Git clone the [mqtt-temp-sensor](https://gitlab.devtools.intel.com/Indu/IEdgeInsights/mqtt-temp-sensor) repository and follow the given steps to run the broker and publisher.
