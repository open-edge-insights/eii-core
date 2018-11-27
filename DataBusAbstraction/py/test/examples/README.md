# DataBusAbstraction
DataBusAbstraction abstracts underlying messagebus to provide a common set of APIs
The test program demonstrates subscription over OPCUA bus

Available in python
Currently supported messagebus:
1. OPCUA

## Dependencies:
1. OPCUA
   * python2.7 (for python)
   * Install databus_requirements.txt (for python) by running cmd: `sudo -H pip2.7 install -r databus_requirements.txt`
2. MQTT
   * paho-mqtt (python)

## How to Test from present working directory:
A test program is available under ./py/test/examples/
  * Start OPCUA message bus client to listen on topic `classifier_results` by running below cmd in another terminal
  ```
  python2.7 databus_client.py --endpoint opcua://localhost:4840/elephanttrunk --direction SUB --ns streammanager --topic classifier_results
  ```
  * Type "STOP" to stop all subscriptions
  * Type "START" to start all subscriptions again
  * Type "TERM" to end testing
