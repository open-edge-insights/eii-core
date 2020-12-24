# Copyright (c) 2020 Intel Corporation.

# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

ARG EII_VERSION
ARG DOCKER_REGISTRY
ARG UBUNTU_IMAGE_VERSION
FROM ${DOCKER_REGISTRY}ia_eiibase:$EII_VERSION as base
FROM ${DOCKER_REGISTRY}ia_common:$EII_VERSION as common

FROM base as builder
LABEL description="EtcdUI image"

RUN apt-get update && \
    apt-get install -y --no-install-recommends curl \
                                               procps && \
    rm -rf /var/lib/apt/lists/*

WORKDIR /app
ARG ETCD_VERSION
RUN mkdir -p etcd && \
    curl -L https://github.com/coreos/etcd/releases/download/${ETCD_VERSION}/etcd-${ETCD_VERSION}-linux-amd64.tar.gz -o etcd-${ETCD_VERSION}-linux-amd64.tar.gz && \
    tar -xvf etcd-${ETCD_VERSION}-linux-amd64.tar.gz -C etcd --strip 1 && \
    rm -f etcd-${ETCD_VERSION}-linux-amd64.tar.gz etcd/etcd && \
    mv etcd/etcdctl . && \
    rm -rf etcd/etcdctl

COPY etcdkeeper ./etcdkeeper/

RUN cd ./etcdkeeper/src/etcdkeeper \
    && go build -o etcdkeeper main.go \
    && mv etcdkeeper ../../

FROM ubuntu:$UBUNTU_IMAGE_VERSION as runtime

# Setting python dev env
RUN apt-get update && \
    apt-get install -y --no-install-recommends software-properties-common && \
    add-apt-repository ppa:deadsnakes/ppa && \
    apt-get update && \
    apt-get install -y --no-install-recommends python3.6 \
                                               python3-distutils && \
    rm -rf /var/lib/apt/lists/*

WORKDIR /app

RUN apt-get update && \
    apt-get install -y --no-install-recommends nginx && \
    rm -rf /var/lib/apt/lists/*

ARG EII_UID
RUN chown -R ${EII_UID}:${EII_UID} /var/log/nginx/ && \
    chown -R ${EII_UID}:${EII_UID} /var/lib/nginx/

ARG ARTIFACTS
ARG CMAKE_INSTALL_PREFIX
ENV LD_LIBRARY_PATH $LD_LIBRARY_PATH:${CMAKE_INSTALL_PREFIX}/lib
ENV PYTHONPATH $PYTHONPATH:/app/.local/lib/python3.6/site-packages:/app
COPY --from=common ${CMAKE_INSTALL_PREFIX}/lib ${CMAKE_INSTALL_PREFIX}/lib
COPY --from=common /eii/common/util util
COPY --from=common /root/.local/lib .local/lib
COPY --from=builder /app .

RUN chown -R ${EII_UID} .local/lib/python3.6

COPY nginx.conf /etc/nginx/nginx.conf
COPY start_etcdkeeper.py ./
COPY eii_nginx_prod.conf ./
COPY eii_nginx_dev.conf ./

RUN touch /run/nginx.pid

ARG EII_UID
RUN chown -R ${EII_UID}:${EII_UID} /run/nginx.pid && \
    ln -sf /dev/stdout /var/log/nginx/access.log && ln -sf /dev/stderr /var/log/nginx/error.log && \
    mkdir -p /opt/nginx && \
    chown -R ${EII_UID}:${EII_UID} /opt/nginx && \
    rm -rf /var/lib/nginx && ln -sf /opt/nginx /var/lib/nginx && \
    rm -f /etc/nginx/sites-enabled/default

HEALTHCHECK NONE
ENTRYPOINT ["python3", "start_etcdkeeper.py"]
