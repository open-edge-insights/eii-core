# DataAnalytics:

This module gets the point data from Influxdb via kapacitor's subscription and using the image handle in the influxdb, gets the images from the ImageStore and runs the classifier's algorithm to find any detects in these images. The results are written back to influxdb after the 
analysis. These results are streamed to DataAgent via StreamManager's subscription to influxdb and published to the message bus like NATS server which could be consumed by NATS client.

## Running DataAnalytics

**Pre-requisites**: Read agent/README.md file to install:
1. Install OpenCV 3 with Python 3 support (it also can be installed by pip3.6 install opencv-python)
2. Install and configure PostgreSQL

For running the DataAnalytics, the kapacitor UDF setup is required. For more details, refer below links:
* Writing a sample UDF at [anomaly detection](https://docs.influxdata.com/kapacitor/v1.5/guides/anomaly_detection/)
* UDF and kapacitor interaction [here](https://docs.influxdata.com/kapacitor/v1.5/guides/socket_udf/)

The udf is **DataAnalytics/kapasender.py** and the tick script is **DataAnalytics/classifier.tick**

Follow below steps to start DataAnalytics module:
1. Kapacitor configuration:
    Use the below config in **docker_setup/kapacitor/kapacitor.conf** file for enabling the UDF. Change 
    ```
    [udf]
    [udf.functions]
        [udf.functions.kapasender]
            # Run python
            prog = "/usr/bin/python2"
            args = ["-u", "<pathtoiapocrepo>/DataAnalytics/kapasender.py"]
            timeout = "10s"
            [udf.functions.kapasender.env]
                PYTHONPATH = "<pathtokapacitorrepo>/kapacitor/udf/agent/py"
    ```
    > Note:
    > Please provide abosolute paths to **kapasender.py** and to PYTHONPATH in the kapacitor.conf file.
    > Pre-requisite for this step: **go get github.com/influxdata/kapacitor/cmd/kapacitor** and **go get github.com/influxdata/kapacitor/cmd/kapacitord** to get the go kapacitor lib source under **$GOPATH/src/github.com/influxdata/kapacitor**. The bins **kapacitor** and **kapacitord** would be copied to $GOPATH/bin. 

2. To install python dependencies for this module, use cmd:
    ```sh
    sudo -H pip3.6 install -r classifier_requirements.txt
    ```

3. Install protobuf dependency for kapasender.py, use cmd:
    ```sh
    sudo -H pip2 install protobuf
    ```
    
4. Run cmds: `export PYTHONPATH=.:"<pathtokapacitorrepo>/kapacitor/udf/agent/py"` and `python2 DataAnalytics/kapasender.py` to make sure there are no errors while running kapasender.py via kapacitord

5. Run this command in a terminal:
    ```sh
    python3.6 DataAnalytics/classifier.py factory.json
    ```
    
6. Run this command in another terminal:
    ```sh
    sudo -E env "PATH=$PATH" kapacitord -config docker_setup/config/kapacitor.conf
    ```
    
5. Run this command in another terminal
    ```sh
    kapacitor define classifier_task -tick DataAnalytics/classifier.tick
    
    kapacitor enable classifier_task
    ```
    
    