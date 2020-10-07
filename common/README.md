# eis_libs_installer

This script installs all the EIS libraries & their respective required dependencies.

## Running the install script

1. As a pre-requisite, please run these commands to install the necessary dependencies

    ```sh
        $ cd libs/ConfigMgr/src/kv_store_plugin && \
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

4. Run this command to update the $LD_LIBRARY_PATH

    ```sh
        $ export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib
    ```