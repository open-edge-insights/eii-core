# Point-data (Time-series data) analytics introduction

Any integral value that gets generated over the time, we can say it is a point data.
The examples can be :
* Temperature at different time in a day.
* Number of oil barrels processed per minute.

By doing the analytics over point data, factory can have anamoly detection mechanism.
That's where the PointDataAnalytics come into the picture.

IEdgeInsights uses the [TICK stack](https://www.influxdata.com/time-series-platform/)
to do point data analytics.

IEdgeInsights has a temperature anomaly detection example for demonstrating the time-series data analytics flow.

The high level flow of the data, in the example can be seen as mqtt-temp-sensor-->Telegraf-->Influx-->Kapacitor-->Influx.

mqtt-temp-sensor simulator sends the data to the Telegraf. Telegraf just sends the same data to the
Influx and Influx sends it to Kapacitor. Kapacitor does anamoly detection and publishes the results back to
Influx.

Here,
Telegraf is TICK stack component and supporting number of input plug-ins for data ingestion.
Influx is time series database.
Kapacitor is an analytics engine where user can write custom analytics plug-ins (TICK scripts).

## Starting the example

1. To start the mqtt-temp-sensor, please refer [tools/mqtt-temp-sensor/README.md](../../tools/mqtt-temp-sensor/README.md) .

2. In case, if SI wants to use the IEdgeInsights only for Point data Analytics,
   then update the 'IEI_SERVICES' variable in the file [docker_setup/.env](../../docker_setup/.env) to 'services_pointdata.json'

   In case, if SI wants to use the IEdgeInsight for Video and Point data Analytics,
   then update the 'IEI_SERVICES' variable in the file [docker_setup/.env](../../docker_setup/.env) to 'services_all.json'.

3. Starting the EIS.
   To start the EIS in production mode, provisioning is required. For more information on provisioning
   please refer the [README.md](../../README.md#Enable-security-(Production-Mode)).
   After provisioning, please follow the below commands
   ```
   cd docker_setup
   sudo make build run
   ```

   To start the EIS in developer mode, please refer the [README.md](../../README.md#Run-EIS-PCB-Demo-in-Developer-Mode).

4. To verify the output please check the output of below command
   ```
   docker logs -f ia_data_agent
   ```

   Below is the snapshot of sample output of the above command.
   ```
   I0608 02:25:57.302956       9 StreamManager.go:159] Publishing topic: point_classifier_results
   I0608 02:25:58.302913       9 StreamManager.go:159] Publishing topic: point_classifier_results
   I0608 02:25:59.302527       9 StreamManager.go:159] Publishing topic: point_classifier_results
   ```

## Purpose of Telegraf
Telegraf is one of the data entry point for IEdgeInsights. It supports many input plugins, which can be used for
point data ingestion. In the above example the mqtt input plugin of Telegraf is used. And below is configuration
of the plugin.

    ```
    # # Read metrics from MQTT topic(s)
    [[inputs.mqtt_consumer]]
    #   ## MQTT broker URLs to be used. The format should be scheme://host:port,
    #   ## schema can be tcp, ssl, or ws.
        servers = ["tcp://localhost:1883"]
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
    ```

The production mode Telegraf configuration file is
[docker_setup/config/telegraf.conf](../../docker_setup/config/telegraf.conf). And in developer mode,
the configuration file is
[docker_setup/config/telegraf_devmode.conf](../../docker_setup/config/telegraf_devmode.conf).

For more information on the supported input and output plugins please refer
[https://docs.influxdata.com/telegraf/v1.10/plugins/](https://docs.influxdata.com/telegraf/v1.10/plugins/)

## Purpose of Kapacitor

  About Kapacitor and UDF
  * User can write the custom anamoly detection algorithm in PYTHON/GOLANG. And these algorithms will be called as
    UDF (user defined function). These algorithms has to follow certain API standards, so that the Kapacitor will be able to
    call these UDFs at run time.

  * IEdgeInsights has come up with the sample UDF written in GOLANG. Kapacitor is subscribed to the InfluxDB, and
    gets the temperature data. After getting this data, Kapacitor calls these UDF, which detects the anamoly in the temerature
    and sends back the results to Influx.

  * The sample UDF is at [point_classifier.go](point_classifier.go) and
    the tick script  is at [point_classifier.tick](point_classifier.tick)

    For more details, on Kapacitor and UDF, please refer below links
    i)  Writing a sample UDF at [anomaly detection](https://docs.influxdata.com/kapacitor/v1.5/guides/anomaly_detection/)
    ii) UDF and kapacitor interaction [here](https://docs.influxdata.com/kapacitor/v1.5/guides/socket_udf/)

  * In production mode the Kapacitor config file is
    [docker_setup/config/kapacitor.conf](../../docker_setup/config/kapacitor.conf)
    and in developer mode the config file would be
    [docker_setup/config/kapacitor_devmode.conf](../../docker_setup/config/kapacitor_devmode.conf)

## Pointdata analytics without Kapacitor
In case, SI don't want to do point data analytics using the Kapacitor, then there is sample App
avaiable at [DataAnalytics/sample_analytics_app/sample_analytics.py](../sample_analytics_app/sample_analytics.py).

In this case, user can write the python based analytics algorithm using the StreamSubLib and DataIngestionLib API. To know more, please refer  [DataAnalytics/sample_analytics_app/README](../sample_analytics_app/README).