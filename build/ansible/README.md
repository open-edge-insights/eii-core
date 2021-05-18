## Ansible based EII Prequisites setup, provisioning, build & deployment

Ansible is the automation engine which can enable EII deployment across single/multi nodes.
We need one control node where ansible is installed and optional hosts. We can also use the control node itself to deploy EII

> **Note**: 
> * Ansible can execute the tasks on control node based on the host information updated in config file.
> * There are 3 types of nodes - control node where ansible must be installed, EII master node and optional worker nodes. Control node and EII master node can be same.

## Installing Ansible on Ubuntu {Control node} 

1.  Execute the following command in the identified control node machine.
    ```sh
        $ sudo apt update
        $ sudo apt install software-properties-common
        $ sudo apt-add-repository --yes --update ppa:ansible/ansible
        $ sudo apt install ansible

        # To maske ssh connection from control node to other nodes
        $ sudo apt install sshpass
    ```
## Updating the hosts Information

*   Update the hosts information in the inventory file `build/ansible/hosts`
    ```
        [group_name]
        <nodename> ansible_host=<ipaddress> ansible_user=<machine_user_name> ansible_ssh_pass=<machine_user_password> ansible_become_pass=<sudo_password>
    ```

    For Eg:
    ```
    [targets]
    master ansible_host=192.0.0.1  ansible_user=user1 ansible_ssh_pass=user1@123 ansible_become_pass=user1@123
    ```
    
    > **Note**: 
    > * The above information is used by ansible to establish ssh  connection to the nodes.
    > * For single node installation, we must provide the same control node details
    > * To deploy EII on multiple nodes, add hosts details to the inventory file



### Steps to encrypt `hosts` file using Password

> **Note**
> Ansible `hosts` file contains the credential information such as host ip, user password & sudo user password.
> It is advised to encrypt the file to make secure.

*   In Control machine, Navigate to `[EII_WORKDIR]/IEdgeInsights/build/ansible` directory and execute following command, 
    ```sh
    $ ansible-vault encrypt hosts
    ```
*   It will prompt `New Password` input to set password for securing `hosts` file.
    For Eg:   

    ```sh
    $ ansible-vault encrypt hosts
    $ New Password:
    <Enter password to encrypt the file>
    ```
    >**Note:** This password should be remembered for decrypting the file & also using with ansible-playbook.
    
### Steps to decrypt `hosts` file using Password
    **Note** This steps is required to decrypt the `hosts` file in to human readable format. 
    So that editing the `hosts` file further is possible.

*   In Control machine, Navigate to `[EII_WORKDIR]/IEdgeInsights/build/ansible` directory and execute following command, 
    ```sh
    $ ansible-vault decrypt hosts
    ```
*   It will prompt `Password` input to decrypt `hosts` file.
    For Eg:   

    ```sh
    $ ansible-vault decrypt hosts
    $ Password:
    <Enter password to decrypt the file>
    ```
    >**Note:** This password should be same as used while encrypting the file.
    > Once file decrypted & edited. For encrypting follow the previous section for `encrypt'.
    > It is recommended to keep the hosts file encrypted always and do the decrypt only for editing it.

## Updating the EII Source Folder, Usecase & Proxy Settings in Group Variables

1. Open `group_vars/all.yml` file
    ```sh
        $ vi group_vars/all.yml
    ```
2. Update EII Source Folder path to `eii_source_path` variable value.
    ```sh
        eii_source_path: "<source_code_path>"
    ```
3. Update Proxy Settings
    ```sh
        http_proxy: <proxy_server_details>
        https_proxy: <proxy_server_details>
        no_proxy: <managed_node ip>,<controller node ip>,<worker nodes ip>,localhost,127.0.0.1
    ```
4. Update the `usecase` variable, based on this `builder.py` generates the EII deployment & config files.
    **Note** By default it will be `video`, For other usecases refer the `build/usecases` folder and update only names without `.yml` extension
    
    For Eg. If you want build & deploy for `build/usecases/video.yml` update the `usecase` key value as `video`
    ```sh
        usecase: <video>
    ```

5.  Save & Close the File.


## Execute ansible Playbook from [EII_WORKDIR]/IEdgeInsights/build/ansible {Control node}

### Steps to execute ansible playbook with `Encrypted hosts` file
> **Note**
> Ansible `hosts` file can be encrypted using `ansible-vault` utility with a password.
> Encrypted `hosts` file can be decrypted while executing playbook using `--ask-vault-pass` argument.

* For running playbook with encrypted `hosts` file. 
    ```sh
    $ ansible-playbook -i hosts eii.yml --ask-vault-pass
    ```
    **Note:** The above step prompts password. The password should be the same used to `encrypt` the `hosts` file.
For Eg:

>* For using encrypted `hosts` file
>    ```sh
>    $ ansible-playbook -i hosts eii.yml --ask-vault-pass
>    ```
>
>  
>* For using Unencrypted `hosts` file
>    ```sh
>    $   ansible-playbook -i hosts eii.yml
>    ```
>

* For EII prequisite, build, provision, deploy & setup master node for multinode deployement usecase
   
    ```$ ansible-playbook -i hosts eii.yml```
  
* For EII Prequisite Setup

    ```$ ansible-playbook -i hosts eii.yml --tags "prerequisites"```
 
* For building EII containers

    ```$ ansible-playbook -i hosts eii.yml --tags "build"```

* Provision EII

    ```$ ansible-playbook -i hosts eii.yml --tags "provision"```

* Deploy selected EII usecase

    ```$ ansible-playbook -i hosts eii.yml --tags "deploy"```

    > **Note**: 
    > * To skip running a particular tag permenantly update `ansible.cfg` under `[tags]` section
