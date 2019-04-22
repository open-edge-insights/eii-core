# Dockerfile for Point Data Analytics

ARG IEI_VERSION
FROM ia_pybase:$IEI_VERSION
LABEL description="Classifier image"

ENV PACKAGES="\
  python2 \ 
  go \
  bash \
  "

ENV DIR /IEI
WORKDIR ${DIR}
ENV HOME /IEI
ENV KAPACITOR_REPO /IEI/go/src/github.com/influxdata/kapacitor
ENV GO_ROOT_BIN /IEI/go/bin
ENV PYTHONPATH .:./DataAgent/da_grpc/protobuff:./kapacitor/udf/agent/py:./DataAgent/da_grpc/protobuff/py:./DataAgent/da_grpc/protobuff/py/pb_internal
ENV PATH ${PATH}:/usr/local/go/bin:${GO_ROOT_BIN}

RUN apk add --no-cache $PACKAGES

# Adding kapacitor related files
ADD kapacitor ${KAPACITOR_REPO}
RUN apk add  --no-cache --virtual .build-deps \
    git \
    musl-dev \
    && cd ${KAPACITOR_REPO} \
    && python2.7 build.py --clean -o ${GO_ROOT_BIN} \
    && mkdir -p ${DIR}/kapacitor \
    && cp -r udf ${DIR}/kapacitor 

#Creating dir for kapacitor certs file
RUN mkdir -p /etc/ssl/ca \
    && mkdir -p /etc/ssl/kapacitor

ARG IEI_UID

RUN chown ${IEI_UID} /IEI \
    && chown -R ${IEI_UID} /etc/ssl \
    && mkdir -p /etc/kapacitor \
    && chown ${IEI_UID} /etc/kapacitor

ENV GOPATH="/IEI/go"
ENV GLOG_GO_PATH ${GOPATH}/src/github.com/golang/glog
RUN mkdir -p ${GLOG_GO_PATH} && \
    git clone https://github.com/golang/glog ${GLOG_GO_PATH} && \
    cd ${GLOG_GO_PATH} && \
    git checkout -b known_version 23def4e6c14b4da8ac2ed8007337bc5eb5007998 

# Installing dependent python modules
ADD DataAnalytics/PointDataAnalytics/classifier_requirements.txt .
RUN pip3.6 install -r classifier_requirements.txt \
    && rm -rf classifier_requirements.txt

# Adding classifier program
ADD DataAnalytics/PointDataAnalytics/ . 
   
RUN go build -o point_classifier point_classifier.go \ 
    && chmod 755 ./classifier_start.sh \
    && apk del .build-deps \
    && rm -rf /var/cache/apk/*

# Adding project depedency modules
ADD DataAgent/__init__.py ./DataAgent/__init__.py
ADD DataAgent/da_grpc ./DataAgent/da_grpc
ADD Util ./Util

ENTRYPOINT ["./classifier_start.sh"]
HEALTHCHECK NONE
