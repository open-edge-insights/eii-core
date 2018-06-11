# DataIngestionLib (DIL):

Data Ingestion Module takes the data and sends it to the influx. If the data contains a buffer, it uses ImageStore module to write to ImageStore and stores the handle of that in datapoint. The Data Ingestion module provides APIs to create datapoint, add fields and save the datapoints in influxDB.

## Running test file from $GOPATH/src/iapoc_elephanttrunkarch - present working directory
* Run DataAgent with the config file having the right configs for InfluxDB and Redis (DIL would be fetching these configs over grpc call). In otherwords,
  go run DataAgent/DataAgent.go -config=<config_file> -log_dir=<log_dir>
* python3 DataIngestionLib/test/DataIngestionLib_Test.py