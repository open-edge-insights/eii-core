## Step 1 - Install Clear Linux OS
The first step is to install Clear Linux on your gateway. Follow the instructions
at [this](https://clearlinux.org/documentation/clear-linux/get-started/bare-metal-install)
link.

**IMPORTANT:** Use [version 24330](https://download.clearlinux.org/releases/24330/clear/clear-24330-installer.iso.xz) 
of Clear Linux.

## Step 2 - Install Clear Linux Bundles

1. Disable `swupd` auto update
    ```sh
    swupd autoupdate --disable
    ```
2. Double check to assure that Clear Linux version 22120 is installed
    ```sh
    swupd verify --fix -m 22120 --picky
    ```

3. Install the following Clear Linux bundles:

    * network-basic
    * openssh-server
    * os-core-dev
    * database-basic
    * dev-utils-dev
    * sysadmin-basic

    The command for installing a bundle is the following:
    ```sh
    $ swupd bundle-add network-basic openssh-server os-core-dev database-basic dev-utils-dev sysadmin-basic
    ```

4. Configure static IP address(192.168.1.x/24) on the gateway by following the instructions
    outlined [here](https://clearlinux.org/documentation/clear-linux/reference/bundles/os-core)

5. Setup the SSH server, and add SSH public key to the authorized_keys file. From all computers that you wish to ssh from, enter the following command and respond to all prompts:

    ```sh
    $ ssh-copy-id [name or IP address]
    ```

## Step 3 - Add users
1.  Add user `intel`
    ```sh
    $ sudo adduser intel
    ```

2.  Change new user's password to `intel123`. Enter the following command and responp to all prompts  
    ```sh
    $ passwd intel
    ```

3.  Configure Linux to allow user `intel` to use the sudo command:
    ```sh
    $ sudo usermod -a -G wheel intel
    ```    

4.  Optional, if located inside Intel's campus: Set up proxy (`Depending on the geo location where the system is setup, please use the proxy settings of that geo. Th
is change may not be needed in  non-proxy environment`):
    ```sh
    $ sudo su
    $ echo "use_proxy=on 
    http_proxy=http://proxy.iind.intel.com:911 
    https_proxy=https://proxy.iind.intel.com:911" > /home/intel/.wgetrc
    $ chown $LINUX_USER:$LINUX_USER /home/$LINUX_USER/.wgetrc
    $ exit
    ``` 
