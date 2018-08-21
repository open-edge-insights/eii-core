## Prerequisites

1. Download the CentOS 7 minimal ISO from [here](http://isoredirect.centos.org/centos/7/isos/x86_64/CentOS-7-x86_64-Minimal-1804.iso)
2. Put the ISO onto a USB thumb drive
3. Install CentOS onto your system
    > **NOTE:** During the setup set the root password to `intel123`. There is
    > no need to create another user yet at this point.

    > **NOTE:** This instructions only set up a blank database.  The database will need to be populated with defects for the refinement app to show anything

## Step 1 - Configure Proxy and other key environment vars

Add one of the following based on your geo to the `/etc/environment` file.

**PRC**
```
http_proxy=http://child-prc.intel.com:913
https_proxy=http://child-prc.intel.com:913
no_proxy=localhost,*.intel.com
LD_LIBRARY_PATH=/usr/local/lib
PKG_CONFIG_PATH=/usr/local/lib/pkgconfig
```

**US**
```
http_proxy=http://proxy-chain.intel.com:911
https_proxy=http://proxy-chain.intel.com:912
no_proxy=localhost,*.intel.com
LD_LIBRARY_PATH=/usr/local/lib
PKG_CONFIG_PATH=/usr/local/lib/pkgconfig
```
> **NOTE:** You must either reboot the computer to enable these environmental variables or you can export all of them  

## Step 2 - Apply OS Updates
> **NOTE:** This also applies the proxy to YUM for future package installs.

```sh
$ yum -y update
```

## Step 3 - Install Yum Packages 
> **Sources:** [Python 3 Installation](https://www.digitalocean.com/community/tutorials/how-to-install-python-3-and-set-up-a-local-programming-environment-on-centos-7), [Nginx Installation](https://www.digitalocean.com/community/tutorials/how-to-install-nginx-on-centos-7)

```sh
# Add repo for Python 3.6
$ yum -y install https://centos7.iuscommunity.org/ius-release.rpm

# Add repo for nginx, tmux, and vim
$ yum -y install epel-release

# Install all dependency packages
$ yum -y install python36u-pip python36u python36u-devel.x86_64 python-devel nginx tmux vim wget 

# Install Postgres on CentOS 7
$ yum -y install postgresql-server postgresql-contrib

# Install OpenCV Development Packages
$ yum -y install cmake gcc gtk2-pkconfig
$ yum -y install unzip
$ yum install uuid-devel
$ yum install openssl-devel

$ pip3.6 install numpy
```

## Step 4 - Install Refinement App backend

1. Install OpenCV

    ```sh
    cd /root
    wget https://github.com/opencv/opencv/archive/3.4.1.zip
    unzip 3.4.1.zip
    cd opencv-3.4.1
    mkdir build
    cd build
    cmake -DCMAKE_BUILD_TYPE=Release -DPYTHON3_EXECUTABLE=`which python3.6` -DPYTHON_DEFAULT_EXECUTABLE=`which python3.6` -DENABLE_PRECOMPILED_HEADERS=OFF -DCMAKE_CXX_FLAGS=-std=c++11 ..
    make -j4
    make install
    ln -s /usr/local/lib/python3.6/site-packages/cv2.cpython-36m-x86_64-linux-gnu.so /usr/lib/python3.6/site-packages
    ```

2. Initialize and start Postgresql.

    ```sh
    postgresql-setup initdb
    systemctl start postgresql

    sudo -u postgres psql -c "alter user postgres with password 'intel123';"
    sudo -u postgres  psql -c "CREATE USER ali WITH PASSWORD 'intel123'"
    sudo -u postgres createdb ali
    ```

    Also, Tweak files pg_hba.conf and postgresql.conf to enable local and network access

    ```sh
    sed -i 's|local[ ]*all[ ]*all[ ]*peer|local   all             all                                     md5|' /var/lib/pgsql/data/pg_hba.conf
    sed -i 's|host[ ]*all[ ]*all[ ]*127.0.0.1/32[ ]*ident|host    all             all             127.0.0.1/32            md5|' /var/lib/pgsql/data/pg_hba.conf
    sed -i 's|host[ ]*all[ ]*all[ ]*::1/128[ ]*ident|host    all             all             ::1/128                 md5|' /var/lib/pgsql/data/pg_hba.conf

    sed -i '/IPv4 local connections/a \
    host    all             all             0.0.0.0/0               md5' /var/lib/pgsql/data/pg_hba.conf

    sed -i "/listen_addresses = 'localhost'/ilisten_addresses = '*'
    " /var/lib/pgsql/data/postgresql.conf
    ```
    > **NOTE:** This is not meant to be _secure_ but rather to allow database connections using the supplied username from localhost.

    To create the initial data base go to the link bellow and copy it to a file called "initial_db.sql"

    https://github.intel.com/Alibaba-Pathfinding/march-poc/wiki/Edge-server-setup-script#file-initial_dbsql

    once you have created this file run the command 
    ```sh
    psql -U ali ali < initial_db.sql
    ```

    Restart the database engin and enable it to start automatically:
    ```sh
    systemctl enable postgresql
    systemctl restart postgresql
    ```

3. Open port 5000 on the firewall

	```sh
    firewall-cmd --zone=public --add-port=5000/tcp --permanent
    firewall-cmd --reload
    ```

4. Clone or unzip the backend repository

    If you are connected to the Intel network, then obtain a copy of the code via the following git commands.  If you are not connected to the Intel network, then you must install the backend via zip file.  

    **If connected to Intel network:**
    ```sh
    cd /root
    git clone https://github.intel.com/Alibaba-Pathfinding/refinement-app-backend.git
    ```

    **If not connected to Intel Network:**
    ```sh
    mkdir ~/refinement-app-backend/
    unzip \path\to\zipfile -d ~/refinement-app-backend
    ```
5. Install backend dependencies:
    > **NOTE:** This will also install the WSGI server

    ```sh
    cd ~/refinement-app-backend/
    pip3.6 install -r requirements.txt
    ```

6. Create a folder for the saved images
    ```sh
    mkdir /root/saved_images
    ```

7. Edit the configuration file **refinement-app-backend/config.json**
    * "image_folder" should point to "/root/saved_images"
    * Database information should use the settings from above.

8. Create ini file for uwsgi
    ```
    mkdir /etc/uwsgi/

    echo "[uwsgi]
    https-websockets = true
    https = 0.0.0.0:5000,/etc/pki/nginx/server.crt,/etc/pki/nginx/private/server.key
    chmod-socket = 666
    chdir = /root/refinement-app-backend
    master = true
    wsgi-file = /root/refinement-app-backend/wsgi.py
    logto = /var/log/backend-uwsgi.log
    " > /etc/uwsgi/refinement-app-backend.ini
    ```

9. Create a systemd service file for the uwsgi app.  
    Create a file: /usr/lib/systemd/system/refinement_app_backend.service and place the following in this file:

    ```sh
    [Unit]
    Description=Refinement App Backend
    After=network.target

    [Unit]
    Description=Refinement App Backend
    After=network.target

    [Service]
    Type=simple
    User=root
    ExecStart=/usr/bin/uwsgi --ini /etc/uwsgi/refinement-app-backend.ini --manage-script-name --mount /=swagger_server:app
    RuntimeDirectory=uwsgi
    KillSignal=SIGQUIT

    [Install]
    WantedBy=multi-user.target
    ```
10. set up symbolic link, Start the backend and enable it to start automatically:
    ```sh
    mkdir /usr/include/uuid
    ln -s /usr/include/uuid.h /usr/include/uuid/uuid.h
    systemctl enable refinement_app_backend
    systemctl start refinement_app_backend
    ```

    > **NOTE:** To view the realtime output of the backend, enter the following command:
    ```sh
    journalctl -f -u refinement_app_backend
    ```

## Step 5 - Install Refinement App 

> **NOTE:** If you are connected to the Intel network, then you can install the Angular infrastructure, clone the repository, and compile the frontend code.  If you are not connected to the Intel network, or if desired, you can skip all of these steps and obtain a copy of the compiled code via a zipfile.  To install the forntend via zip file, skip to step 7

1. Download version 10.5 of NodeJS
    ```sh
    cd /root
    curl --silent --location https://rpm.nodesource.com/setup_10.x | sudo bash -
    ```
2. Install Node, Git and the nginx web server
    ```sh
    yum install -y nodejs
    ```

3. Install the Angular sdk. Angular has to be installed globally for the ng tool to work.
    ```sh
    npm install -g @angular/cli@6.0.3
    ```

4. Download the repository from git
    ```sh
    git clone https://github.intel.com/Alibaba-Pathfinding/refinement-app.git
    ```

5. Install frontend dependencies
    ```sh  
    cd refinement-app/
    npm install --save classlist.js
    ```

6. Build the app into the dist/ directory.
    ```sh
    ng build --prod
    ```

    > **NOTE:** If you are installing the frontend from the git repository, skip step 7, proceed to step 8

7. Create a folder and unzip the compiled frontend code.  
    ```sh
    mkdir -p ~/refinement-app/dist/defect-identification
    unzip /path/to/zipfile -d ~/refinement-app/dist/defect-identification
    ```

8. Copy files to where nginx will serve the app.

    ```sh
    /usr/bin/cp -R ~/refinement-app/dist/defect-identification/* /usr/share/nginx/html/
    ```
    > **NOTE:** use /usr/bin/cp because CentOS alias 'cp' to 'cp -i'


9. Open port 80 on the firewall
    ```sh
    firewall-cmd --zone=public --add-port=80/tcp --permanent
    firewall-cmd --zone=public --add-port=443/tcp --permanent
    firewall-cmd --add-service=https --permanent
    firewall-cmd --reload
    ```

10. Enable Nginx to proxy pass requests to the backend
    ```sh
    setsebool httpd_can_network_connect 1 -P

    sed -i '/^[ ]*error_page[ ]*404/i \
            location /api/ { \
               proxy_pass      http://127.0.0.1:5000; \
            } \
    ' /etc/nginx/nginx.conf
    ```

11. Turn on the web server and make make sure it runs at boot.
    ```sh
    systemctl enable nginx
    systemctl start nginx
    ```

    ### Change Language Text
    Language files are located in the **src/assets/lang/** directory. Currently the languages available are

    * en_us.json for English (US)
    * zh_cn.json for Chinese (Simplified)

    Open these files to alter and/or append text. Do not change the label id without making a corresponding change to both the html/javascript file as well as any other language files. To add a new language, place the the file in the src/assets/lang/ directory and make an addition to the **src/app/view-settings-dialog/view-settings-dialog.component.ts** file **languages** variable.

## Step 6 - Setup user with SSH key for ETR rsync service

1. Generate Edge server keys and configure ssh
    ```sh
    mkdir ~/.ssh
    cd ~/.ssh
    ssh-keygen -f id_rsa -t rsa -N ''
    touch authorized_keys
    ```

2. Copy the public key from the gateway running ETR to the Edge Server:
    >*NOTE** The next step is performed on the gateway computer running ETR.  It is **NOT** performed on the Edge Server
    ```sh
    sudo su
    ssh-copy-id *Edge Sevrer DNS name or IP address*
    ssh *Edge Server DNS name or IP address*
    exit
    exit
    ```

    >**NOTE:** If you are deploying the edge server into a factory environment, then you must configure SSH to only allow access to the system via SSH keys and you should remove the proxy environmental variables from the /etc/environment file.

    1. Open the /etc/ssh/sshd_config file,set PasswordAuthentication value to no

    2. Restart the sshd service:
    ```sh
    sudo systemctl restart sshd
    ```

## Step 7: Create Certificates
In refinement-app-backend branch staging has a folder called ssl. This folder holds all files that you will run.

1. Modify the .cnf files Located in the input_files folder

    rootCA.pem.cnf configuration file for root certificate authority, identifies who the root authority is.
    rootCA.csr.cnf configuration file identifies the organization that owns the server.
    v3.ext identifies the hostname and/or IP address for the server getting the certificate.

2. Run the following scripts

    createRootCA.sh creates a root certificate authority.

    arguments:
                    path to store the key
    output: 
            rootCA.key private key requires password
    rootCA.pem public key

    once you run the script you will have to enter password 3 times all of the same password.
    ```sh
    cd ssl/source/
    ./createRootCA.sh keys
    ```
    createSelfSignedCertificate.sh 

    arguments: path to where the keys are stored
    
    output:

      server.crt server public key 

      server.key private key intermediate folder will have

      server.csr certificate signing request**not needed** 

      rootCA.srl serial number log **not needed**will request to enter a password which should be the same as your certificate authority.

    ```sh
    ./createSelfSignedCertificate.sh keys
    ```
## Install certificates on the edge-server

create a directory /etc/pki/nginx
	
```sh
mkdir -r /etc/pki/nginx/private
cp ~/refinement-app-backend/ssl/keys/server.crt /etc/pki/nginx/
cp ~/refinement-app-backend/ssl/keys/server.key /etc/pki/nginx/private/
```

## Install certificates on the gateway

1. scp your rootCA.pem file to gateway.

    ```sh
    scp ~/refinement-app-backend/ssl/rootCA.pem gateway@gatewayaddress:/etc/pki/nginx/
    #if the directory does not exists on the gateway create it.
    ```

## Install the .pem file on the browser Chrome/Firefox.

    To get https on your browser you will need to copy the rootCA.pem to you computer.

    For chrome go to settings>advanced>Manage Certificates. Click on Trusted Root Certification Authorities tab and then click import. press next and then browse. find the .pem file and press ok (if you cant see the .pem file put "*.pem" in filename to see it). Press next and then finish, after pressing fininish dialog box will pop up and press ok.

    For Firefox go to preferences then privacy and security on the left. Scroll to the bottom and then click "View Certificates" and go to the Authorities tab and then click import and find your file and then press "open"

## Step 8: Nginx https set up
open up nginx config file

```sh
vim /etc/nginx/nginx.conf
```

change the file so it matches what is below

```
server {
listen       80 default_server;
listen       [::]:80 default_server;
listen       443 ssl;
ssl_certificate     /etc/pki/nginx/server.crt;        
ssl_certificate_key /etc/pki/nginx/private/server.key;
server_name  _;
root         /usr/share/nginx/html;

# Load configuration files for the default server block.
include /etc/nginx/default.d/*.conf;
location / {
}

location /rest/ {
proxy_pass      https://127.0.0.1:5000;
proxy_ssl_certificate_key  /etc/pki/nginx/private/server.key;
proxy_ssl_verify           off;
error_page 500 502 503 504 /50x.html;
location = /50x.html {
}
}
```
save and restart nginx
```sh
systemctl restart nginx
```
