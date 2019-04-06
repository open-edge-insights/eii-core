# ImageStore dockerfile
ARG IEI_VERSION
FROM ia_gobase:$IEI_VERSION

ENV GO_WORK_DIR /IEI/go/src/IEdgeInsights
ENV PATH ${PATH}:/usr/local/go/bin:/IEI/go/bin
WORKDIR ${GO_WORK_DIR}
ENV PYTHONPATH .:/IEI/go/src/IEdgeInsights/DataAgent/da_grpc/protobuff

ENV GOPATH /IEI/go

RUN mkdir -p ${GO_WORK_DIR}/log

RUN apt-get update

# Installing build tools
RUN apt-get install -y cmake g++ build-essential

# Installing all golang dependencies
# TODO: Use dep tool itself in future once the "source" value
# is obeyed and just "name" value is not used for deducing the
# repo (https://github.com/golang/dep/pull/1857/commits)

ENV GLOG_GO_PATH ${GOPATH}/src/github.com/golang/glog
RUN mkdir -p ${GLOG_GO_PATH} && \
    git clone https://github.com/golang/glog ${GLOG_GO_PATH} && \
    cd ${GLOG_GO_PATH} && \
    git checkout -b known_version 23def4e6c14b4da8ac2ed8007337bc5eb5007998

ENV REDIS_GO_PATH ${GOPATH}/src/github.com/go-redis/redis
RUN mkdir -p ${REDIS_GO_PATH} && \
    git clone https://github.com/go-redis/redis ${REDIS_GO_PATH} && \
    cd ${REDIS_GO_PATH} && \
    git checkout -b v6.14.1 tags/v6.14.1

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

ENV GO_INI_PATH ${GOPATH}/src/github.com/go-ini/ini
RUN mkdir -p ${GO_INI_PATH} && \
    git clone https://github.com/go-ini/ini ${GO_INI_PATH} && \
    cd ${GO_INI_PATH} && \
    git checkout -b known_version 6ed8d5f64cd79a498d1f3fab5880cc376ce41bbe

ENV GO_X_HOMEDIR ${GOPATH}/src/github.com/mitchellh/go-homedir
RUN mkdir -p ${GO_X_HOMEDIR} && \
    git clone https://github.com/mitchellh/go-homedir ${GO_X_HOMEDIR} && \
    cd ${GO_X_HOMEDIR} && \
    git checkout -b known_version ae18d6b8b3205b561c79e8e5f69bff09736185f4

ENV GO_X_CRYPTO ${GOPATH}/src/golang.org/x/crypto
RUN mkdir -p ${GO_X_CRYPTO} && \
    git clone https://github.com/golang/crypto ${GO_X_CRYPTO} && \
    cd ${GO_X_CRYPTO} && \
    git checkout -b known_version ff983b9c42bc9fbf91556e191cc8efb585c16908

ENV MINIO_GO_PATH ${GOPATH}/src/github.com/minio/minio-go
RUN mkdir -p ${MINIO_GO_PATH} && \
    git clone https://github.com/minio/minio-go ${MINIO_GO_PATH} && \
    cd ${MINIO_GO_PATH} && \
    git checkout -b  v6.0.10 tags/v6.0.10

# Setting timezone inside the container
RUN apt-get update
RUN apt-get -y install build-essential
RUN apt-get -y install tcl

ARG REDIS_VERSION
RUN wget http://download.redis.io/releases/redis-${REDIS_VERSION}.tar.gz
RUN tar xzf redis-${REDIS_VERSION}.tar.gz
RUN cd /IEI/go/src/IEdgeInsights/redis-${REDIS_VERSION} && \
    make && \
    cp /IEI/go/src/IEdgeInsights/redis-${REDIS_VERSION}/src/redis-server /usr/local/bin && \
    cp /IEI/go/src/IEdgeInsights/redis-${REDIS_VERSION}/src/redis-cli /usr/local/bin

ARG MINIO_VERSION
RUN wget https://dl.minio.io/server/minio/release/linux-amd64/archive/minio.${MINIO_VERSION}
RUN mv minio.${MINIO_VERSION} minio
RUN chmod +x minio

# Adding cert dirs
RUN mkdir -p /etc/ssl/imagestore

# These flags are needed for enabling security while compiling and linking with open62541, cpuidcheck in golang
ENV CGO_CFLAGS "$CGO_FLAGS -O2 -D_FORTIFY_SOURCE=2 -Werror=format-security -fstack-protector-strong -fPIC"
ENV CGO_LDFLAGS "$CGO_LDFLAGS -z noexecstack -z relro -z now"

# Building safestringlib
ENV SAFESTRING_VER 77b772849eda2321fb0dca56a321e3939930d7b9
RUN	git clone https://github.com/intel/safestringlib.git && \
	cd safestringlib && \
	git checkout ${SAFESTRING_VER} && \
    make

WORKDIR /IEI/go/src/IEdgeInsights
ADD ImageStore/ ./ImageStore
ADD DataAgent/ ./DataAgent
ADD Util/ ./Util

# Copying safestringlib to DataBusAbstraction and Util
RUN cd safestringlib && \
    cp -rf libsafestring.a ${GO_WORK_DIR}/Util/cpuid

RUN cd Util/cpuid && \
    make

ENV PYTHONPATH ${PYTHONPATH}:./DataAgent/da_grpc/protobuff/py:./DataAgent/da_grpc/protobuff/py/pb_internal:./ImageStore/protobuff/py

RUN go build -o /IEI/go/src/IEdgeInsights/ImageStore/main ImageStore/main.go

RUN mkdir /.minio
ARG IEI_UID
RUN chown -R ${IEI_UID} /.minio
ENTRYPOINT ["./ImageStore/main"]
CMD ["-stderrthreshold", ${GO_LOG_LEVEL}, "-v", ${GO_VERBOSE}]
HEALTHCHECK NONE
