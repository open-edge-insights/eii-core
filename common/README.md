# Contents

- [Contents](#contents)
  - [eii_libs_installer](#eii_libs_installer)
  - [Running the install script](#running-the-install-script)
    - [Install OEI libraries](#install-oei-libraries)
  - [Libs package generation](#libs-package-generation)
  - [Prerequisites](#prerequisites)
    - [Steps](#steps)

## eii_libs_installer

This script installs all the Open Edge Insights (OEI) libraries & their respective required dependencies.

>**Note:** In this document, you will find labels of 'Edge Insights for Industrial (EII)' for filenames, paths, code snippets, and so on. Consider the references of EII as OEI. This is due to the product name change of EII as OEI.

## Running the install script

**Note:**

- The installer needs specific versions of grpc and protobuf to be installed.
  If these libraries are already present in the `/usr/local/lib` or at the `CMAKE_INSTALL_PREFIX` env location, the installation will skip building and installing the libraries. It is recommended to remove grpc before proceeding, as it can cause version conflicts.

  ```sh
    sudo apt-get remove --auto-remove --purge -y grpc
  ```

- Also, ensure that all the OEI occurrences are removed from `/usr/local/lib` and `/usr/local/include`

### Install OEI libraries

1. To install all of OEI libraries and their dependencies, run the following commands:

    ```sh
    sudo apt-get update
    ```

    For Ubuntu-20.04

    ```sh
    sudo apt-get install -y libcjson-dev libzmq3-dev zlib1g-dev
    ```

    For Ubuntu-18.04

    ```sh
    sudo apt-get install -y libjson-c-dev libzmq3-dev zlib1g-dev
    ```

    ```sh
    mkdir -p /opt/intel/eii/
    sudo -E CMAKE_INSTALL_PREFIX="/opt/intel/eii" ./eii_libs_installer.sh
    ```

    > **Note:** During the execution of `eii_libs_installer.sh`, if any $GOPATH-related error occurs then, set the `GOPATH` appropriately, where GO is installed. Ideally, the above script will take care of the GO installation and setting up the path. If GO is already installed, and GOPATH is tampered, then ensure to set GOPATH appropriately.

2. For unforeseen errors leading to unsuccessful installation, run the following command to cleanup any or all untracked tar files:

    ```sh
        sudo git clean -xdf
    ```

3. Complete the following steps to set the required env variables while running in bare metal:

    a. For updating the $LD_LIBRARY_PATH env variable:

    ```sh
        export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/opt/intel/eii/lib
        export PATH=$PATH:/usr/local/go/bin
        export GOPATH=~/go
    ```

    >**Note:** `$LD_LIBRARY_PATH` should be appended by `$CMAKE_INSTALL_PREFIX/lib`. In the above installation CMAKE_INSTALL_PREFIX is `/opt/intel/eii` and hence LD_LIBRARY_PATH appended with /opt/intel/eii/lib

    b. For updating the $no_proxy env variable to connect to etcd:

    ```sh
        export no_proxy=$no_proxy,127.0.0.1
    ```

## Libs package generation

OEI Core libraries (Message Bus, Config Manager and C Utils) are getting distributed as packages for easier integration with the applications built on top of OEI stack, so users don't have go through the pain of compiling these libraries from source or copying these built binaries from the OEI base images.

The underlying C/C++ OEI core libraries are distributed as deb, rpm and apk packages for ubuntu, fedora and alpine OS or docker images respectively. This package installation is a must as this is where the underlying logic lies.

The Python binding of these OEI core libraries are distributed as wheel packages, we have 2 specific multi-linux packages:

1. Python3.8 compatible manylinux wheel package for Ubuntu
2. Python3.9 compatible manylinux wheel package for Fedora

Due to some challenges with underlying musl C library in alpine, we don't have a compatible Python wheel package to work on alpine.

The golang binding of these OEI core libraries are distributed as GitHub repos [Config Manager Golang repo](https://github.com/open-edge-insights/eii-configmgr-go) and [MessageBus Golang repo](https://github.com/open-edge-insights/eii-messagebus-go) which gets referenced in the go.mod files of OEI services.

>**Note:**

For now, the core libraries distribution is exercised through the OEI sample apps accessible at [sample Apps](https://github.com/open-edge-insights/eii-samples).

The script libs_package_generator.sh when run natively on Ubuntu 20.04 OS system generates the required packages which can then be hosted to respective locations.

## Prerequisites

1. Libraries need to be installed in the system. Before generating packages, follow the steps for the [eii_libs_installer](#eii_libs_installer).

2. Install dependency packages:

    ```sh
        sudo apt install -y rpm
        pip3 install wheel==0.37.1
     ```

3. Below are the dependencies for Python Fedora packages:

    ```sh
        sudo apt install -y python3.9
        sudo apt install -y python3.9-venv
        python3.9 -m venv fedora_package
        source fedora_package/bin/activate
        sudo apt-get install python3.9-dev
        deactivate
    ```

4. Removal of root build folders of libraries.

    ```sh
        sudo rm -rf util/c/build libs/EIIMessageBus/build libs/ConfigMgr/build libs/ConfigMgr/python/build libs/EIIMessageBus/python/build
    ```

### Steps

1. After completing the prerequisites, run the following command

   ```sh
      CMAKE_INSTALL_PREFIX="/opt/intel/eii" ./libs_package_generator.sh
   ```

2. After successful execution of above script, all the packages will be available under the folder `./packages`
