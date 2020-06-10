#!/bin/bash -e

# Copyright (c) 2020 Intel Corporation.

# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

# Provision EIS
# Usage: sudo ./provision_eis <path-of-docker-compose-file>

docker_compose=$1
RED='\033[0;31m'
YELLOW="\033[1;33m"
GREEN="\033[0;32m"
NC='\033[0m' # No Color

function log_warn() {
    echo -e "${YELLOW}WARN: $1 ${NC}"
}

function log_info() {
    echo -e "${GREEN}INFO: $1 ${NC}"
}

function log_error() {
    echo -e "${RED}ERROR: $1 ${NC}"
}

function log_fatal() {
    echo -e "${RED}FATAL: $1 ${NC}"
    exit -1
}

function check_error() {
    if [ $? -ne 0 ] ; then
        log_fatal "$1"
    fi
}

function souce_env() {
    EIS_ENV="../.env"
    EIS_PROVISIONING_ENV=".env"

    set -a
    if [ -f $EIS_ENV ]; then
        source $EIS_ENV
    fi

    if [ -f $EIS_PROVISIONING_ENV ]; then
        source $EIS_PROVISIONING_ENV
    fi
    set +a
}


function export_host_ip() {
    if [ -z $HOST_IP ]; then
        hostIP=`hostname -I | awk '{print $1}'`
        export HOST_IP=$hostIP
    fi
    echo 'System IP Address is:' $HOST_IP
    export no_proxy=$eis_no_proxy,$HOST_IP
}

function set_docker_host_time_zone() {
    echo "Updating .env for container timezone..."
    # Get Docker Host timezone
    hostTimezone=`timedatectl status | grep "zone" | sed -e 's/^[ ]*Time zone: \(.*\) (.*)$/\1/g'`
    hostTimezone=`echo $hostTimezone`
    # This will remove the HOST_TIME_ZONE entry if it exists and adds a new one with the right timezone
    sed -i '/HOST_TIME_ZONE/d' ../.env && echo "HOST_TIME_ZONE=$hostTimezone" >> ../.env
}

function create_eis_user() {
    echo "Create $EIS_USER_NAME if it doesn't exists. Update UID from env if already exits with different UID"

    # EIS containers will be executed as eisuser
    if ! id $EIS_USER_NAME >/dev/null 2>&1; then
        groupadd $EIS_USER_NAME -g $EIS_UID
        useradd -r -u $EIS_UID -g $EIS_USER_NAME $EIS_USER_NAME
    else
        if ! [ $(id -u $EIS_USER_NAME) = $EIS_UID ]; then
            usermod -u $EIS_UID $EIS_USER_NAME
            groupmod -g $EIS_UID $EIS_USER_NAME
        fi
    fi
}

function create_eis_install_dir() {
    log_info "Creating EIS install Directories"
    if [ $ETCD_RESET = 'true' ]; then
        rm -rf $EIS_INSTALL_PATH/data/etcd
    fi

    mkdir -p $EIS_INSTALL_PATH/data/influxdata
    check_error "Failed to create dir '$EIS_INSTALL_PATH/data/influxdata'"
    mkdir -p $EIS_INSTALL_PATH/data/etcd/data
    mkdir -p $EIS_INSTALL_PATH/sockets/
    chown -R $EIS_USER_NAME:$EIS_USER_NAME $EIS_INSTALL_PATH
    chmod -R 755 $EIS_INSTALL_PATH/data/

    if [ -d $TC_DISPATCHER_PATH ]; then
        chown -R $EIS_USER_NAME:$EIS_USER_NAME $TC_DISPATCHER_PATH
        chmod -R 755 $TC_DISPATCHER_PATH
    fi
}

function copy_docker_compose_file() {
    echo "Copying docker compose yaml file which is provided as an argument."
    # This file will be volume mounted inside the provisioning container and deleted once privisioning it done
    if ! [ -f $docker_compose ] || [ -z $docker_compose ]; then
        log_error "Supplied docker compose file '$docker_compose' does not exists"
        log_fatal "Usage: $ sudo ./provision_eis.sh <path_to_eis_docker_compose_file>"
    else
        cp $docker_compose ./docker-compose.yml
    fi
}

function remove_docker_compose_file() {
    if [ -f "docker-compose.yml" ]; then
        echo "Removing copied docker compose yaml file ..."
        rm -rf ./docker-compose.yml
    fi
}

function remove_client_server() {
    echo "Deleting client and server cert files ..."
    if [ -d "client" ]; then
        rm -rf client
    fi
    if [ -d "server" ]; then
        rm -rf server
    fi
}

function prod_mode_gen_certs() {
   log_info "Generating EIS Certificates"
   if [ -d "rootca" ]; then
       log_warn "Making use of existing CA from ./rootca dir for generating certs..."
       log_warn "To generate new CA, remove roootca/ from current dir.."
       python3 gen_certs.py --f $docker_compose --capath rootca/
   else
       python3 gen_certs.py --f $docker_compose
   fi
   chown -R $EIS_USER_NAME:$EIS_USER_NAME Certificates/
}

function gen_csl_certs() {
    log_info "Generating CSL Certificates"
    /bin/bash ./dep/generate_csl_certificates.sh
    chown -R $EIS_USER_NAME:$EIS_USER_NAME Certificates/
}

function check_ETCD_port() {
    log_info "Checking if ETCD ports are already up..."
    ports=($ETCD_CLIENT_PORT)
    for port in "${ports[@]}"
    do
       set +e
       fuser $port/tcp
       if [ $? -eq 0 ]; then
           log_fatal "$port is already being used, so please kill that process and re-run the script."
       fi
       set -e
    done
}

souce_env
export_host_ip
set_docker_host_time_zone
create_eis_user
create_eis_install_dir

if [ $DEV_MODE = 'false' ]; then
    if [ -d 'Certificates' ]; then
        chown -R $EIS_USER_NAME:$EIS_USER_NAME Certificates/
    fi
fi

#############################################################

if [ $PROVISION_MODE = 'csl' -a $ETCD_NAME = 'master' ]; then
    if [ $DEV_MODE = 'false' ]; then
	log_info "Provisioning EIS with mode... "
        copy_docker_compose_file
        prod_mode_gen_certs
        gen_csl_certs
        docker-compose -f dep/docker-compose-cslprovision.yml up --build -d
    else
	log_fatal "Orchestration with CSL is not supported in Dev mode"
    fi
elif [ $ETCD_NAME = 'master' ]; then
    log_info "Bringing down existing ETCD container"
    python3 stop_and_remove_existing_eis.py --f dep/docker-compose-provision.yml

    copy_docker_compose_file

    log_info "Installing dependencies.."
    pip3 install -r cert_requirements.txt
    echo "Clearing existing Certificates..."
    rm -rf Certificates

    echo "Checking ETCD port..."
    check_ETCD_port

    if [ $DEV_MODE = 'true' ]; then
        log_info "EIS is not running in Secure mode. Generating certificates is not required.. "
        log_info "Starting and provisioning ETCD ..."
        docker-compose -f dep/docker-compose-provision.yml up --build -d
    else
        prod_mode_gen_certs
        log_info "Starting and provisioning ETCD ..."
        docker-compose -f dep/docker-compose-provision.yml -f dep/docker-compose-provision.override.prod.yml up --build -d
    fi

    echo "Bringing down existing EIS containers"
    python3 stop_and_remove_existing_eis.py --f $docker_compose
fi

remove_client_server
remove_docker_compose_file

log_info "EIS Provisioning is Successful..."
