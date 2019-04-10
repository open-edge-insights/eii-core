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

ADD DataBusAbstraction/ DataBusAbstraction/

# Build the open62541W.so file
RUN cd /iei/DataBusAbstraction/py && \
    pip3.6 install -r databus_requirements.txt && \
    cd test && \
    make build

RUN mkdir -p /iei/dist_libs

ENV DBA_PATH /dist_libs/DataBusAbstraction
ENV IMGSTORE_PATH /dist_libs/ImageStore

# Copying DataBusAbstraction py library/test files.
ADD DataBusAbstraction/py/databus_requirements.txt ${DBA_PATH}/py/
ADD DataBusAbstraction/py/DataBus*.py ${DBA_PATH}/py/
RUN mkdir -p ${DBA_PATH}/py/Util
ADD Util/log.py ${DBA_PATH}/py/Util/
RUN cp DataBusAbstraction/py/open62541W.so ${DBA_PATH}/py/
ADD DataBusAbstraction/py/test/DataBusTest.py ${DBA_PATH}/py/test/
ADD DataBusAbstraction/py/test/README.md ${DBA_PATH}/py/test/

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

ENTRYPOINT ["/bin/bash", "-c", "cp -rf /dist_libs/* /iei/dist_libs/"]
