This is the reference for all the algorithms used in IEI


## Configuration
[factory.json](https://github.intel.com/IEdgeInsights/IEdgeInsights/blob/master/docker_setup/config/factory.json)
is the configuration file where algorithm related configurations have been made with the following entires:

```
{
    "machine_id": <STRING: Unique string identifying the machine>,
    "trigger_threads": <INT: (OPTIONAL) Number of threads to use for classification>,
    "data_ingestion_manager": {
        "ingestors": {
            "<INGESTOR>": <OBJECT: Ingestor configuration object>
        }
    },
    "triggers": {
        "<TRIGGER>": <OBJECT: Trigger configuration>
    },
    "classification": {
        "classifiers": {
            "<CLASSIFIER>": {
                "trigger": <STRING: Name of the trigger to use>,
                "config": <OBJECT: Classifier configuration>
            }
        }
    }
}

```
If you are working with a RTSP camera, you will need to use opencv as your stream, then put the stream in 'capture_streams' using gstreamer commands.
```
    "machine_id": <STRING: Unique string identifying the machine>,
    "trigger_threads": <INT: (OPTIONAL) Number of threads to use for classification>,
    "log_file_size": <INT: (OPTIONAL) Maximum size for rotating log files>,
    "data_ingestion_manager": {
        "ingestors": {
            "video":{
                "poll_interval":0.01,
                "streams":{
                    "opencv":{
                        "capture_streams":"rtsp://admin:intel123@192.168.0.14:554/cam/realmonitor?channel=1&subtype=1 latency=100 ! rtph265depay ! h265parse ! mfxhevcdec ! videoconvert ! appsink"
                    }
                }
            }
        }
    },
```
> **Note**:
> The example above contains the hard coded string: 'admin:intel123@192.168.0.14:554/cam/realmonitor?channel=1&subtype=1'. This string may need to be altered to work with the specific camera that is in use. Also, this string has embedded within it a user name (admin) and password (intel123). Care must be taken to assure that these values are set correct for your specific camera.
    
Gstreamer mediaSDK decoding commands requires there to be a parser and then the the decoder "h265parse ! mhevcdec".
parsers and decoders:
h264parse !  mfx264dec
h265parse ! mfxhevcdec
mfxmpegvideoparse ! mfxmpeg2dec


The following sections describe each of the major components of the configuration
structure specified above.

### Classifiers

Classifiers represent the different classification algorithms to use on the
video frames that are ingested into the agent.

The the keys in the `classifiers` object in the configuration for the agent
represent the classifier you wish to have the agent load. The value associated
with that key is the configuration for that classifier.

It is important to note that if the specified classifier does not exist, then
the agent will fail to start. In addition, if the configuration for the
classifier is not correct, then the classifier will fail to be loaded.


#### Supported Classifiers

| Classifier | Reference |
| :--------: | :-----------: |
| pcbdemo     | [Link](https://github.intel.com/IEdgeInsights/IEdgeInsights/tree/master/algos/dpm/classification/classifiers/pcbdemo) |

### Ingestion

The ingestion portion of the configuration represents the different ingestors
for the agent to load. The `ingestors` key inside of this configuration
represents which ingestors to load into the ingestion pipeline. Each key in
the `ingestors` object represents an ingestor to load and the value associated
with that key is the configuration for that ingestor.

It is important to note that if the specified ingestor does not exist, then
the agent will fail to start. In addition, if the configuration for the
ingestor is not correct, then the classifier will fail to be loaded.

See the documentation for the ingestors you wish to have loaded for how to
configure them.

#### Supported Ingestors

| Ingestor | Documentation |
| :------: | :-----------: |
| video_file    | [Link](https://github.intel.com/IEdgeInsights/IEdgeInsights/blob/master/algos/dpm/ingestion/video_file.py) |
