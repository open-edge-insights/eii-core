# Dockerfile for DataAgent
ARG IEI_VERSION
FROM ia_gobase:${IEI_VERSION}
LABEL description="DataAgent image"

RUN mkdir -p ${GO_WORK_DIR}/log && \
    apt-get update

# Installing all golang dependencies
# TODO: Use dep tool itself in future once the "source" value
# is obeyed and just "name" value is not used for deducing the
# repo (https://github.com/golang/dep/pull/1857/commits)

ENV TOML_GO_PATH ${GOPATH}/src/github.com/BurntSushi/toml
RUN mkdir -p ${TOML_GO_PATH} && \
    git clone https://github.com/BurntSushi/toml ${TOML_GO_PATH} && \
    cd ${TOML_GO_PATH} && \
    git checkout -b v0.3.0 tags/v0.3.0

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
RUN apt-get install -y libmbedtls-dev

# Installing influxdb
ARG INFLUXDB_VERSION
RUN wget https://dl.influxdata.com/influxdb/releases/influxdb_${INFLUXDB_VERSION}_amd64.deb && \
    dpkg -i influxdb_${INFLUXDB_VERSION}_amd64.deb && \
    rm -rf influxdb_${INFLUXDB_VERSION}_amd64.deb

# These flags are needed for enabling security while compiling and linking with open62541, cpuidcheck in golang
ENV CGO_CFLAGS "$CGO_FLAGS -O2 -D_FORTIFY_SOURCE=2 -Werror=format-security -fstack-protector-strong -fPIC"
ENV CGO_LDFLAGS "$CGO_LDFLAGS -z noexecstack -z relro -z now"

# Installing TPM
RUN apt-get -y install \
  autoconf-archive \
  libcmocka0 \
  libcmocka-dev \
  iproute2 \
  pkg-config \
  gcc \
  m4 \
  libtool \
  automake \
  libgcrypt20-dev \
  libssl-dev \
  uthash-dev \
  autoconf \
  gnulib \
  libcurl4-gnutls-dev

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
RUN mkdir -p /etc/ssl/influxdb/ && \
    mkdir -p /etc/ssl/opcua/ && \
    mkdir -p /etc/ssl/streammanager/ && \
    mkdir -p /etc/ssl/ca/

# Building safestringlib
ENV SAFESTRING_VER 77b772849eda2321fb0dca56a321e3939930d7b9
RUN	git clone https://github.com/intel/safestringlib.git && \
	cd safestringlib && \
	git checkout ${SAFESTRING_VER} && \
    make

ADD DataAgent ./DataAgent
ADD Util ./Util
ADD DataBusAbstraction/ ./DataBusAbstraction/

ADD ImageStore/ ./ImageStore/
ADD StreamManager ./StreamManager
ADD docker_setup/config/vault_config.hcl ./vault/config/vault_config.hcl
RUN mkdir -p ./vault/file

# Copying safestringlib to DataBusAbstraction and Util
RUN cd safestringlib && \
    cp -rf libsafestring.a ${GO_WORK_DIR}/DataBusAbstraction/go && \
    cp -rf libsafestring.a ${GO_WORK_DIR}/DataBusAbstraction/c/open62541/src && \
    cp -rf libsafestring.a ${GO_WORK_DIR}/Util/cpuid

# building libopen62541W.a for cgo
RUN cd DataBusAbstraction/go/test && \
    make build_lib_for_docker

ENV PYTHONPATH ${PYTHONPATH}:./DataAgent/da_grpc/protobuff/py:./DataAgent/da_grpc/protobuff/py/pb_internal

RUN cd Util/cpuid && \
    make

RUN go build -o /IEI/go/bin/DataAgent DataAgent/DataAgent.go

ARG IEI_UID
RUN chown ${IEI_UID} /IEI/go/src/IEdgeInsights
RUN chown -R ${IEI_UID} /etc/ssl/
ENTRYPOINT ["./DataAgent/da_startup.sh"]
CMD ["-stderrthreshold", ${GO_LOG_LEVEL}, "-v", ${GO_VERBOSE}, "-m", ${DEV_MODE}]
HEALTHCHECK NONE

