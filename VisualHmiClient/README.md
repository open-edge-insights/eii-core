VisualHMI EtaDataSync Client  - Python 2.7 Only
    VisualHMIClient is a datasync app which takes classifier results and push to
    VisualHmi Server.VisualHMI Should be run Only on Python2.7 as our Databus Client have python2.7 Compatibilty only.

    VisualHMI starts subscribing on databus topic which will provide the classifier results.
    Based on the Classifier results, VisualHMICLient Converts the data to VisualHMI Standard as a json and
    post the data to VisualHMI.Also VisualHMIClient get the image from the ImageStore Using getblob via GRPC
    and persist the Image in VisualHMI local filesystem.

Pre-Requests:
--> !. Please make sure that the below libraries availability. (either in iapoc_elephanttrunkarch or yourfolder)
        1. Databus Library  
        2. GRPC Client      
    
    Set your PYTHONPATH Based on above libraries placed
    For Eg:
        If it is iapoc_elephanttrunkarch under home:
            echo PYTHONPATH=$PYTHONPATH:~/iapoc_elephanttrunkarch:iapoc_elephanttrunkarch/DataBusAbstraction/py/
        else:
            set your path


--> !. VisualHMI (RefinementApp) Should be running.

VisualHMI Dependencies Installtion:
    
    Install Pre-Requests of VisualHMIClient.
        pip2.7 install -r requirements.txt

Configure Databus & VisualHMI Server Details using config.json

Running VisualHMIClient.

    VisualHMIClient Can be run two Modes

    1. Running VisualHMI in Prod Mode 

        python2.7 VisualHmiEtaDataSync.py
    
    2. Running VisualHMI without HMI Backend (For Testing)

        python2.7 VisualHmiEtaDataSync.py -local <path to store image locally>


Currently VisualHMI Cantbe Exited Directly, Please use following Commands to Terminate the VisualHMI App

    ps -ef | grep VisualHmiEtaDataSync.py | grep -v grep | awk {'print$2'} | xargs kill