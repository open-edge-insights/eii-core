#! /bin/bash

# Copyright (c) 2021 Intel Corporation.

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

if [ -z $1 ]; then
    echo "Please provide the Kibana Server IP"
    exit
fi

## Creating the Cert directory
if [ -d kibana_certs ]; then
    rm -rf kibana_certs
    mkdir kibana_certs
else
    mkdir kibana_certs
fi

cd kibana_certs

## Generating the key for ca certificate
openssl genrsa -out ca_key.pem 3072

## Generating CA certificate using the key generated above
openssl req -x509 -new -nodes -key ca_key.pem -sha256 -days 1024 -out ca_certificate.pem -subj "/CN=$1"

## Generating the key for server certificate
openssl genrsa -out kibana_server_key.pem 3072

cat >> ext.conf << EOF
[ req ]
prompt             = yes
default_md         = sha256
distinguished_name = root_ca_distinguished_name
req_extensions     = san


[ root_ca_distinguished_name ]
countryName                = Country Name (2 letter code)
countryName_default        = US
countryName_min            = 2
countryName_max            = 2
commonName                 = Common Name (FQDN)
0.organizationName         = Organization Name (eg, company)
0.organizationName_default = root_ca

[ san ]
subjectAltName=DNS:localhost,DNS:*,IP:127.0.0.1,IP:$1
EOF

## Generating the csr for the server certificate
openssl req -new -key kibana_server_key.pem -out kibana_server.csr -config ext.conf -subj "/C=IN/CN=KibanaServer/"

## Generating the server certificate and signing it with the ca certificate
openssl x509 -req -in kibana_server.csr -CA ca_certificate.pem -CAkey ca_key.pem -CAcreateserial -out kibana_server_certificate.pem -days 365 -extensions san -extfile ext.conf
