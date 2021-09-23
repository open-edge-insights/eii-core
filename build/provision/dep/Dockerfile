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
ARG UBUNTU_IMAGE_VERSION
FROM ubuntu:$UBUNTU_IMAGE_VERSION as builder

# Setting python dev env
RUN apt-get update && \
    apt-get install -y --no-install-recommends software-properties-common curl

ARG ETCD_VERSION
RUN mkdir -p /EII/etcd/data
RUN curl -L https://github.com/coreos/etcd/releases/download/${ETCD_VERSION}/etcd-${ETCD_VERSION}-linux-amd64.tar.gz -o /EII/etcd-${ETCD_VERSION}-linux-amd64.tar.gz && \
    tar -xvf /EII/etcd-${ETCD_VERSION}-linux-amd64.tar.gz -C /EII/etcd --strip 1 && \
    rm -f /EII/etcd-${ETCD_VERSION}-linux-amd64.tar.gz

FROM ubuntu:$UBUNTU_IMAGE_VERSION as runtime
ARG EII_UID
COPY --from=builder /EII/etcd /EII/etcd/
WORKDIR /EII/etcd/
COPY dep/start_etcd.sh .
RUN chmod +x start_etcd.sh
RUN chown -R ${EII_UID}:${EII_UID} /EII/etcd/data && \
    chmod 760 /EII/etcd/data

HEALTHCHECK NONE
ENTRYPOINT ["./start_etcd.sh"]
