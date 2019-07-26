# VideoAnalytics Module

The high level logical flow of VideoAnalytics pipeline is as below:

1. VideoAnalytics will start the zmq publisher thread, single/multiple classifier
   threads and zmq subscriber thread based on classifier configuration.
2. zmq subscriber thread connects to the PUB socket of zmq bus on which
   the data is published by VideoIngestion and adds it to classifier
   input queue
3. Based on the classifier configuration, single or multiple classifier
   threads consume classifier input queue and processes the frames to
   add defects/display_info to metadata and add's the updated data to
   classifier output queue
4. zmq publisher thread reads from the classifier output queue and
   publishes it over the ZMQ bus
    
## Configuration

All the module configurations (classifier) are added 
into etcd (distributed key-value data store) under `AppName` as mentioned in the
service definition in docker-compose.

### Classifier config

The Classifier (user defined function) is responsible for running classification
algorithm on the video frames received from filter. 

Sample configuration for classifiers used:
1. PCB classifier (has to be used with `PCB Filter`)
   ```
   {
        "queue_size": 10,
        "max_workers": 5,
        "ref_img": "./VideoAnalytics/classifiers/ref_pcbdemo/ref.png",
        "ref_config_roi": "./VideoAnalytics/classifiers/ref_pcbdemo/roi_2.json",
        "model_xml": "./VideoAnalytics/classifiers/ref_pcbdemo/model_2.xml",
        "model_bin": "./VideoAnalytics/classifiers/ref_pcbdemo/model_2.bin",
        "device": "CPU"
    }      
    ```
2. Classification sample classifier (has to be used with `Bypass Filter`)
   ```
    {
        "queue_size": 10,
        "output_queue_size": 10,
        "max_workers": 5,
        "model_xml": "./VideoAnalytics/classifiers/ref_classification/squeezenet1.1_FP32.xml",
        "model_bin": "./VideoAnalytics/classifiers/ref_classification/squeezenet1.1_FP32.bin",
        "labels": "./VideoAnalytics/classifiers/ref_classification/squeezenet1.1.labels",
        "device": "CPU"
    }
   ```
3. Dummy classifier (to be used when no classification needs to be done)
   ```
    {
        "queue_size": 10,
        "output_queue_size": 10
    }
   ```    

[TODO: Add classifier related configuration details]


## Installation

* Follow [Etcd/README.md](../Etcd/README.md) to have EIS pre-loaded data in
  etcd

* Run VideoAnalytics

  Present working directory to try out below commands is: `[repo]/VideoAnalytics`

    1. Build and Run VideoAnalytics as container
        ```
        $ cd [repo]/docker_setup
        $ ln -sf DataAnalytics/VideoAnalytics/.dockerignore ../.dockerignore
        $ docker-compose up --build ia_video_analytics
        ```
    2. Update EIS VideoAnalytics keys(classifier) in `etcd` using UI's
       like `EtcdKeeper` or programmatically and see if it picks it up 
       automatically without any container restarts. The important keys here
       would be `classifier_name` which would allow one to
       choose the available classifier configs. So whenever the values
       of above keys or the values of the ones that are pointed by them change, 
       the VA pipeline restarts automatically.
       Eg: <br>
       
       **Sample Etcd config:**
       ```
       "/../classifier_name" : "pcb_classifier"
       "/../pcb_classfier": {...}
       ```

