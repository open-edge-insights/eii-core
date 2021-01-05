# eis_libs_installer

This script installs all the EIS libraries & their respective required dependencies.

## Running the install script

**Note** : The installer needs specific versions of grpc and protobuf to be installed. If these libraries are already present in the /usr/local/lib, the installation will skip build and install of these. It is recommended to remove the libgrpc*.a and libproto*.a from /usr/local/lib before proceeding as it can cause version conflicts.

```sh
    $ sudo rm -rf /usr/local/lib/libgrpc*.a /usr/local/lib/libproto*.a
```

1. To install all of EIS libraries and their dependencies, run the command mentioned below

    ```sh
        $ sudo -E ./eis_libs_installer.sh
    ```

2. Incase of unforeseen errors leading to unsuccessful installation, run the command mentioned below to cleanup any/all untracked tar files

    ```sh
        $ sudo git clean -xdf
    ```

3. Please follow the below steps to set the required env variables.

    a. For updating the $LD_LIBRARY_PATH env variable:
    ```sh
        export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib
    ```

    b. For updating the $no_proxy env variable to connect to etcd:
    ```sh
        export no_proxy=$no_proxy,127.0.0.1
    ```