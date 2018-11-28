# Yumei App:

This module controls the Robotic Arm, Panel Light, Alarm Light and Reset button. IO_Module(Adam controller) publishes the data to mqtt with topic "Advantech/<ADAM_MODULE_ID>/data" and telegraf which is subscribed to same topic receives the data from io_module(Adam controller) and writes module_io measurement in influxdb.

YumeiApp uses StreamSubLib to subscribe to influxdb and gets data from **module_io** and **classifier_results**.

1. **module_io** data is used to control robotic arm and reset button.
2. **classifier_results** data is used to control alarm light. 

# Running YumeiApp

**Pre-requisites**
1. Configure io_module using a tool **AdamApax.Net Utility V2.05.11 B19.msi**. This can be downloaded from [here](https://support.advantech.com/support/DownloadSRDetail_New.aspx?SR_ID=1-2AKUDB&Doc_Source=Download) by selecting **Primary** Download Site with the **AdamApax.Net Utility V2.05.11 B19.msi**.

**Note**: System from which io_module is configures and io_module both should be in same subnet.
During the next step, if password prompt is asked then the password is "00000000"

2. Install and open the downloaded app and follow the below instructions:<br>
    a. On the left-hand side pannel, right click on `Ethernet` and select `Search Device`<br>
    b. io_module will be detected (ADAM-6050) under Ethernet, click on it and go to `Network` tab and set the `<ip_address>` to the io_module.<br>
    c. go to `Cloud -> MQTT` tab and set the mqtt broker host (this could be ip-address of any host system where our ETA is running)
     

Changes needs to be done in few files. They are as follows:

1. YumeiApp/yumeiapp_config.json
    * "io_module_ip" : `"ip_address"` given in the pre-requisite step 2.b
    * "mqtt_broker"  : `"ip_address"` given in the pre-requisite step 2.c
2. docker_setup/config/telegraf.conf
    * Under "inputs.mqtt", servers = ["tcp://`ip_address given in the pre-requisite step 2.c`:1883"]

**NOTE**: For the circuit connections of the lab setup, refer 4.5 (4.5.2 IO module) in the document **YumeiApp/HW_Configuration.pdf**