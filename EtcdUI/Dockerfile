FROM python:3.7-slim
RUN  apt update && \
     apt install -y curl unzip
ARG ETCD_KEEPER_VERSION
RUN curl -L https://github.com/evildecay/etcdkeeper/releases/download/${ETCD_KEEPER_VERSION}/etcdkeeper-${ETCD_KEEPER_VERSION}-linux_x86_64.zip -o etcdkeeper-${ETCD_KEEPER_VERSION}-linux_x86_64.zip && \
    unzip etcdkeeper-${ETCD_KEEPER_VERSION}-linux_x86_64.zip
RUN rm -rf etcdkeeper-${ETCD_KEEPER_VERSION}-linux_x86_64.zip
RUN chmod +x etcdkeeper/etcdkeeper
ADD start_etcdkeeper.py ./
ENTRYPOINT ["python3", "start_etcdkeeper.py"]
