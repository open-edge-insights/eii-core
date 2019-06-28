# Dockerfile for Point Data Analytics
ARG IEI_VERSION
FROM ia_gobase:$IEI_VERSION as base

FROM ia_gopybase:$IEI_VERSION
LABEL description="Data Analytics image"

ENV PY_WORK_DIR /IEI
WORKDIR ${PY_WORK_DIR}
ENV PYTHONPATH .
ENV HOME /IEI
ENV KAPACITOR_REPO /IEI/go/src/github.com/influxdata/kapacitor
ENV GO_ROOT_BIN /IEI/go/bin
ENV PYTHONPATH ${PYTHONPATH}
ARG IEI_UID

# Installing protobuf module needed for UDF function
RUN mkdir -p /etc/ssl/ca

# Adding kapacitor related files
COPY external/kapacitor ${KAPACITOR_REPO}
RUN cd ${KAPACITOR_REPO} && \
    python2.7 build.py --clean -o ${GO_ROOT_BIN} && \
    mkdir -p ${DIR}/kapacitor && \
    cp -r udf ${DIR}/kapacitor

#Creating dir for kapacitor certs file
RUN mkdir -p /etc/ssl/kapacitor && \
    chown ${IEI_UID} /IEI && \
    chown -R ${IEI_UID} /etc/ssl && \
    mkdir -p /etc/kapacitor && \
    chown ${IEI_UID} /etc/kapacitor && \
    mkdir -p /etc/ssl/ca && \
    chown ${IEI_UID} /etc/ssl/ca
    
# Installing dependent python modules
COPY DataAnalytics/PointDataAnalytics/classifier_requirements.txt .
RUN pip3.6 install -r classifier_requirements.txt && \
    rm -rf classifier_requirements.txt

RUN mkdir -p /IEI/go/src/github.com/golang/glog
COPY --from=base /IEI/go/src/github.com/golang/glog /IEI/go/src/github.com/golang/glog

# Adding classifier program
COPY Util/ ./Util/
COPY libs/ConfigManager ./libs/ConfigManager
COPY libs/common ./libs/common
COPY DataAnalytics/PointDataAnalytics .
RUN chmod +x ./classifier_start.sh

RUN mkdir /IEI/classifier_logs && \
    chown ${IEI_UID} -R /IEI/classifier_logs 

RUN go build -o point_classifier point_classifier.go 
ENTRYPOINT ["./classifier_start.sh"]
HEALTHCHECK NONE
