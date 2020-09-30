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

ARG EIS_VERSION
ARG DOCKER_REGISTRY
FROM ${DOCKER_REGISTRY}ia_eisbase:$EIS_VERSION as eisbase

ARG ETCD_VERSION
RUN apt-get update && \
    apt-get install -y curl && \
    mkdir /EIS/etcd && \
    curl -L https://github.com/coreos/etcd/releases/download/${ETCD_VERSION}/etcd-${ETCD_VERSION}-linux-amd64.tar.gz -o /EIS/etcd-${ETCD_VERSION}-linux-amd64.tar.gz && \
    tar -xvf /EIS/etcd-${ETCD_VERSION}-linux-amd64.tar.gz -C /EIS/etcd --strip 1 && \
    rm -f /EIS/etcd-${ETCD_VERSION}-linux-amd64.tar.gz /EIS/etcd/etcd && \
    mv /EIS/etcd/etcdctl /EIS/ && apt-get remove -y curl

RUN apt-get update && \
    apt-get install -y nginx && \
    apt-get update && apt-get install -y procps

ARG EIS_UID
RUN chown -R ${EIS_UID}:${EIS_UID} /var/log/nginx/ && \
    chown -R ${EIS_UID}:${EIS_UID} /var/lib/nginx/


FROM ${DOCKER_REGISTRY}ia_common:$EIS_VERSION as common

FROM eisbase

COPY --from=common ${GO_WORK_DIR}/common/libs ${PY_WORK_DIR}/libs
COPY --from=common ${GO_WORK_DIR}/common/util ${PY_WORK_DIR}/util

COPY --from=common /usr/local/include /usr/local/include
COPY --from=common /usr/local/lib /usr/local/lib
COPY --from=common /usr/local/lib/python3.6/dist-packages/ /usr/local/lib/python3.6/dist-packages


COPY etcdkeeper ./etcdkeeper/

RUN cd ./etcdkeeper/src/etcdkeeper \
    && go build -o etcdkeeper main.go \
    && mv etcdkeeper ../../

COPY nginx.conf /etc/nginx/nginx.conf
COPY start_etcdkeeper.py ./
COPY eis_nginx_prod.conf ./
COPY eis_nginx_dev.conf ./

RUN touch /run/nginx.pid

ARG EIS_UID
RUN chown -R ${EIS_UID}:${EIS_UID} /run/nginx.pid && \
    ln -sf /dev/stdout /var/log/nginx/access.log && ln -sf /dev/stderr /var/log/nginx/error.log && \
    mkdir -p /tmp/nginx && \
    chown -R ${EIS_UID}:${EIS_UID} /tmp/nginx && \
    rm -rf /var/lib/nginx && ln -sf /tmp/nginx /var/lib/nginx && \
    rm -f /etc/nginx/sites-enabled/default

ENTRYPOINT ["python3.6", "start_etcdkeeper.py"]
