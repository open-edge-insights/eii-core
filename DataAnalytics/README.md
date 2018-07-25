# DataAnalytics:

This module gets the point data from Influxdb via kapacitor's subscription and using the image handle in the influxdb, gets the images from the ImageStore and runs the classifier's algorithm to find any detects in these images. The results are written back to influxdb after the 
analysis. These results are streamed to DataAgent via StreamManager's subscription to influxdb and published to the message bus like NATS server which could be consumed by NATS client.

## Running DataAnalytics from $GOPATH/src/iapoc_elephanttrunkarch - present working directory

**Pre-requisites:**
1. Configuring ETR agent:
    * Install OpenCV 3 with Python 3 support by running command `sudo -H pip3.6 install opencv-python`
    * Install and configure PostgreSQL (PostgreSQL is used as the underlying database by the factory agent)
        * Install postgreSQL: `sudo apt install postgresql`
        * Configuring postgreSQL:
            * Create database: `sudo -u postgres createdb ali`
            * Create user ali with password `intel123`: `sudo -u postgres createuser -l -P ali`
            * Storing password credentials in a password file: 
                
                ```sh
                    $ touch ~/.pgpass && \
                    echo "localhost:5432:ali:ali:intel123" > ~/.pgpass && \
                    chmod 0600 ~/.pgpass
                ```
        * Populating database by running below commands:
            * Insert camera location: `python3.6 factory.py db insert cam-loc 0 0 0`
            * Insert camera position: `python3.6 factory.py db insert cam-pos 0 0 0`
            * Insert camera: `python3.6 factory.py db insert camera camera-serial-number 1 1`
2. Install below DataAnalytics dependencies:
    * python3.6 package by following [this](http://ubuntuhandbook.org/index.php/2017/07/install-python-3-6-1-in-ubuntu-16-04-lts/)
    * **sudo -H pip3.6 install -r classifier_requirements.txt** - installs all dependencies for classifer program
    * **sudo -H pip2 install protobuf** - Needed by the kapasender.py UDF file.
    * **sudo apt-get install mosquitto** - MQTT implementation
    * **sudo apt-get install postgresql** - database used by ETR

**Note**: For more details on the pre-requisites and ETR, please refer: `agent/README.md`

For running the DataAnalytics, the kapacitor UDF setup is required. For more details, refer below links:
* Writing a sample UDF at [anomaly detection](https://docs.influxdata.com/kapacitor/v1.5/guides/anomaly_detection/)
* UDF and kapacitor interaction [here](https://docs.influxdata.com/kapacitor/v1.5/guides/socket_udf/)

The udf is **DataAnalytics/kapasender.py** and the tick script is **DataAnalytics/classifier.tick**

Follow below steps to start DataAnalytics module:
1. Kapacitor configuration:
    Use the below config in **docker_setup/config/kapacitor.conf** file for enabling the UDF. Change 
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
    python3.6 DataAnalytics/classifier.py --config factory.json
    ```
    
6. Run this command in another terminal:
    ```sh
    sudo -E env "PATH=$PATH" kapacitord -config docker_setup/config/kapacitor.conf
    ```
    
7. Run this command in another terminal
    ```sh
    kapacitor define classifier_task -tick DataAnalytics/classifier.tick
    
    kapacitor enable classifier_task
    ```
    
    