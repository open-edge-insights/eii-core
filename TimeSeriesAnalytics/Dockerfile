# Dockerfile for Point Data Analytics
ARG EIS_VERSION
FROM ia_gobase:$EIS_VERSION as base

FROM ia_gopybase:$EIS_VERSION
LABEL description="Data Analytics image"

ENV PY_WORK_DIR /EIS
WORKDIR ${PY_WORK_DIR}
ENV PYTHONPATH .
ENV HOME /EIS
ENV KAPACITOR_REPO /EIS/go/src/github.com/influxdata/kapacitor
ENV GO_ROOT_BIN /EIS/go/bin
ENV PYTHONPATH ${PYTHONPATH}
ARG EIS_UID

# Installing protobuf module needed for UDF function
RUN mkdir -p /etc/ssl/ca

# Adding kapacitor related files
COPY kapacitor ${KAPACITOR_REPO}
RUN cd ${KAPACITOR_REPO} && \
    python2.7 build.py --clean -o ${GO_ROOT_BIN} && \
    mkdir -p ${DIR}/kapacitor && \
    cp -r udf ${DIR}/kapacitor

#Creating dir for kapacitor certs file
RUN mkdir -p /etc/ssl/kapacitor && \
    chown ${EIS_UID} /EIS && \
    chown -R ${EIS_UID} /etc/ssl && \
    mkdir -p /etc/kapacitor && \
    chown ${EIS_UID} /etc/kapacitor && \
    mkdir -p /etc/ssl/ca && \
    chown ${EIS_UID} /etc/ssl/ca
    
# Installing dependent python modules
COPY classifier_requirements.txt .
RUN pip3.6 install -r classifier_requirements.txt && \
    rm -rf classifier_requirements.txt

RUN mkdir -p /EIS/go/src/github.com/golang/glog
COPY --from=base /EIS/go/src/github.com/golang/glog /EIS/go/src/github.com/golang/glog

# Adding classifier program
COPY . ./
RUN chmod +x ./classifier_start.sh

RUN mkdir /EIS/classifier_logs && \
    chown ${EIS_UID} -R /EIS/classifier_logs 

RUN go build -o point_classifier point_classifier.go 
ENTRYPOINT ["./classifier_start.sh"]
HEALTHCHECK NONE
