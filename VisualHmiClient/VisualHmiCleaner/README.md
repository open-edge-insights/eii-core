# VisualHmiCleaner Utility:

VisualHmiCleaner is a utiliy to clear the VisualHmi Images & Postgre Entries

> **Note**:
> * VisualHmiCleaner runs on Python2.7

## Pre-requisites:

**VisualHmiCleaner Can be run two Modes**
    * 1. CronJob - For Background run as a Job Using Docker
    * 2. Simple Run - For Immediate run.

## Steps

    * Login as root User `sudo su`  **Mandatory**

    * Clone or Copy `ElephantTrunkArch` Src under `/root/` Directory

    * Configure ViusalHmiCleaner

      ```sh
        vi /root/ElephantTrunkArch/VisualHmiClient/VisualHmiCleaner/config.json
      ```


## Setting Up VisualHMICleaner in CronJob Mode Using Container

    ```sh
      cd /root/ElephantTrunkArch/
    ```

    ```sh
      docker build -f VisualHmiClient/VisualHmiCleaner/Dockerfile -t visualhmicleaner .
    ```
## Update settings in config.json

  "retentiondays" is a numeric value based on this Images & Entries will be cleared from VisualHmi.
    for Eg.
        If retentiondays = 15 . Then Daily it will clean the before 15 Days Images & Entries.

        If retentiondays = 0 . Then Daily it will clean All the Entries.


  "dailyjobscedulehrs" , "dailyjobscedulehrs" is the specific hour & min on which VisualHmiCleaner will be Executed based on retentiondays value.


## RUnning VisualHmiCleaner with CronJob
    ```sh
      docker run -v /root/ElephantTrunkArch/VisualHmiClient/VisualHmiCleaner/config.json:/eta/VisualHmiClient/VisualHmiCleaner/config.json -v /root/saved_images:/root/saved_images -v /root/ElephantTrunkArch/VisualHmiClient/VisualHmiCleaner/logs:/eta/VisualHmiClient/VisualHmiCleaner/logs --privileged=true --network host --name visualhmicleanernew -itd visualhmicleanernew -m docker -cron
    ```
## Steps to run VisualHmiCleaner in Normal

    ```sh
      cd /root/ElephantTrunkArch/VisualHmiClient/VisualHmiCleaner
      python2.7 VisualHmiCleaner.py
    ```
