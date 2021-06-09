## Ansible based EII Prequisites setup, provisioning, build & deployment

Ansible is the automation engine which can enable EII deployment across single/multi nodes.
We need one control node where ansible is installed and optional hosts. We can also use the control node itself to deploy EII

> **Note**: 
> * Ansible can execute the tasks on control node based on the playbooks defined
> * There are 3 types of nodes - control node where ansible must be installed, EII leader node where ETCD server will be running and optional worker nodes, all worker nodes remotely connect to ETCD server running on leader node. Control node and EII leader node can be same.

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

## Prerequisite step needed for all the control/worker nodes.
### Generate SSH KEY for all nodes

Generate the SSH KEY for all nodes using following command (to be executed in the respective node)

```sh
    $ ssh-keygen
```
> **Note**:
>   * Dont give any passphrase and id, just press `Enter` for all the prompt which will generate the key.
>   * The above step should be done for all the machines including `control` node.

For Eg.
```sh
$ ssh-keygen

Generating public/private rsa key pair.
Enter file in which to save the key (/root/.ssh/id_rsa):  <ENTER>
Enter passphrase (empty for no passphrase):  <ENTER>
Enter same passphrase again:  <ENTER>
Your identification has been saved in ~/.ssh/id_rsa.
Your public key has been saved in ~/.ssh/id_rsa.pub.
The key fingerprint is:
SHA256:vlcKVU8Tj8nxdDXTW6AHdAgqaM/35s2doon76uYpNA0 root@host
The key's randomart image is:
+---[RSA 2048]----+
|          .oo.==*|
|     .   .  o=oB*|
|    o . .  ..o=.=|
|   . oE.  .  ... |
|      ooS.       |
|      ooo.  .    |
|     . ...oo     |
|      . .*o+.. . |
|       =O==.o.o  |
+----[SHA256]-----+
```

### Configure Sudoers file to accept NO PASSWORD for sudo operation.

> **Note**: Ansible need to execute some commands as `sudo`. The below configuration is needed so that `passwords` need not be saved in the ansible `hosts` file as a variable.

Update `sudoers` file

1. Open the `sudoers` file.
    ```sh
        $ sudo visudo
    ```
2. Append the following to the `sudoers` file
   ```sh
       $ <ansible_non_root_user>  ALL=(ALL:ALL) NOPASSWD: ALL
   ```
   For E.g:

   In control node the executing not root user name is `user1`, you should append as follows
   ```sh
      $ user1 ALL=(ALL:ALL) NOPASSWD: ALL
   ```
   Now the above line authorizes `user1` user to do `sudo` operation in the control node without `PASSWORD` ask.
   > **Note**: The same procedure applies to all other nodes where ansible connection is involved.


3. Save & Close the file

## Adding SSH Authorized Key of all the nodes to control node

Please follow the steps to copy the generated keys from all other nodes to the control node.
1. Execute the following command from `control` node.
```sh
    $ ssh-copy-id <USER_NAME>@<HOST_IP>
```

For Eg.
```sh
    $ ssh-copy-id test@192.0.0.1

    /usr/bin/ssh-copy-id: INFO: Source of key(s) to be installed: "/home/<username>/.ssh/id_rsa.pub"

    Number of key(s) added: 1

    Now try logging into the machine, with:   "ssh 'test@192.0.0.1'"
    and check to make sure that only the key(s) you wanted were added.
```
## Updating the Leader & Worker node Information for using remote hosts

>**Note**: By `default` both control/leader node `ansible_connection` will be `localhost` for executing in same node.

Please follow below steps to update the details of leader / worker nodes for multi node scenario.

*   Update the hosts information in the inventory file `build/ansible/hosts`
    ```
        [group_name]
        <nodename> ansible_connection=ssh ansible_host=<ipaddress> ansible_user=<machine_user_name>
    ```

    For Eg:
    ```
    [targets]
    leader ansible_connection=ssh ansible_host=192.0.0.1  ansible_user=user1
    ```
    
    > **Note**: 
    > * `ansible_connection=ssh` is mandatory when you are updating any remote hosts, which makes ansible to connect via `ssh`.
    > * The above information is used by ansible to establish ssh  connection to the nodes.
    > * control node will always be `ansible_connection = local`, Don't update the control node
    > * For single node installation, control and leader nodes can be same or different
    > * To deploy EII on multiple nodes, add hosts(worker1, worker2 etc..) details to the inventory file

### Updating docker registry details in hosts file

Update the below information for using docker registry for deploying the images from control node to other nodes in multi node scenario.

1. Open the `hosts` file.
    ```sh
        $ vi hosts
    ```
2. Update `docker` registry details in following section
    ```yml
        [targets:vars]
        # Varibles to login to docker registry
        docker_registry="<regsitry_url>"
        docker_login_user="<username>"
        docker_login_passwd="<password>"
    ```
    > **Note**:
    >    1. If you `registry` is a no password  registry, not required to update the `docker_login_user` & `docker_login_passwd` details.

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
2. Update Proxy Settings
    ```sh
        http_proxy: <proxy_server_details>
        https_proxy: <proxy_server_details>
        no_proxy: <managed_node ip>,<controller node ip>,<worker nodes ip>,localhost,127.0.0.1
    ```
3. Update the `usecase` variable, based on this `builder.py` generates the EII deployment & config files.
    **Note** By default it will be `video`, For other usecases refer the `build/usecases` folder and update only names without `.yml` extension
    
    For Eg. If you want build & deploy for `build/usecases/video.yml` update the `usecase` key value as `video`
    ```sh
        usecase: <video>
    ```
4. Optionally you can choose number of video pipeline instances to be created by updating `instances`

5. Set `multi_node` to `true` to enable multinode deployment without k8s and `false` to enable single node deployment. Update `vars/vars.yml` to the services to run on a specific node in case of multi_node deployment by following [this](#Select-EII-services-to-run-on-a-particular-node-in-multinode-deployment), where in single node deployment all the services based on the `usecase` chosen will be deployed. 

6. Update other optional variables provided if required

## Select EII services to run on a particular node in multinode deployment

* Edit `vars/vars.yml` -> under `nodes` add a specific a node which was defined in the inventory file(`hosts`) and add EII services to `include_services` list

    Eg. if you want leader to run ia_video_ingesstion, `vars/vars.yml` should be

    ```yml
        nodes:
          leader:
            include_services:
                - ia_video_ingestion
    ```

* If you want to add `worker1` to `nodes` and bring up `ia_visualizer` in `worker1`:

    ```yml
    nodes:
      leader:
        include_services:
            - ia_video_ingestion
      worker1:
        include_services:
            - ia_visualizer
    ```


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

## Execute ansible Playbook from [EII_WORKDIR]/IEdgeInsights/build/ansible {Control node}

*  For Single Point of Execution
   > **Note**: This will execute all the steps of EII as prequisite, build, provision, deploy & setup leader node for multinode deployement usecase in one shot sequentialy.
    > * Updating messagebus endpoints to connect to interfaces is still the manual process. Make sure to update Application specific endpoints in `[AppName]/config.json`

    ```sh
    $ ansible-playbook -i hosts eii.yml
    ```
  
 > **Note**: Below steps are the individual execution of setups.
* For EII Prequisite Setup

    ```sh
    $ ansible-playbook -i hosts eii.yml --tags "prerequisites"
    ```
 
* For building EII containers

    ```sh
    $ ansible-playbook -i hosts eii.yml --tags "build"
    ```

* To generate Certificates in control node
    ```sh
    $ ansible-playbook -i hosts eii.yml --tags "gen_certs"
    ```

* To generate provision bundle for leader

    ```sh
    $ ansible-playbook -i hosts eii.yml --tags "gen_leader_provision_bundle"
    ```

* To generate provision bundle for worker

    ```sh
    $ ansible-playbook -i hosts eii.yml --tags "gen_worker_provision_bundle"
    ```

* To generate eii bundles for leader, worker nodes

    ```sh
    $ ansible-playbook -i hosts eii.yml --tags "gen_bundles"
    ```

* provision leader and bring up ETCD server

    ```sh
    $ ansible-playbook -i hosts eii.yml --tags "leader_provision"
    ```

* provision worker node

    ```sh
    $ ansible-playbook -i hosts eii.yml --tags "worker_provsion"
    ```
* Provision EIS Config values to etcd

    ```sh
    $ ansible-playbook -i hosts eii.yml --tags "etcd_provision"
    ```

* To generate eii bundles for leader, worker nodes

    ```sh
    $ ansible-playbook -i hosts eii.yml --tags "deploy"
    ```

    > **Note**: 
    > * To skip running a particular tag permenantly update `ansible.cfg` under `[tags]` section

### Deploying EII Using Helm in Kubernetes (k8s) environment

> **Note**
>   1. To Deploy EII using helm in k8s aenvironment, `k8s` setup is a prerequisite.
>   2. You need update the `k8s` leader machine as leader node in `hosts` file.
>   3. Non `k8s` leader machine the `helm` deployment will fail.

* Update the `DEPLOYMENT_MODE` flag as `k8s` in `group_vars/all.yml` file:
    *   Open `group_vars/all.yml` file
    ```sh
        $ vi group_vars/all.yml
    ```
    *   Update the `DEPLOYMENT_MODE` flag as `k8s`

    ```yml
        ## Deploy in k8s mode using helm
        DEPLOYMENT_MODE: "k8s"
    ```

    * Save & Close

*  For Single Point of Execution
   > **Note**: This will execute all the steps of EII as prequisite, build, provision, deploy for a usecase in one shot sequentialy.

    ```sh
    $ ansible-playbook -i hosts eii.yml
    ```

> **Note**: Below steps are the individual execution of setups.
* For EII Prequisite Setup

    ```sh
    $ ansible-playbook -i hosts eii.yml --tags "prerequisites"
    ```

* For building EII containers

    ```sh
    $ ansible-playbook -i hosts eii.yml --tags "build"
    ```

* To generate Certificates in control node
    ```sh
    $ ansible-playbook -i hosts eii.yml --tags "gen_certs"
    ```

* Prerequisites for deploy EII using Ansible helm environment.
    ```sh
    $ ansible-playbook -i hosts eii.yml --tags "helm_k8s_prerequisites"
    ```
* Provision & Deploy EII Using Ansible helm environment
    ```sh
    $ ansible-playbook -i hosts eii.yml --tags "helm_k8s_deploy"
    ```
