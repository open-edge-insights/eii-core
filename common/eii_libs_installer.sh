#!/bin/bash -e

export CUR_DIR=$PWD

service_exists () {
    type "$1" &> /dev/null ;
}

install_go_dependencies () {

    export GLOG_GO_PATH=$GOPATH/src/github.com/golang/glog
    if [ -d "$GLOG_GO_PATH" ]; then
        echo "$GLOG_GO_PATH already exists, skipping installation..."
    else
        export GLOG_VER=23def4e6c14b4da8ac2ed8007337bc5eb5007998
        mkdir -p $GLOG_GO_PATH && \
        git clone https://github.com/golang/glog $GLOG_GO_PATH && \
        cd $GLOG_GO_PATH && \
        git checkout -b $GLOG_VER $GLOG_VER
    fi

    export ETCD_GO_PATH=$GOPATH/src/go.etcd.io/etcd
    if [ -d "$ETCD_GO_PATH" ]; then
        echo "$ETCD_GO_PATH already exists, skipping installation..."
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
    apt-get install -y wget git build-essential pkg-config iputils-ping g++ && \
    wget https://dl.google.com/go/go1.12.linux-amd64.tar.gz && \
    tar -C /usr/local -xzf go1.12.linux-amd64.tar.gz
    export GOPATH=/home/$USER/go/
    mkdir -p $GOPATH/src

    install_go_dependencies
    export PATH=$PATH:/usr/local/go/bin
}

if service_exists go ; then
    echo "Go already installed"
    install_go_dependencies
else
    install_go
fi

# Unsetting GOROOT since it is not required
# with GOPATH set
unset GOROOT

DynLibLoad="$CUR_DIR/libs/DynLibLoad"
EIIMessageBus="$CUR_DIR/libs/EIIMessageBus"
ConfigMgr="$CUR_DIR/libs/ConfigMgr"
GrpcProtoPath="/usr/local/lib"

# Installing cmake 3.15
wget -O- https://cmake.org/files/v3.15/cmake-3.15.0-Linux-x86_64.tar.gz | \
    tar --strip-components=1 -xz -C /usr/local

apt-get install -y python3-distutils

# Install ConfigMgr requirements
if [[ ! -f "$GrpcProtoPath/libgrpc.a" ]] || [[ ! -f "$GrpcProtoPath/libprotobuf.a" ]]; then
    echo "Installing ConfigMgr dependencies"
    cd $ConfigMgr &&
    rm -rf deps && \
    ./install.sh
else
    echo "Skipping installation of grpc & protobuf"
fi

# Install EIIMessageBus requirements
cd $EIIMessageBus &&
   rm -rf deps && \
   ./install.sh --cython

cd $EIIMessageBus/../IntelSafeString/ &&
   rm -rf build && \
   mkdir build && \
   cd build && \
   cmake -DCMAKE_BUILD_TYPE=$CMAKE_BUILD_TYPE .. && \
   make install

cd $EIIMessageBus/../EIIMsgEnv/ &&
   rm -rf build && \
   mkdir build && \
   cd build && \
   cmake  -DWITH_TESTS=${RUN_TESTS} -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} .. && \
   make && \
   if [ "${RUN_TESTS}" = "ON" ] ; then cd ./tests && \
   ./msg-envelope-tests  && \
   ./crc32-tests  && \
   cd .. ; fi && \
   make install

cd $EIIMessageBus/../../util/c/ &&
   ./install.sh && \
   rm -rf build && \
   mkdir build && \
   cd build && \
   cmake -DWITH_TESTS=${RUN_TESTS} -DCMAKE_BUILD_TYPE=$CMAKE_BUILD_TYPE .. && \
   make && \
   if [ "${RUN_TESTS}" = "ON" ] ; then cd ./tests  && \
   ./config-tests
   ./log-tests
   ./thp-tests
   ./tsp-tests
   ./thexec-tests
   cd .. ; fi  && \
   make install

cd $EIIMessageBus/../DynLibLoad/ && \
    rm -rf build &&  \
    mkdir build && \
    cd build && \
    cmake -DWITH_TESTS=${RUN_TESTS} -DCMAKE_BUILD_TYPE=$CMAKE_BUILD_TYPE .. && \
    make && \
    if [ "${RUN_TESTS}" = "ON" ] ; then cd ./tests && ./dynlibload-tests && cd .. ; fi && \
    make install

# Installing EIIMessageBus C++ from DEB package
cd $EIIMessageBus &&
   apt install ./eii-messagebus-2.4.0-Linux.deb

# Installing EIIMessageBus python
cd $EIIMessageBus/python &&
   python3 setup.py install

# Installing EIIMessageBus Go
cd $EIIMessageBus &&
   cp -a go/. $GOPATH/src/

# Install ConfigMgr and kv_store_plugin
cd $ConfigMgr && \
   rm -rf build && \
   mkdir build && \
   cd build && \
   cmake -DWITH_GO=ON -DWITH_PYTHON=ON .. && \
   make install
