# eis_libs_installer

This script installs all the EIS libraries & their respective required dependencies.

## Running the install script

1. As a pre-requisite, please run this one-time command to install the necessary dependencies

    ```sh
        $ cd libs/ConfigMgr && \
          sudo ./install.sh
    ```

2. To install all of EIS libraries and their dependencies, run the command mentioned below

    ```sh
        $ sudo -E ./eis_libs_installer.sh
    ```

3. Incase of unforeseen errors leading to unsuccessful installation, run the command mentioned below to cleanup any/all untracked tar files

    ```sh
        $ git clean -xdf
    ```


4. Please follow the below steps to set the required env variables.

    a. For updating the $LD_LIBRARY_PATH env variable:
    ```sh
        export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib
    ```

    b. For updating the $no_proxy env variable to connect to etcd:
    ```sh
        export no_proxy=$no_proxy,127.0.0.1
    ```