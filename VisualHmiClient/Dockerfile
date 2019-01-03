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

ADD VisualHmiClient/requirements.txt ./VisualHmiClient/requirements.txt
RUN pip install -r VisualHmiClient/requirements.txt

# tls, crypto library needs to open62541
RUN apt-get install -y libmbedtls-dev

ADD DataBusAbstraction/ DataBusAbstraction/

# Build the opent62541W.so file
RUN cd /eta/DataBusAbstraction/py && \
    pip3.6 install -r databus_requirements.txt && \
    cd test && \
    make build
ADD algos/__init__.py algos/__init__.py
ADD algos/etr_utils algos/etr_utils
ADD VisualHmiClient/ VisualHmiClient/
ADD ImageStore/ ImageStore/
ADD Util/ Util/

ENV PYTHONPATH .:./ImageStore/protobuff/py/:./DataBusAbstraction/py
ENV PYTHONIOENCODING UTF-8

ENTRYPOINT [ "python3.6", "VisualHmiClient/VisualHmiEtaDataSync.py"]
CMD ["-c", "VisualHmiClient/config.json"]
