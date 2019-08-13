FROM python:3.7-slim
RUN  apt update && \
     apt install -y curl
ARG ETCD_VERSION
RUN mkdir -p /EIS/etcd/data
RUN curl -L https://github.com/coreos/etcd/releases/download/${ETCD_VERSION}/etcd-${ETCD_VERSION}-linux-amd64.tar.gz -o /EIS/etcd-${ETCD_VERSION}-linux-amd64.tar.gz && \
    tar -xvf /EIS/etcd-${ETCD_VERSION}-linux-amd64.tar.gz -C /EIS/etcd --strip 1 && \
    rm -f /EIS/etcd-${ETCD_VERSION}-linux-amd64.tar.gz
WORKDIR /EIS/etcd/
ADD dep/start_etcd.sh .
RUN chmod +x start_etcd.sh
ENTRYPOINT ["./start_etcd.sh"]
