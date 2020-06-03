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

FROM golang:1.12 as build

ENV GO111MODULE on
ENV GOPROXY "https://goproxy.io"

WORKDIR /opt
RUN mkdir etcdkeeper
ADD . /opt/etcdkeeper
WORKDIR /opt/etcdkeeper/src/etcdkeeper

RUN go mod download \
    && go build -o etcdkeeper.bin main.go


FROM alpine:3.10

ENV HOST="0.0.0.0"
ENV PORT="8080"

# RUN apk add --no-cache ca-certificates

RUN mkdir /lib64 && ln -s /lib/libc.musl-x86_64.so.1 /lib64/ld-linux-x86-64.so.2

WORKDIR /opt/etcdkeeper
COPY --from=build /opt/etcdkeeper/src/etcdkeeper/etcdkeeper.bin .
ADD assets assets

EXPOSE ${PORT}

ENTRYPOINT ./etcdkeeper.bin -h $HOST -p $PORT