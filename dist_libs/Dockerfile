FROM ubuntu:16.04

USER root
WORKDIR /eta

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
RUN python3.6 get-pip.py

# tls, crypto library needs to open62541
RUN apt-get install -y libmbedtls-dev

ADD DataBusAbstraction/ DataBusAbstraction/

# Build the open62541W.so file
RUN cd /eta/DataBusAbstraction/py && \
    pip3.6 install -r databus_requirements.txt && \
    cd test && \
    make build

RUN mkdir -p /eta/dist_libs

# Copying DataBusAbstraction py library/test files.
ADD DataBusAbstraction/py/databus_requirements.txt /dist_libs/DataBusAbstraction/py/
ADD DataBusAbstraction/py/DataBus*.py /dist_libs/DataBusAbstraction/py/
RUN mkdir -p /dist_libs/DataBusAbstraction/py/Util
ADD Util/log.py /dist_libs/DataBusAbstraction/py/Util/
RUN cp DataBusAbstraction/py/open62541W.so /dist_libs/DataBusAbstraction/py/
ADD DataBusAbstraction/py/test/DataBusTest.py /dist_libs/DataBusAbstraction/py/test/
ADD DataBusAbstraction/py/test/README.md /dist_libs/DataBusAbstraction/py/test/

# Copying ImageStore py library/test files.
ADD ImageStore/__init__.py /dist_libs/ImageStore/
ADD ImageStore/client/py/client.py /dist_libs/ImageStore/client/py/
ADD ImageStore/client/py/__init__.py /dist_libs/ImageStore/client/py/
ADD ImageStore/test/py/clientTest.py /dist_libs/ImageStore/test/py/
ADD ImageStore/protobuff/py/ImageStore_pb2_grpc.py /dist_libs/ImageStore/protobuff/py/
ADD ImageStore/protobuff/py/ImageStore_pb2.py /dist_libs/ImageStore/protobuff/py/
ADD ImageStore/protobuff/py/__init__.py /dist_libs/ImageStore/protobuff/py/
ADD ImageStore/test/py/README.md /dist_libs/ImageStore/test/py/
ADD Util/crypto/encrypt_decrypt.py /dist_libs/ImageStore/test/py/Util/crypto/encrypt_decrypt.py

# Copying ImageStore py library/test files.
ADD ImageStore/client/cpp/ImageStoreClient.cc /dist_libs/ImageStore/client/cpp/ImageStoreClient.cc
ADD ImageStore/test/cpp/README.md /dist_libs/ImageStore/test/cpp/README.md
ADD ImageStore/test/cpp/clientTest.cc /dist_libs/ImageStore/test/cpp/clientTest.cc
ADD ImageStore/test/cpp/Makefile /dist_libs/ImageStore/test/cpp/Makefile
ADD ImageStore/protobuff/cpp/*.cc /dist_libs/ImageStore/protobuff/cpp/
ADD ImageStore/protobuff/cpp/*.h /dist_libs/ImageStore/protobuff/cpp/
ADD ImageStore/protobuff/*.proto /dist_libs/ImageStore/protobuff/

ENTRYPOINT ["/bin/bash", "-c", "cp -rf /dist_libs/* /eta/dist_libs/"]
