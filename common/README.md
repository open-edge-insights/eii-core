**Contents**

- [eii_libs_installer](#eii_libs_installer)
  - [Running the install script](#running-the-install-script)
    - [Steps:](#steps)
- [Libs package generation](#libs-package-generation)

# eii_libs_installer

This script installs all the EII libraries & their respective required dependencies.

## Running the install script

**Note** :

- The installer needs specific versions of grpc and protobuf to be installed. 
  If these libraries are already present in the `/usr/local/lib` or at the CMAKE_INSTALL_PREFIX env location, 
  the installation will skip build and install of these.  It is recommended to remove grpc before proceeding
  as it can cause version conflicts.

```sh
    sudo apt-get remove --auto-remove --purge -y grpc
```

- Also, make sure all the eii occurances are removed from `/usr/local/lib` and `/usr/local/include`
#### Steps

1. To install all of EII libraries and their dependencies, run the command mentioned below

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

**Note**: If an error occurs during execution of eii_libs_installer.sh regarding the $GOPATH, please set the GOPATH appropriately where GO is installed. Ideally, above script will take care of GO installation and setting up the path. If GO is already installed and GOPATH is tampered, then user has to make sure GOPATH is set appropriately.

2. Incase of unforeseen errors leading to unsuccessful installation, run the command mentioned below to cleanup any/all untracked tar files

    ```sh
        sudo git clean -xdf
    ```

3. Please follow the below steps to set the required env variables while running in baremetal.

    a. For updating the $LD_LIBRARY_PATH env variable:

    ```sh
        export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/opt/intel/eii/lib
        export PATH=$PATH:/usr/local/go/bin
        export GOPATH=~/go
    ```

    **Note**: `$LD_LIBRARY_PATH` should be appended by `$CMAKE_INSTALL_PREFIX/lib`. In the above installation CMAKE_INSTALL_PREFIX is `/opt/intel/eii` and hence LD_LIBRARY_PATH appended with /opt/intel/eii/lib

    b. For updating the $no_proxy env variable to connect to etcd:

    ```sh
        export no_proxy=$no_proxy,127.0.0.1
    ```

# Libs package generation

EII core libraries (EII Message Bus, Config Manager and C Utils) are getting distributed as packages for easier integration with the applications built on top of EII stack so users don't have go through the pain of compiling these libraries from source or copying these built binaries from the EII base images.

The underlying C/C++ EII core libraries are distributed as deb, rpm and apk packages for ubuntu, fedora and alpine OS or docker images respectively. These package installation is a must as this is where the underlying logic lies.

The python binding of these EII core libraries are distributed as wheel packages, we have 2 specific multilinux packages:

1. python3.8 compatible manylinux wheel package for ubuntu
2. pythin3.9 compatible manylinux wheel package for fedora

Due to some challenges with underlying musl C library in alpine, we don't have a compatible python wheel package to work on alpine.

The golang binding of these EII core libraries are distributed as github repos (Config Manager Golang repo: https://github.com/open-edge-insights/eii-configmgr-go and EII MessageBus Golang repo: https://github.com/open-edge-insights/eii-messagebus-go) which gets referenced in the go.mod files of EII services.

**NOTE**:

For now, the EII core libraries distribution is been exercised through the EII sample apps accessible at https://github.com/open-edge-insights/eii-samples.

The script libs_package_generator.sh when run natively on ubuntu 20.04 OS system generates the required packages which can then be hosted to respective locations.

## Pre-requisites:

1. Libraries needs to be installed in the system. One has to follow [eii_libs_installer](#eii_libs_installer) steps before generating packages.

2. Install dependency packages:

    ```sh
        sudo apt install -y rpm
        pip3 install wheel==0.37.1
    
    ```
3. Below are the dependencies for python fedora packages:

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
        sudo rm -rf util/c/build libs/EIIMessageBus/build libs/ConfigMgr/build
    ```

### Steps:

1. After pre-requisites, below script needs to be executed

```sh
    ./libs_package_generator.sh
```

2. After successful execution of above script, all the packages will be available under the folder `./packages`


