# VisualHmiCleaner Utility:

VisualHmiCleaner is a utiliy to clear the VisualHmi images on the disk & postgresql entries

> **Note**:
> * VisualHmiCleaner runs on python3.6

## Setting up VisualHmiCleaner

### Configuration

  * Login as root User `sudo su`  **Mandatory**

  * Clone or Copy `IEdgeInsights` Src under `/root/`    
    Directory

  * Configure ViusalHmiCleaner
    ```sh
    $vi /root/IEdgeInsights/VisualHmiClient/VisualHmiCleaner/config.json
    ```

    >**Note**:
    >1. `retentiondays` is a numeric value based on which Images & Entries will be cleared from 
    >    VisualHmi.
    >    for Eg.
    >    If retentiondays = 15 . Then Daily it will clean the before 15 Days Images & Entries.
    >    If retentiondays = 0 . Then Daily it will clean All the Entries.
    >
    >2. `dailyjobscedulehrs` and `dailyjobscedulemins` indicates the specific hour & min on which 
    >    VisualHmiCleaner will be Executed based on retentiondays value.

  * Install dependencies:
    ```sh
    $cd /root/IEdgeInsights/VisualHmiClient/VisualHmiCleaner
    $sudo -H pip3.6 install -r requirements.txt
    ```

### Running VisualHmiCleaner 

**VisualHmiCleaner can be run in two Modes**:
  * CronJob - For Background run as a Job Using Docker

    **Building VisualHmiCleaner docker image**:
    ```sh
      $cd /root/IEdgeInsights/
      $cp docker_set
      $docker build -f VisualHmiClient/VisualHmiCleaner/Dockerfile -t visualhmicleaner .
    ```

    **Running VisualHmiCleaner container**:
    ```sh
      $docker run -v /root/IEdgeInsights/VisualHmiClient/VisualHmiCleaner/config.json:/iei/VisualHmiClient/VisualHmiCleaner/config.json -v /root/saved_images:/root/saved_images -v /root/IEdgeInsights/VisualHmiClient/VisualHmiCleaner/logs:/iei/VisualHmiClient/VisualHmiCleaner/logs --privileged=true --network host --name visualhmicleanernew -itd visualhmicleanernew -m docker -cron
    ```
  * Simple Run - For Immediate run.
    ```sh
      cd /root/IEdgeInsights/VisualHmiClient/VisualHmiCleaner
      python3.6 VisualHmiCleaner.py
    ```
