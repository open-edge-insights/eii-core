# Dockerfile for Point Data Analytics
ARG IEI_VERSION
FROM ia_gopybase:$IEI_VERSION
LABEL description="Data Analytics image"

ENV PY_WORK_DIR /IEI
WORKDIR ${PY_WORK_DIR}
ENV PYTHONPATH .:./DataAgent/da_grpc/protobuff
ENV HOME /IEI
ENV KAPACITOR_REPO /IEI/go/src/github.com/influxdata/kapacitor
ENV GO_ROOT_BIN /IEI/go/bin

# Installing protobuf module needed for UDF function
RUN pip2 install protobuf

# Installing dependent python modules
ADD DataAnalytics/PointDataAnalytics/classifier_requirements.txt .
RUN pip3.6 install -r classifier_requirements.txt && \
    rm -rf classifier_requirements.txt

# Adding kapacitor related files
ADD kapacitor ${KAPACITOR_REPO}
RUN cd ${KAPACITOR_REPO} && \
    python2.7 build.py --clean -o ${GO_ROOT_BIN} && \
    mkdir -p ${DIR}/kapacitor && \
    cp -r udf ${DIR}/kapacitor

#Creating dir for kapacitor certs file
RUN mkdir -p /etc/ssl/kapacitor && \
    chown ${IEI_UID} /IEI && \
    chown -R ${IEI_UID} /etc/ssl && \
    mkdir -p /etc/kapacitor && \
    chown ${IEI_UID} /etc/kapacitor

ENV GLOG_GO_PATH ${GOPATH}/src/github.com/golang/glog
RUN mkdir -p ${GLOG_GO_PATH} && \
    git clone https://github.com/golang/glog ${GLOG_GO_PATH} && \
    cd ${GLOG_GO_PATH} && \
    git checkout -b known_version 23def4e6c14b4da8ac2ed8007337bc5eb5007998

# Adding classifier program
ADD DataAnalytics/PointDataAnalytics .
RUN chmod +x ./classifier_start.sh

# Adding project depedency modules
ADD DataAgent/__init__.py ./DataAgent/__init__.py
ADD DataAgent/da_grpc ./DataAgent/da_grpc
ADD Util ./Util

RUN mkdir -p /etc/ssl/ca
ENV PYTHONPATH ${PYTHONPATH}:./DataAgent/da_grpc/protobuff/py:./DataAgent/da_grpc/protobuff/py/pb_internal
ARG IEI_UID

RUN go build -o point_classifier point_classifier.go 
ENTRYPOINT ["./classifier_start.sh"]
HEALTHCHECK NONE
