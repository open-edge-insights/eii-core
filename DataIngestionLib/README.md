Data Ingestion Module takes the data and sends it to the influx. Currently if the data contains a buffer it stores locally and stores the handle in datapoint. Moving forward instead of storing locally the buffer will be stored in ImageStore.
The Data Ingestion module provides API's to create datapoint and add fields and save the datapoints in influxDB.
To test the Data Ingestion module:
1. Create a database in influxDB by name "datain". Use command
   >> create database datain
2. Copy the video bigbuckbunny.mp4 from shared drive \\vmspfsfsbg01.gar.corp.intel.com\QSD_SW_BA\FOG\test_video and keep in /iapoc_elephanttrunkarch/
3. cd /iapoc_elephanttrunkarch/
   export PYTHONPATH=./
4. Run python3 DataIngestionLib/test/DataIngestionLib_Test.py


