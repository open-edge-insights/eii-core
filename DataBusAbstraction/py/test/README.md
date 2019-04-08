# DataBusAbstraction
DataBusAbstraction abstracts underlying messagebus to provide a common set of APIs needed for publish and subscribe.
The python example program demonstrates publish and subscription over OPCUA bus only (`OPCUA is the only messagebus supported for now`)

## Dependencies:
OPCUA
  * python3.6 (for python)
  * Install databus_requirements.txt (for python) by running cmd: `sudo -H pip3.6 install -r databus_requirements.txt`
  * Install open62541 library dependencies (mbedTLS, python dev):

    ```sh
    sudo apt-get install -y libmbedtls-dev python3.6-dev
    ```
    
  > **Note**: Due to `libmbedtls-dev` package not working with Ubuntu 18.04 for
  >           open62541 clients, so please run the [iei-simple-visualizer](https://gitlab.devtools.intel.com/Indu/IEdgeInsights/iei-simple-visualizer.git)
  >           app's as a container

  * Set `PYTHONPATH` env variable to DataBusAbstraction/py folder path
    ```sh
    export PYTHONPATH=..
    ```
  * Copying certs and keys:
    * Copy opcua client cert and key to /etc/ssl/opcua
    * Copy CA cert to /etc/ssl/ca

    > **Note**: If one wish to provide a diff cert/key path, they can do so by providing the right cert/key path while running `DataBusTest.py` script below

## How to Test from present working directory:

### Start server, publish and destroy

  ```sh
  python3.6 DataBusTest.py -direction PUB -endpoint opcua://localhost:65003 -ns StreamManager \
                            -topic classifier_results \
                            -certFile /etc/ssl/opcua/opcua_server_certificate.der \
                            -privateFile /etc/ssl/opcua/opcua_server_key.der \
                            -trustFile /etc/ssl/ca/ca_certificate.der
  ```

### Start client, subscribe and destroy

  ```sh
  python3.6 DataBusTest.py -direction SUB -endpoint opcua://localhost:65003 -ns StreamManager \
                            -topic classifier_results \
                            -certFile /etc/ssl/opcua/opcua_client_certificate.der \
                            -privateFile /etc/ssl/opcua/opcua_client_key.der \
                            -trustFile /etc/ssl/ca/ca_certificate.der
  ```

> **Note**: Change the opcua endpoint as per the usecase. If one wish to subscribe to IEI opcua bus, then make sure to change the
>           opcua://<IEI_node_ip_address>:4840