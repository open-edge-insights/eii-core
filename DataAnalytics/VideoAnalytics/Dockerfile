# Dockerfile for VideoAnalytics
ARG IEI_VERSION
FROM ia_pybase:$IEI_VERSION
LABEL description="VideoAnalytics image"

ARG IEI_UID
RUN useradd -r -u ${IEI_UID} -G video ieiuser

ENV DIR /IEI
WORKDIR ${DIR}
ENV PYTHONPATH .:./DataAgent/da_grpc/protobuff
ENV HOME /IEI

RUN apt-get update

# Installing dependent python modules
ADD DataAnalytics/VideoAnalytics/va_requirements.txt .
RUN pip3.6 install -r va_requirements.txt && \
    rm -rf va_requirements.txt

# OpenVINO install
RUN apt-get install -y libpng12-dev libcairo2-dev libpango1.0-dev libglib2.0-dev libgtk2.0-dev \
    libswscale-dev libavcodec-dev libavformat-dev libgstreamer1.0-0 gstreamer1.0-plugins-base \
    build-essential cmake libusb-1.0-0-dev libva-dev libdrm-dev cpio
RUN apt-get install -y lsb-release
ADD DataAnalytics/VideoAnalytics/l_openvino_toolkit*  ${HOME}/openvino_toolkit/
RUN cd ${HOME}/openvino_toolkit && \
    sed -i -e 's/^ACCEPT_EULA=decline/ACCEPT_EULA=accept/g' silent.cfg && \
    ./install.sh -s silent.cfg --ignore-cpu

# Configure MO and install dependencies for processor graphics
RUN bash -c 'source /opt/intel/computer_vision_sdk/bin/setupvars.sh && \
    cd /opt/intel/computer_vision_sdk/deployment_tools/model_optimizer/install_prerequisites && \
    sed -i -e "s/sudo \(-[^- ]*\)\?//g" install_prerequisites.sh && \
    sed -i '/onnx/d' ../requirements.txt && echo "onnx==1.1.2" >> ../requirements.txt && \
    sed -i '/networkx/d' ../requirements.txt && echo "networkx==1.11" >> ../requirements.txt && \
    sed -i '/numpy/d' ../requirements.txt && echo "numpy==1.12.0" >> ../requirements.txt && \
    ./install_prerequisites.sh && \
    cd /opt/intel/computer_vision_sdk/install_dependencies && \
    ./install_NEO_OCL_driver.sh'

# Remove OpenVINO toolkit after installation
RUN rm -rf ${HOME}/l_openvino_toolkit*

#Creating dir for certs file
RUN mkdir -p /etc/ssl/imagestore
RUN mkdir -p /etc/ssl/streamsublib
RUN mkdir -p /etc/ssl/grpc_int_ssl_secrets

# Adding classifier program
ADD DataAnalytics/VideoAnalytics/ .

# Adding project depedency modules
ADD algos ./algos
ADD DataAgent/__init__.py ./DataAgent/__init__.py
ADD DataAgent/da_grpc ./DataAgent/da_grpc
ADD ImageStore ./ImageStore
ADD StreamSubLib ./StreamSubLib
ADD DataIngestionLib ./DataIngestionLib
ADD Util ./Util

RUN mkdir -p /etc/ssl/ca
ENV PYTHONPATH ${PYTHONPATH}:./DataAgent/da_grpc/protobuff/py:./DataAgent/da_grpc/protobuff/py/pb_internal:./ImageStore/protobuff/py/:./algos/dpm/classification/

RUN chown ${IEI_UID} /IEI
RUN chown -R ${IEI_UID} /etc/ssl
ENTRYPOINT ["./va_classifier_start.sh"]
HEALTHCHECK NONE
