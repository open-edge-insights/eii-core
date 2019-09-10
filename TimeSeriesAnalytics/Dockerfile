# Dockerfile for Point Data Analytics
ARG EIS_VERSION
FROM ia_eisbase:$EIS_VERSION as eisbase
LABEL description="Data Analytics image"

ENV PY_WORK_DIR /EIS
WORKDIR ${PY_WORK_DIR}
ENV PYTHONPATH .
ENV HOME /EIS
ENV KAPACITOR_REPO /EIS/go/src/github.com/influxdata/kapacitor
ENV GO_ROOT_BIN /EIS/go/bin
ENV PYTHONPATH ${PYTHONPATH}
ARG EIS_UID
ENV KAPACITOR_URL http://localhost:9092/
ENV KAPACITOR_UNSAFE_SSL true
ENV KAPACITOR_INFLUXDB_0_URLS_0 http://localhost:8086/

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

RUN mkdir ${PY_WORK_DIR}/classifier_logs && \
    chown ${EIS_UID} -R ${PY_WORK_DIR}/classifier_logs 

ENV KAPACITOR_SERVER localhost
ENV KAPACITOR_PORT 9092

FROM ia_common:$EIS_VERSION as common

FROM eisbase

COPY --from=common /libs ${PY_WORK_DIR}/libs
COPY --from=common /util ${PY_WORK_DIR}/util

# Adding classifier program
COPY . ./
RUN chmod +x ./classifier_start.sh

RUN go build -o point_classifier point_classifier.go 
ENTRYPOINT ["./classifier_start.sh"]
