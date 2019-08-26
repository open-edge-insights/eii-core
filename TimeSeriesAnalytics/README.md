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
l
1. To start the mqtt-temp-sensor, please refer [tools/mqtt-temp-sensor/README.md](../../tools/mqtt-temp-sensor/README.md) .

2. In case, if SI wants to use the IEdgeInsights only for Point data Analytics,
<<<<<<< HEAD
   then open the docker-compose.yml file in [docker_setup/docker-compose.yml](../../docker_setup/docker-compose.yml) and
   use only ia_telegraf, ia_influxdbconnector, ia_data_analytics and ia_visualizer services.
=======
   then comment all Video use case containers in ../docker_setup/docker-compose.yml
>>>>>>> EIS: Cleanup of stale code & replace IEI with EIS

3. Starting the EIS.
   To start the EIS in production mode, provisioning is required. For more information on provisioning
   please refer the [README.md](../../README.md#Enable-security-(Production-Mode)).
   After provisioning, please follow the below commands
   ```
   cd docker_setup
   docker-compose build
   docker-compose up -d
   ```

   To start the EIS in developer mode, please refer the [README.md](../../README.md#Run-EIS-PCB-Demo-in-Developer-Mode).

4. To verify the output please check the output of below commands
   ```
   docker logs -f ia_influxdbconnector
   docker logs -f ia_visualizer
   ```

   Below is the snapshot of sample output of the ia_influxdbconnector command.
   ```
   I0822 09:03:01.705940       1 pubManager.go:111] Published message: map[data:point_classifier_results,host=ia_telegraf,topic=temperature/simulated/0 temperature=19.29358085726703,ts=1566464581.6201317 1566464581621377117] 
   I0822 09:03:01.927094       1 pubManager.go:111] Published message: map[data:point_classifier_results,host=ia_telegraf,topic=temperature/simulated/0 temperature=19.29358085726703,ts=1566464581.6201317 1566464581621377117]
   I0822 09:03:02.704000       1 pubManager.go:111] Published message: map[data:point_data,host=ia_telegraf,topic=temperature/simulated/0 ts=1566464582.6218634,temperature=27.353740759929877 1566464582622771952]
   ```

   Below is the snapshot of sample output of the ia_visualizer command.
   ```
   2019-08-26 10:38:13,869 : INFO : root : [visualize.py] :zmqSubscriber : in line : [402] : Classifier results: {'data': 'point_classifier_results,host=ia_telegraf,topic=temperature/simulated/0 temperature=18.624896449633443,ts=1566815892.9866698 1566815892987649482\n'}
   2019-08-26 10:38:15,154 : INFO : root : [visualize.py] :zmqSubscriber : in line : [402] : Classifier results: {'data': 'point_classifier_results,host=ia_telegraf,topic=temperature/simulated/0 temperature=10.84324034762356,ts=1566815893.9882996 1566815893989408936\n'}
   2019-08-26 10:38:15,154 : INFO : root : [visualize.py] :zmqSubscriber : in line : [402] : Classifier results: {'data': 'point_classifier_results,host=ia_telegraf,topic=temperature/simulated/0 temperature=10.052214661918322,ts=1566815894.990011 1566815894991129870\n'}
   2019-08-26 10:38:16,776 : INFO : root : [visualize.py] :zmqSubscriber : in line : [402] : Classifier results: {'data': 'point_classifier_results,host=ia_telegraf,topic=temperature/simulated/0 temperature=12.555421975490562,ts=1566815895.9918363 1566815895993111771\n'}
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
