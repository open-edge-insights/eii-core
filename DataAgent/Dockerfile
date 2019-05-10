# Dockerfile for DataAgent

ARG IEI_VERSION
FROM ia_gobase:$IEI_VERSION
LABEL description="DataAgent image"

ARG IEI_UID

RUN mkdir -p ${GO_WORK_DIR}/log

# Installing build tools
RUN apk add --no-cache cmake g++  

RUN go get -u github.com/golang/dep/cmd/dep

# Installing all golang dependencies
# TODO: Use dep tool itself in future once the "source" value
# is obeyed and just "name" value is not used for deducing the
# repo (https://github.com/golang/dep/pull/1857/commits)

ENV TOML_GO_PATH ${GOPATH}/src/github.com/BurntSushi/toml
RUN mkdir -p ${TOML_GO_PATH} && \
    git clone https://github.com/BurntSushi/toml ${TOML_GO_PATH} && \
    cd ${TOML_GO_PATH} && \
    git checkout -b v0.3.0 tags/v0.3.0

ENV GLOG_GO_PATH ${GOPATH}/src/github.com/golang/glog
RUN mkdir -p ${GLOG_GO_PATH} && \
    git clone https://github.com/golang/glog ${GLOG_GO_PATH} && \
    cd ${GLOG_GO_PATH} && \
    git checkout -b known_version 23def4e6c14b4da8ac2ed8007337bc5eb5007998

ENV UUID_GO_PATH ${GOPATH}/src/github.com/google/uuid
RUN mkdir -p ${UUID_GO_PATH} && \
    git clone https://github.com/google/uuid ${UUID_GO_PATH} && \
    cd ${UUID_GO_PATH} && \
    git checkout -b known_version 9b3b1e0f5f99ae461456d768e7d301a7acdaa2d8

ENV PROTOBUF_GO_PATH ${GOPATH}/src/github.com/golang/protobuf
RUN mkdir -p ${PROTOBUF_GO_PATH} && \
    git clone https://github.com/golang/protobuf ${PROTOBUF_GO_PATH} && \
    cd ${PROTOBUF_GO_PATH} && \
    git checkout -b v1.1.0 tags/v1.1.0

ENV GO_X_NET ${GOPATH}/src/golang.org/x/net
RUN mkdir -p ${GO_X_NET} && \
    git clone https://github.com/golang/net ${GO_X_NET} && \
    cd ${GO_X_NET} && \
    git checkout -b known_version 26e67e76b6c3f6ce91f7c52def5af501b4e0f3a2

ENV GO_X_TEXT ${GOPATH}/src/golang.org/x/text
RUN mkdir -p ${GO_X_TEXT} && \
    git clone https://github.com/golang/text ${GO_X_TEXT} && \
    cd ${GO_X_TEXT} && \
    git checkout -b v0.3.0 tags/v0.3.0

ENV GO_X_SYS ${GOPATH}/src/golang.org/x/sys
RUN mkdir -p ${GO_X_SYS} && \
    git clone https://github.com/golang/sys ${GO_X_SYS} && \
    cd ${GO_X_SYS} && \
    git checkout -b known_version d0be0721c37eeb5299f245a996a483160fc36940

ENV GO_GRPC ${GOPATH}/src/google.golang.org/grpc
RUN mkdir -p ${GO_GRPC} && \
    git clone https://github.com/grpc/grpc-go ${GO_GRPC} && \
    cd ${GO_GRPC} && \
    git checkout -b v1.13.0 tags/v1.13.0

ENV GO_PROTOGEN ${GOPATH}/src/google.golang.org/genproto
RUN mkdir -p ${GO_PROTOGEN} && \
    git clone https://github.com/google/go-genproto ${GO_PROTOGEN} && \
    cd ${GO_PROTOGEN} && \
    git checkout -b known_version 4b56f30a1fd96a133a036b62cdd2a249883dd89b

ENV PAHO_MQTT_GO_PATH ${GOPATH}/src/github.com/eclipse/paho.mqtt.golang
RUN mkdir -p ${PAHO_MQTT_GO_PATH} && \
    git clone https://github.com/eclipse/paho.mqtt.golang ${PAHO_MQTT_GO_PATH} && \
    cd ${PAHO_MQTT_GO_PATH} && \
    git checkout -b v1.1.1 tags/v1.1.1

ENV INFLUXDB_GO_PATH ${GOPATH}/src/github.com/influxdata/influxdb
RUN mkdir -p ${INFLUXDB_GO_PATH} && \
    git clone https://github.com/influxdata/influxdb ${INFLUXDB_GO_PATH} && \
    cd ${INFLUXDB_GO_PATH} && \
    git checkout -b v1.6.0 tags/v1.6.0

ENV VAULT_GO_PATH ${GOPATH}/src/github.com/hashicorp/vault
RUN mkdir -p ${VAULT_GO_PATH} && \
    git clone https://github.com/hashicorp/vault ${VAULT_GO_PATH} && \
    cd  ${VAULT_GO_PATH} && \
    git checkout -b v1.0.0-beta2 tags/v1.0.0-beta2 && \
    go install

# tls, crypto library needs to open62541
RUN apk add --no-cache mbedtls-dev

# Installing influxdb
ARG INFLUXDB_VERSION
RUN set -ex && \
    apk add --no-cache --virtual .build-deps wget gnupg tar ca-certificates && \
    update-ca-certificates && \
    gpg --list-keys && \
    echo "honor-http-proxy" >> ~/.gnupg/dirmngr.conf && \
    for key in \
        05CE15085FC09D18E99EFB22684A14CF2582E0C5 ; \
    do \
        gpg --keyserver ha.pool.sks-keyservers.net --recv-keys "$key" || \
        gpg --keyserver pgp.mit.edu --recv-keys "$key" || \
        gpg --keyserver keyserver.pgp.com --recv-keys "$key" ; \
    done && \
    wget --no-verbose https://dl.influxdata.com/influxdb/releases/influxdb-${INFLUXDB_VERSION}-static_linux_amd64.tar.gz.asc && \
    wget --no-verbose https://dl.influxdata.com/influxdb/releases/influxdb-${INFLUXDB_VERSION}-static_linux_amd64.tar.gz && \
    gpg --batch --verify influxdb-${INFLUXDB_VERSION}-static_linux_amd64.tar.gz.asc influxdb-${INFLUXDB_VERSION}-static_linux_amd64.tar.gz && \
    mkdir -p /usr/src && \
    tar -C /usr/src -xzf influxdb-${INFLUXDB_VERSION}-static_linux_amd64.tar.gz && \
    rm -f /usr/src/influxdb-*/influxdb.conf && \
    chmod +x /usr/src/influxdb-*/* && \
    cp -a /usr/src/influxdb-*/* /usr/bin/ && \
    rm -rf *.tar.gz* /usr/src  && \
    apk del .build-deps

# These flags are needed for enabling security while compiling and linking with open62541, cpuidcheck in golang
ENV CGO_CFLAGS "$CGO_FLAGS -O2 -D_FORTIFY_SOURCE=2 -Werror=format-security -fstack-protector-strong -fPIC"
ENV CGO_LDFLAGS "$CGO_LDFLAGS -z noexecstack -z relro -z now"

RUN apk add --no-cache --virtual .build-deps \
    musl \
    musl-dev \
    iproute2 \
    gcc \
    m4 \
    libtool \
    automake \
    libressl-dev \
    uthash-dev \
    autoconf \
    npth-dev 
        

RUN apk add --no-cache -X http://dl-cdn.alpinelinux.org/alpine/edge/testing \
  autoconf-archive  

RUN apk add --no-cache -X http://dl-cdn.alpinelinux.org/alpine/edge/main \
  libgcrypt-dev \
  curl-dev

ENV TPM_PATH ./TPM_SRC
RUN mkdir -p ${TPM_PATH}/tpm2_tss && \
    git clone https://github.com/tpm2-software/tpm2-tss.git ${TPM_PATH}/tpm2_tss && \
    cd ${TPM_PATH}/tpm2_tss && git checkout -b 2.1.0 tags/2.1.0

RUN cd ${TPM_PATH}/tpm2_tss/ && ./bootstrap &&  ./configure && make -j8 && make install


# build TPM-tool
RUN mkdir -p ${TPM_PATH}/tpm2_tools && \
    git clone https://github.com/tpm2-software/tpm2-tools.git ${TPM_PATH}/tpm2_tools && \
    cd ${TPM_PATH}/tpm2_tools && git checkout -b sma-pv 9f765f1959f2a4ee9683165071d05f9878d78443

RUN cd ${TPM_PATH}/tpm2_tools/ && ./bootstrap &&  ./configure && make -j8 && make install

# Adding the CERTS dir
RUN mkdir -p /etc/ssl/influxdb/ \
    && mkdir -p /etc/ssl/opcua/ \
    && mkdir -p /etc/ssl/streammanager/ \
    && mkdir -p /etc/ssl/ca/

# Building safestringlib
ENV SAFESTRING_VER 77b772849eda2321fb0dca56a321e3939930d7b9
RUN	git clone https://github.com/intel/safestringlib.git && \
	cd safestringlib && \
	git checkout ${SAFESTRING_VER} && \
    make

RUN mkdir -p ./vault/file

ENV PYTHONPATH .:./DataAgent/da_grpc/protobuff:./DataAgent/da_grpc/protobuff/py:./DataAgent/da_grpc/protobuff/py/pb_internal

RUN chown ${IEI_UID} /IEI/go/src/IEdgeInsights && \
    chown -R ${IEI_UID} /etc/ssl/
    
ADD DataAgent ./DataAgent
ADD Util ./Util
ADD DataBusAbstraction/ ./DataBusAbstraction/
ADD ImageStore/ ./ImageStore/
ADD StreamManager ./StreamManager
ADD docker_setup/config/vault_config.hcl ./vault/config/vault_config.hcl

# Copying safestringlib to DataBusAbstraction and Util
RUN cd safestringlib && \
    cp -rf libsafestring.a ${GO_WORK_DIR}/DataBusAbstraction/go && \
    cp -rf libsafestring.a ${GO_WORK_DIR}/DataBusAbstraction/c/open62541/src && \
    cp -rf libsafestring.a ${GO_WORK_DIR}/Util/cpuid

# building libopen62541W.a for cgo
RUN cd DataBusAbstraction/go/test && \
    make build_lib_for_docker

RUN cd Util/cpuid && \
    make && \
    apk del .build-deps && \
    rm -rf /var/cache/apk/*

RUN go build -o /IEI/go/bin/DataAgent DataAgent/DataAgent.go
ARG DEV_MODE

ENTRYPOINT ["./DataAgent/da_startup.sh"]
CMD ["-stderrthreshold", ${GO_LOG_LEVEL}, "-v", ${GO_VERBOSE}, "-m", ${DEV_MODE}]
HEALTHCHECK NONE
