ARG UBUNTU_IMAGE_VERSION
FROM ubuntu:${UBUNTU_IMAGE_VERSION}

USER root
WORKDIR /iei

ENV no_proxy localhost

RUN apt-get update
RUN apt-get install -y software-properties-common
RUN apt-get install -y pkg-config python python-pip
RUN add-apt-repository ppa:deadsnakes/ppa
RUN apt-get update
RUN apt-get install -y python3.6 python3.6-dev wget
RUN apt-get install -y git

ENV TERM xterm
RUN wget https://bootstrap.pypa.io/get-pip.py

# Needed for only ubuntu 18.04
RUN apt-get install -y python3-distutils

RUN python3.6 get-pip.py

# tls, crypto library needs to open62541
RUN apt-get install -y libmbedtls-dev

# Building safestringlib
ENV SAFESTRING_VER 77b772849eda2321fb0dca56a321e3939930d7b9
ENV MAX_SAFESTRING_SIZE 60
RUN git clone https://github.com/intel/safestringlib.git && \
    cd safestringlib && \
    git checkout ${SAFESTRING_VER} && \
    cd include && \
    sed -i "/RSIZE_MAX_STR/c\#define RSIZE_MAX_STR      ( ${MAX_SAFESTRING_SIZE}UL << 10 )      /* ${MAX_SAFESTRING_SIZE}KB */" "safe_str_lib.h" && \
    cd .. && \
    make && \
    cp -rf libsafestring.a /iei

# Build the open62541W.so file
ADD DataBusAbstraction/py/databus_requirements.txt DataBusAbstraction/py/databus_requirements.txt
RUN cd /iei/DataBusAbstraction/py/ && \
    pip3.6 install -r databus_requirements.txt

ADD DataBusAbstraction/c DataBusAbstraction/c/
# Build c open62541 static libraries
RUN cd /iei/DataBusAbstraction/c/test && \
    make clean && \
    make build_static_lib

ADD DataBusAbstraction/py DataBusAbstraction/py/
RUN cd /iei/DataBusAbstraction/py/test && \
    make clean && \
    cp -rf /iei/libsafestring.a ../open62541 && \
    make build && \
    rm -rf ../libsafestring.a

RUN mkdir -p /iei/dist_libs

ENV DBA_PATH /dist_libs/DataBusAbstraction
ENV IMGSTORE_PATH /dist_libs/ImageStore

ADD Util/log.py ${DBA_PATH}/py/Util/
# Copying DataBusAbstraction py library/test files.
RUN cp DataBusAbstraction/py/databus_requirements.txt ${DBA_PATH}/py/ && \
    cp DataBusAbstraction/py/DataBus*.py ${DBA_PATH}/py/ && \
    cp DataBusAbstraction/py/open62541W.so ${DBA_PATH}/py/ && \
    mkdir -p ${DBA_PATH}/py/test && \
    cp DataBusAbstraction/py/test/DataBusTest.py ${DBA_PATH}/py/test/ && \
    cp DataBusAbstraction/py/test/Makefile ${DBA_PATH}/py/test/ && \
    cp DataBusAbstraction/py/test/README.md ${DBA_PATH}/py/test/ && \
    rm -rf DataBusAbstraction/py

# Copying DataBusAbstraction c library/test files.
RUN mkdir -p ${DBA_PATH}/c/
RUN mkdir -p ${DBA_PATH}/c/test/
ADD DataBusAbstraction/c/test/ ${DBA_PATH}/c/test/
RUN cp DataBusAbstraction/c/DataBus.c ${DBA_PATH}/c/ && \
    cp DataBusAbstraction/c/DataBus.h ${DBA_PATH}/c/ && \
    cp DataBusAbstraction/c/open62541/include/common.h ${DBA_PATH}/c/ && \
    cp DataBusAbstraction/c/open62541/include/open62541.h ${DBA_PATH}/c/ && \
    cp DataBusAbstraction/c/open62541/include/open62541_wrappers.h ${DBA_PATH}/c/ && \
    cp DataBusAbstraction/c/test/libopen62541_wrappers.a ${DBA_PATH}/c/test/ && \
    cp /iei/libsafestring.a ${DBA_PATH}/c/test/ && \
    rm -rf DataBusAbstraction/c

# Copying ImageStore py library/test files.
ADD ImageStore/__init__.py ${IMGSTORE_PATH}/
ADD ImageStore/client/py/client.py ${IMGSTORE_PATH}/client/py/
ADD ImageStore/client/py/__init__.py ${IMGSTORE_PATH}/client/py/
ADD ImageStore/py/imagestore_requirements.txt ${IMGSTORE_PATH}/test/py/
ADD ImageStore/test/py/clientTest.py ${IMGSTORE_PATH}/test/py/
ADD ImageStore/protobuff/py/ImageStore_pb2_grpc.py ${IMGSTORE_PATH}/protobuff/py/
ADD ImageStore/protobuff/py/ImageStore_pb2.py ${IMGSTORE_PATH}/protobuff/py/
ADD ImageStore/protobuff/py/__init__.py ${IMGSTORE_PATH}/protobuff/py/
ADD ImageStore/test/py/README.md ${IMGSTORE_PATH}/test/py/
ADD Util/crypto/encrypt_decrypt.py ${IMGSTORE_PATH}/test/py/Util/crypto/encrypt_decrypt.py

# Copying ImageStore cpp library/test files.
ADD ImageStore/client/cpp/ImageStoreClient.cc ${IMGSTORE_PATH}/client/cpp/ImageStoreClient.cc
ADD ImageStore/test/cpp/README.md ${IMGSTORE_PATH}/test/cpp/README.md
ADD ImageStore/test/cpp/clientTest.cc ${IMGSTORE_PATH}/test/cpp/clientTest.cc
ADD ImageStore/test/cpp/Makefile ${IMGSTORE_PATH}/test/cpp/Makefile
ADD ImageStore/test/cpp/setup_ubuntu_dev_env_cpp.sh ${IMGSTORE_PATH}/test/cpp/setup_ubuntu_dev_env_cpp.sh
ADD ImageStore/protobuff/cpp/*.cc ${IMGSTORE_PATH}/protobuff/cpp/
ADD ImageStore/protobuff/cpp/*.h ${IMGSTORE_PATH}/protobuff/cpp/
ADD ImageStore/protobuff/*.proto ${IMGSTORE_PATH}/protobuff/

# Generating documentation for c,cpp, py modules
ADD docker_setup/generate_doc_doxygen.sh /dist_libs/generate_doc_doxygen.sh
RUN cd /dist_libs && \
    chmod +x generate_doc_doxygen.sh && \
    ./generate_doc_doxygen.sh && \
    rm -rf Doxyfile generate_doc_doxygen.sh

ENTRYPOINT ["/bin/bash", "-c", "cp -rf /dist_libs/* /iei/dist_libs/ && rm -rf /dist_libs/"]
