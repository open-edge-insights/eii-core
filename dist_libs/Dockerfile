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

# Copying DataBusAbstraction library files.
ADD DataBusAbstraction/py/databus_requirements.txt /dist_libs/DataBusAbstraction/py/
ADD DataBusAbstraction/py/DataBus*.py /dist_libs/DataBusAbstraction/py/
ADD DataBusAbstraction/py/open62541W.so /dist_libs/DataBusAbstraction/py/
ADD DataBusAbstraction/py/test/DataBusTest.py /dist_libs/DataBusAbstraction/py/test/examples/
ADD DataBusAbstraction/py/test/examples/README.md /dist_libs/DataBusAbstraction/py/test/examples

# Copying ImageStore library files.
ADD ImageStore/client/py/client.py /dist_libs/ImageStore/client/py/
ADD ImageStore/client/py/__init__.py /dist_libs/ImageStore/client/py/
ADD ImageStore/test/py/clientTest.py /dist_libs/ImageStore/test/py/
ADD ImageStore/test/py/__init__.py /dist_libs/ImageStore/test/py/
ADD ImageStore/protobuff/py/ImageStore_pb2_grpc.py /dist_libs/ImageStore/protobuff/py/
ADD ImageStore/protobuff/py/ImageStore_pb2.py /dist_libs/ImageStore/protobuff/py/
ADD ImageStore/protobuff/py/__init__.py /dist_libs/ImageStore/protobuff/py/
ADD ImageStore/test/py/README.md /dist_libs/ImageStore/test/py/
ADD Util/crypto/encrypt_decrypt.py /dist_libs/ImageStore/test/py/Util/crypto/encrypt_decrypt.py

ENTRYPOINT ["/bin/bash", "-c", "cp -rf /dist_libs/* /eta/dist_libs/"]