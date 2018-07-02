# StreamManager
StreamManager package subscribes to InfluxDB and posts topic data to Data bus

Pre-requisites:
1. In Influx terminal, create db by running "create database DA_INTERNAL_DB" and then run: use DA_INTERNAL_DB
2. Insert some records to measurment test_measure Eg: insert test_measure,tag=tag1 a=b,c=d
3. NATS server can be installed in 2 ways:
   * Download go executable at http://nats.io/download/nats-io/gnatsd/
   * Get it by running command **go get github.com/nats-io/gnatsd**
   Start NATS server by running **gnatsd** command. The NATS server (messaging bus) has to be running locally for publish and subscribe to work.
4. Install python dependency:
   ```sh
   sudo -H pip3 install asyncio-nats-client
   ```

## 3 ways to run test file from $GOPATH/src/iapoc_elephanttrunkarch - present working directory
* go run StreamManager/test/strmMgrTest.go
* go build StreamManager/test/strmMgrTest.go && ./strmMgrTest
* go install StreamManager/test/strmMgrTest.go && strmMgrTest

The above test file starts the stream manager's UDP server listening on InfluxDB subscription and publishing the datapoints received to nats server
to topic "test_topic". Start the 2 nats client like below to see messages being received on that topic:
* go run StreamManager/test/natsClient.go -topic="test_topic"
* python3 StreamManager/test/natsClient.py "test_topic"


