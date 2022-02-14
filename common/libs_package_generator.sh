#!/bin/bash -e

# Copyright (c) 2022 Intel Corporation.
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

Utils="$CUR_DIR/util/c"
EIIMessageBus="$CUR_DIR/libs/EIIMessageBus"
ConfigMgr="$CUR_DIR/libs/ConfigMgr"
GrpcProtoPath="$CMAKE_INSTALL_PREFIX/lib"

Packages="$CUR_DIR/packages"

mkdir -p $Packages/native/debs
mkdir -p $Packages/native/apks
mkdir -p $Packages/native/rpms
mkdir -p $Packages/python/whls

DEBs="$Packages/native/debs"
APKs="$Packages/native/apks"
RPMs="$Packages/native/rpms"
WHLs="$Packages/python/whls"

log_info "----Generating Util lib .deb .rpm and .apk packages----"
cd $Utils &&
   rm -rf build && \
   mkdir build && \
   cd build && \
   cmake -DPACKAGING=ON -DPACKAGE_RPM=ON -DPACKAGE_APK=ON .. && \
   make package && \
   make package-apk && \
   cp *.deb $DEBs && \
   cp *.rpm $RPMs && \
   cp *.apk $APKs
check_error "----Failed to generate Util packages----"

log_info "----Generating EIIMessageBus .deb .rpm and .apk packages----"
cd $EIIMessageBus &&
   rm -rf build && \
   rm -rf apks && \
   mkdir build && \
   mkdir apks && \
   cp $Utils/build/*.apk apks/ && \
   cd build && \
   cmake -DPACKAGING=ON -DPACKAGE_RPM=ON -DPACKAGE_APK=ON .. && \
   make package && \
   make package-apk && \
   cp *.deb $DEBs && \
   cp *.rpm $RPMs && \
   cp *.apk $APKs
check_error "----Failed to generate EIIMessageBus packages----"

log_info "----Generating ConfigMgr .deb .rpm and .apk packages----"
cd $ConfigMgr &&
   rm -rf build && \
   rm -rf apks && \
   mkdir build && \
   mkdir apks && \
   cp $Utils/build/*.apk apks/ && \
   cp $EIIMessageBus/build/*.apk apks/
   cd build && \
   cmake -DPACKAGING=ON -DPACKAGE_RPM=ON -DPACKAGE_APK=ON .. && \
   make package && \
   make package-apk && \
   cp *.deb $DEBs && \
   cp *.rpm $RPMs && \
   cp *.apk $APKs
check_error "----Failed to generate ConfigMgr packages----"

log_info "----Generating EIIMessageBus Python Ubuntu .whl packages----"
cd $EIIMessageBus/python &&
   python3 setup_packaging.py sdist bdist_wheel --plat-name=manylinux2014_x86_64 && \
   cp dist/*.whl $WHLs
check_error "----Failed to generate EIIMessageBus python ubuntu package----"

log_info "----Generating ConfigMgr Python Ubuntu .whl package----"
cd $ConfigMgr/python &&
   python3 setup_packaging.py sdist bdist_wheel --plat-name=manylinux2014_x86_64 && \
   cp dist/*.whl $WHLs
check_error "----Failed to generate ConfigMgr python ubuntu package----"

log_info " ----Generating EIIMessageBus Python Frdora .whl packages---- "

python3.9 -m venv fedora_package
source fedora_package/bin/activate

pip3.9 install Cython==0.29.28
pip3.9 install --upgrade setuptools==44.0.0 wheel==0.37.1

log_info "----Generating EIIMessageBus Python fedora .whl packages----"
cd $EIIMessageBus/python &&
   python3.9 setup_packaging.py sdist bdist_wheel --plat-name=manylinux2014_x86_64 && \
   cp dist/*.whl $WHLs
check_error "----Failed to generate EIIMessageBus python fedora package----"

echo "----Generating ConfigMgr Python fedora .whl package----"
cd $ConfigMgr/python &&
   python3.9 setup_packaging.py sdist bdist_wheel --plat-name=manylinux2014_x86_64 && \
   cp dist/*.whl $WHLs
check_error "----Failed to generate ConfigMgr python fedora package----"

log_info " --- Packages generation is successful under the folder ./packages --- "