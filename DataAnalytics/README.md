# DataAnalytics:

This module gets the point data from Influxdb via kapacitor's subscription and using the image handle in the influxdb, gets the images from the ImageStore and runs the classifier's algorithm to find any detects in these images. The results are written back to influxdb after the 
analysis. These results are streamed to DataAgent via StreamManager's subscription to influxdb and published to the message bus like NATS server which could be consumed by NATS client.

## Running DataAnalytics from $GOPATH/src/IEdgeInsights - present working directory

**Pre-requisites:**
1. Configuring ETR agent:
    * Install OpenCV 3 with Python 3 support by running command `sudo -H pip3.6 install opencv-python`
2. Install below DataAnalytics dependencies:
    * python3.6 package by following [this](http://ubuntuhandbook.org/index.php/2017/07/install-python-3-6-1-in-ubuntu-16-04-lts/)
    * **sudo -H pip3.6 install -r classifier_requirements.txt** - installs all dependencies for classifer program

**Note**: For more details on the pre-requisites on ETR, please refer: `agent/README.md`

For running the DataAnalytics, the kapacitor UDF setup is required. For more details, refer below links:
* Writing a sample UDF at [anomaly detection](https://docs.influxdata.com/kapacitor/v1.5/guides/anomaly_detection/)
* UDF and kapacitor interaction [here](https://docs.influxdata.com/kapacitor/v1.5/guides/socket_udf/)

The udf is **DataAnalytics/classifier.py** and the tick script is **DataAnalytics/classifier.tick**

Follow below steps to start DataAnalytics module:
1. Kapacitor configuration:
    Use the below config in **docker_setup/config/kapacitor.conf** file for enabling the UDF. Change 
    ```
    [udf]
    [udf.functions]
        [udf.functions.classifier]
            socket = "/tmp/classifier"
            timeout = "10s"
    ```
    > Note:
    > Pre-requisite for this step: 
    > 1. Clone the a locally maintained [kapacitor repository](https://github.intel.com/IEdgeInsights/kapacitor) inside the `IEdgeInsights` folder by obtaining the command from gerrit/teamforge
    > 2. Run commands `./build.py --clean -o $GOPATH/bin` from kapacitor directory to get the bins **kapacitor** and **kapacitord** copied to $GOPATH/bin. 

2. To install python dependencies for this module, use cmd:

    ```sh
    sudo -H pip3.6 install -r classifier_requirements.txt
    ```

3. Run cmds: `export PYTHONPATH=.:./DataAgent/da_grpc/protobuff:"<pathtokapacitorrepo>/udf/agent/py"` to make sure there are no      errors while running classifier.py file.
   
4. Run this command in a terminal:

    ```sh
    python3.6 DataAnalytics/classifier.py --config factory.json
    ```
    
5. Run this command in another terminal:

    ```sh
    touch /tmp/classifier
    chmod 777 /tmp/classifier
    sudo -E env "PATH=$PATH" kapacitord -config docker_setup/config/kapacitor.conf
    ```
    
6. Run these command in another terminal

    ```sh
    kapacitor delete tasks classifier_task
    
    kapacitor define classifier_task -tick DataAnalytics/classifier.tick
    
    kapacitor enable classifier_task
    ```
