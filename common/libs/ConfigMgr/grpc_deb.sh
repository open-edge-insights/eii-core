#!/bin/bash

# Copyright (c) 2021 Intel Corporation.
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

# Installing grpc dependency
# Library versions
grpc_version="v1.29.0"

config_mgr="../../.."
grpc_install_prefix="usr/local"
grpc_deb_file_dir="grpc-1.29.0-Linux"

# URLs
grpc_url="https://github.com/grpc/grpc"


# Dirs
grpc_dir="grpc"

if [ ! -d "deps" ] ; then
    mkdir deps
    check_error "Failed to create dependencies directory"
fi

cd deps
check_error "Failed to change to deps directory"

if [ ! -d "grpc_libs" ] ; then
    mkdir grpc_libs
    check_error "Failed to create grpc_libs directory"
fi

if [ ! -d "grpc" ] ; then
    log_info "git clone of grpc"
    git clone --recurse-submodules -b $grpc_version $grpc_url
    check_error "Failed to git clone"
    # Updating the version of libcares to 1.17.1
    # to fix the vulnerability CVE-2020-8277
    cd grpc/third_party/cares/cares
    git checkout cares-1_17_1
    cd ../../../../
fi

cd grpc
check_error "Failed to cd $grpc_dir"

if [ ! -d "cmake/build" ] ; then
    mkdir -p cmake/build
    check_error "Failed to create cmake/build directory"
fi

cd cmake/build
check_error "Falied to change directory to cmake/build"

log_info "Configuring lib grpc for building"
cmake -DCMAKE_INSTALL_PREFIX=$config_mgr/grpc_libs ../..

check_error "Failed to configure lib grpc"

log_info "Compiling grpc library"
make
check_error "Failed to compile grpc library"

log_info "Installing grpc library"
make install
check_error "Failed to install grpc library"

cd $config_mgr

## Creation of deb file

# create a folder from which deb file gets generated
if [ ! -d $grpc_deb_file_dir ] ; then
    mkdir $grpc_deb_file_dir
    check_error "Failed to create $grpc_deb_file_dir directory"
fi

cd $grpc_deb_file_dir
rm -rf *
check_error "Failed to remove files from $grpc_deb_file_dir"

# DEBIAN folder needs to be created which consists of control file
mkdir DEBIAN
check_error "Failed to create DEBIAN directory"
cd DEBIAN

#control file copied from ConfigMgr
cp $config_mgr/control ./
check_error "Failed to copy control to DEBIAN"

# cd to $grpc_deb_file_dir. i.e., to the grpc-1.29.0-Linux folder
cd ../

# # create usr/local/bin, usr/local/lib and usr/local/include directories
mkdir -p $grpc_install_prefix
check_error "Failed to create grpc_install_prefix directory"

grpc_libs="../grpc_libs"

# copy grpc's bin, lib and include to usr/local/bin, usr/local/lib 
# and usr/local/include of deb file respectively
cp -R $grpc_libs/bin $grpc_libs/lib $grpc_libs/include $grpc_install_prefix/.
check_error "Failed to copy bin, lib and include to usr/local in the deb file"

# cd to ConfigMgr to build the .deb file from the deb folder
cd ../

apt-get install build-essential
check_error "build-essential failed"

log_info "Building a deb package..."
# build the ./deb package from deb folder
dpkg-deb --build $grpc_deb_file_dir
check_error "Creation of deb package failed"

log_info "deb package created in the name of \"$grpc_deb_file_dir.deb\""

mv $grpc_deb_file_dir.deb ../.
log_info "deb package successfully moved to ConfigMgr directory"

log_info "Cleaning up in process ..."
rm -rf grpc/ grpc_libs/ $grpc_deb_file_dir/
log_info "Creation of \"$grpc_deb_file_dir.deb\" in ConfigMgr directory is complete"
