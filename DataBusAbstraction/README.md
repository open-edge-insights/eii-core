# DataBusAbstraction
DataBusAbstraction abstracts underlying messagebus to provide a common set of APIs.
Available in python & golang

Currently supported messagebus (Only OPCUA is been tested so far):
1. OPCUA
2. MQTT

## Dependencies:   
1. Install databus py dependencies:
  ```sh
  cd <repo>/DataBusAbstraction/py
  sudo -H pip3.6 install -r databus_requirements.txt
  ```
2. Install open62541 library dependencies (mbedTLS, python dev):

  > **Note**: This works only on Ubuntu OS. On ClearLinux or any other OS, please  
  >           run the dockerized version of the DataBusAbstraction client like 
  >           `VisualHmiClient` app

  ```sh
  sudo apt-get install -y libmbedtls-dev python3.6-dev
  ```

## How to Test from $GOPATH/src/ElephantTrunkArch - present working directory:
A test program is available under ./go/test/ and ./py/test/ and only works for `OPCUA` now, `MQTT` channel needs to be tested and a generic test program would be provided soon.

1. **`./py/test/`**

### Compilation steps (current directory: <repo>/DataBusAbstraction/go/test):

#### For building the `open62541W.so` and `open62541W.c` files:

  ```sh
  make build
  ```

#### Start server, publish and destroy 

  ```sh
  make pub
  ```

#### Start client, subscribe and destroy

1. Passing the same ca_cert.der that was used to sign the server certificate used for
   started the server above

  ```sh
  make sub
  ```

2. Passing the different ca_cert.der that was `not` used to sign the server certificate used for
   started the server above

  ```sh
  make invalid_sub
  ```

#### Remove all binaries/object files

  ```sh
  make clean
  ```

2. ./go/test/
### Compilation steps (current directory: <repo>/DataBusAbstraction/go/test):

#### For building the `libopen62541W.a` and `test/DataBusTest.go` files:

  ```sh
  make build
  ```

#### Start server, publish and destroy 

  ```sh
  make pub
  ```

#### Start client, subscribe and destroy

> **Note**: This subscription part is not integrated with DataBusAbstraction

1. Passing the same ca_cert.der that was used to sign the server certificate used for
   started the server above

  ```sh
  make sub
  ```

2. Passing the different ca_cert.der that was `not` used to sign the server certificate used for
   started the server above

  ```sh
  make invalid_sub
  ```

#### Remove all binaries/object files

  ```sh
  make clean
  ```
  