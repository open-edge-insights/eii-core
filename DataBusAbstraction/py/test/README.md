# DataBusAbstraction
DataBusAbstraction abstracts underlying messagebus to provide a common set of APIs
The test program demonstrates subscription over OPCUA bus

Available in python
Currently supported messagebus:
1. OPCUA

## Dependencies:
1. OPCUA
   * python3.6 (for python)
   * Install databus_requirements.txt (for python) by running cmd: `sudo -H pip3.6 install -r databus_requirements.txt`
2. MQTT
   * paho-mqtt (python)

## How to Test from present working directory:
A test program is available under DataBusAbstraction/py/test/
  * Start OPCUA message bus client to listen on topic `classifier_results` by running below cmd in another terminal
  ```
  python3.6 DataBusTest.py  -endpoint opcua://localhost:4840/elephanttrunk -direction SUB -ns streammanager -topic classifier_results -certFile cert-tool/Certificates/opcua/opcua_client_certificate.der -privateFile cert-tool/Certificates/opcua/opcua_client_key.der -trustFile cert-tool/Certificates/ca/ca_certificate.der
  ```