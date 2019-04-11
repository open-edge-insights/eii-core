# DataBusAbstraction

DataBusAbstraction abstracts underlying messagebus to provide a common set of APIs needed for publish and subscribe.

The python example program demonstrates publish and subscription over OPCUA bus only (`OPCUA is the only messagebus supported for now`)

## Dependencies

* python3.6 (for python) installed
* Install databus_requirements.txt (for python) by running cmd: `sudo -H pip3.6 install -r  databus_requirements.txt`
* Install open62541 library dependencies (mbedTLS, python dev):

  ```sh
  sudo apt-get install -y libmbedtls-dev python3.6-dev
  ```

* Set `PYTHONPATH` env variable to DataBusAbstraction/py folder path

  ```sh
  export PYTHONPATH=..
  ```

* Copying certs and keys:
  * Copy opcua client cert and key to /etc/ssl/opcua
  * Copy CA cert to /etc/ssl/ca

## How to Test from present working directory

### 1. Pre-requisite

  ```sh
  make clean
  make build_safestring_lib
  ```

### 2. Testing with security enabled

* Start publisher, publish and destroy

```sh
make pub
```

* Start subscriber, subscribe and destroy

```sh
make sub
```

### 3. Testing with security disabled

* Start publisher, publish and destroy

```sh
make pub_insecure
```

* Start subscriber, subscribe and destroy

```sh
make sub_insecure
```

### 4. Remove all binaries/object files

```sh
make clean
```
