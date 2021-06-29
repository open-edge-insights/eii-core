#!/bin/bash -e

# Copyright (c) 2019 Intel Corporation.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

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

export CUR_DIR=$PWD

service_exists () {
    type "$1" &> /dev/null ;
}

install_go_dependencies () {
    log_info "----Installing required golang dependencies----"
    export GLOG_GO_PATH=$GOPATH/src/github.com/golang/glog
    if [ -d "$GLOG_GO_PATH" ]; then
        log_info "----$GLOG_GO_PATH already exists, skipping installation----"
    else
        export GLOG_VER=23def4e6c14b4da8ac2ed8007337bc5eb5007998
        mkdir -p $GLOG_GO_PATH && \
        git clone https://github.com/golang/glog $GLOG_GO_PATH && \
        cd $GLOG_GO_PATH && \
        git checkout -b $GLOG_VER $GLOG_VER
    fi

    export ETCD_GO_PATH=$GOPATH/src/go.etcd.io/etcd
    if [ -d "$ETCD_GO_PATH" ]; then
        log_info "$ETCD_GO_PATH already exists, skipping installation..."
    else
        export ETCD_GO_VER=0c787e26bcd102c3bb14050ac893b07ba9ea029f
        mkdir -p $ETCD_GO_PATH && \
        git clone https://github.com/etcd-io/etcd $ETCD_GO_PATH && \
        cd $ETCD_GO_PATH && \
        git checkout -b $ETCD_GO_VER $ETCD_GO_VER
    fi

}

install_go () {

    apt-get update && \
        apt-get install -y build-essential \
                        git \
                        g++ \
                        pkg-config \
                        wget  && \
        wget -q --show-progress https://dl.google.com/go/go${GO_VERSION}.linux-amd64.tar.gz && \
        tar -C /usr/local -xzf go${GO_VERSION}.linux-amd64.tar.gz
    export GOPATH=$HOME/go
    log_info "----GOPATH: $GOPATH----"
    mkdir -p $GOPATH/src

    install_go_dependencies
    export PATH=$PATH:/usr/local/go/bin
}

# Verify that the CMAKE_INSTALL_PREFIX is set
if [[ -z "${CMAKE_INSTALL_PREFIX}" ]] ; then
    log_fatal "CMAKE_INSTALL_PREFIX must be set in the environment"
fi

if service_exists go ; then
    log_info "----Golang already installed----"
    install_go_dependencies
else
    install_go
fi

# Unsetting GOROOT since it is not required
# with GOPATH set
unset GOROOT

EIIMessageBus="$CUR_DIR/libs/EIIMessageBus"
ConfigMgr="$CUR_DIR/libs/ConfigMgr"
GrpcProtoPath="$CMAKE_INSTALL_PREFIX/lib"

# Disabling error exit for this block for avoiding error out if package not available
set +e
CMAKE_EXISTS=`cmake --version`
if [ $? -ne 0 ]; then
    log_info "----Installing cmake----"
    wget -O- https://cmake.org/files/v3.15/cmake-3.15.0-Linux-x86_64.tar.gz | \
        tar --strip-components=1 -xz -C /usr/local
fi

DISTUTILS_INSTALLED=`dpkg --list|grep distutils`
if [ $? -ne 0 ]; then
    apt-get install -y python3-distutils
fi

PIP_INSTALLED=`dpkg --list|grep python3-pip`
if [ $? -ne 0 ]; then
    apt-get -y install python3-pip
fi
set -e

# Install ConfigMgr requirements
log_info "----Installing ConfigMgr dependencies----"
cd $ConfigMgr &&
rm -rf deps && \
./install.sh

log_info "----Installing EIIMessageBus lib dependencies----"
cd $EIIMessageBus &&
   rm -rf deps && \
   ./install.sh --cython

log_info "----Installing Util lib----"
cd $EIIMessageBus/../../util/c/ &&
   ./install.sh && \
   rm -rf build && \
   mkdir build && \
   cd build && \
   cmake -DCMAKE_INSTALL_INCLUDEDIR=$CMAKE_INSTALL_PREFIX/include -DCMAKE_INSTALL_PREFIX=$CMAKE_INSTALL_PREFIX -DWITH_TESTS=${RUN_TESTS} -DCMAKE_BUILD_TYPE=$CMAKE_BUILD_TYPE .. && \
   make && \
   if [ "${RUN_TESTS}" = "ON" ] ; then cd ./tests  && \
   ./config-tests
   ./log-tests
   ./thp-tests
   ./tsp-tests
   ./thexec-tests
   cd .. ; fi  && \
   make install
check_error "----Failed to install Util lib----"

log_info "----Installing EIIMessageBus library----"
cd $EIIMessageBus && \
    rm -rf build/ && \
    mkdir build/ && \
    cd build/ && \
    cmake -DCMAKE_INSTALL_INCLUDEDIR=$CMAKE_INSTALL_PREFIX/include -DCMAKE_INSTALL_PREFIX=$CMAKE_INSTALL_PREFIX -DWITH_TESTS=${RUN_TESTS} -DWITH_TESTS=${RUN_TESTS} -DCMAKE_BUILD_TYPE=$CMAKE_BUILD_TYPE .. && \
    make install
check_error "----Failed to install EIIMessageBus library----"

log_info "----Installing EIIMessageBus python binding----"
cd $EIIMessageBus/python &&
   python3 setup.py install --user
check_error "----Failed to install EIIMessageBus python binding----"

log_info "----Installing EIIMessageBus Golang binding----"
cd $EIIMessageBus &&
   cp -a go/. $GOPATH/src/
check_error "----Failed to install EIIMessageBus Golang binding----"

log_info "----Installing ConfigMgr C and golang libs----"
cd $ConfigMgr && \
   # Installing grpc from DEB package
   apt install ./grpc-1.29.0-Linux.deb && \
   rm -rf build && \
   mkdir build && \
   cd build && \
   cmake -DCMAKE_INSTALL_INCLUDEDIR=$CMAKE_INSTALL_PREFIX/include -DCMAKE_INSTALL_PREFIX=$CMAKE_INSTALL_PREFIX -DWITH_TESTS=${RUN_TESTS} -DCMAKE_BUILD_TYPE=$CMAKE_BUILD_TYPE -DWITH_GO=ON .. && \
   make install
check_error "----Failed to install ConfigMgr C and golang libs----"

log_info "----Installing ConfigMgr python bindings----"
cd $ConfigMgr/python &&
   python3 setup.py.in install --user
check_error "----Failed to install ConfigMgr python binding----"
