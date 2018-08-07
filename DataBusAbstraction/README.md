# DataBusAbstraction
DataBusAbstraction abstracts underlying messagebus to provide a common set of APIs
Available in python & golang(WIP)
Currently supported messagebus:
1. OPCUA
2. MQTT

## Dependencies:
1. OPCUA
   * python2.7 (for python + golang)
   * Install databus_requirements.txt (for python + golang) by running cmd: `python2.7 install -r databus_requirements.txt`
   * sbinet/go-python (golang)
2. MQTT
   * paho-mqtt (python)
   * eclipse/paho.mqtt.golang (golang)

## How to Test from $GOPATH/src/iapoc_elephanttrunkarch - present working directory:
A test program is available under ./go/test/ and ./py/test/
1. ./py/test/
   * Template for publish:
     * To start the publication test
     ```sh
     python2.7 DataBusAbstraction/py/test/DataBusTest.py --endpoint <address> --direction PUB --ns <a name> --topic <list of topics> --msg <Message to send>

     examples:

     python2.7 DataBusAbstraction/py/test/DataBusTest.py --endpoint opcua://0.0.0.0:65003/ --direction PUB --ns streammanager --topic classifier_results --msg TESTMESSAGE

     python2.7 DataBusAbstraction/py/test/DataBusTest.py --endpoint mqtt://localhost:1883/ --direction PUB --ns streammanager --topic classifier_results --msg TESTMESSAGE
     ```
     * Anything typed after the above step is considered as a message to transfer, except string 'TERM'
     * To end testing type 'TERM'

   * Template for subscribe:
     * To start the subscription test
     ```sh
     python2.7 DataBusAbstraction/py/test/DataBusTest.py --endpoint <address> --direction SUB --ns <a name> --topic <list of topics>

     examples:

     python2.7 DataBusAbstraction/py/test/DataBusTest.py --endpoint opcua://localhost:65003/ --direction SUB --ns streammanager --topic classifier_results

     python2.7 DataBusAbstraction/py/test/DataBusTest.py --endpoint mqtt://localhost:1883/ --direction SUB --ns streammanager --topic classifier_results
     ```
     * Type "STOP" to stop all subscriptions
     * Type "START" to start all subscriptions again
     * Type "TERM" to end testing
2. ./go/test (TODO)
