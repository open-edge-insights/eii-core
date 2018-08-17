# VisualHMIClient Module:

VisualHMIClient is a datasync app which takes classifier results and posts the same to VisualHmi Server.

VisualHMI starts subscribing on databus topic (`classifier_results`) to get the classifier results from the databus server. Based on the Classifier results, VisualHMICLient converts the data to VisualHMI standard as a json and post the data to VisualHMI REST API endpoint. 

Also, VisualHMIClient gets the image blob from the gRPC interface `GetBlob(imgHandle)` from DataAgent module and persist the image in VisualHMI local filesystem.

> **Note**:
> * VisualHMI runs only on Python2.7 as our OPCUA Databus Client have support for python2.7 only at present.

## Pre-requisites:

* Please make sure that the below libraries availability. (either in `iapoc_elephanttrunkarch` or yourfolder)
  * DataBusAbstraction Library  (files under `DataBusAbstraction/py` in our `iapoc_elephanttrunkarch` repo)
  * GRPC Client wrapper (`client.py` and protobuff files: 
    
* Set your `PYTHONPATH` based on where libraries are placed
    For Eg:
    If the pwd is `iapoc_elephanttrunkarch` and it is under home:
    	echo PYTHONPATH=$PYTHONPATH:~/iapoc_elephanttrunkarch:iapoc_elephanttrunkarch/DataBusAbstraction/py/:iapoc_elephanttrunkarch/DataAgent/da_grpc/protobuff
    else:
    	set your `PYTHONPATH` appropriately
* Install VisualHmiClient dependencies:

  ```sh
    pip2.7 install -r requirements.txt
  ```

* Configure Databus & VisualHMI Server Details using `config.json`

## Steps to run VisualHMI (RefinementApp) module

VisualHMIClient Can be run two Modes

* Running VisualHMI in production mode: 

  ```sh
  python2.7 VisualHmiEtaDataSync.py
  ```
    
* Running VisualHMI without HMI Backend (For Testing)

  ```sh
  python2.7 VisualHmiEtaDataSync.py -local <path to store image locally>
  ```

> **Note**:
> Currently VisualHMI cannot be exited directly. Please use following command to terminate the VisualHMI App
> ```sh
  ps -ef | grep VisualHmiEtaDataSync.py | grep -v grep | awk {'print$2'} | xargs kill
> ```