#!/bin/bash -e

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


set +H
echo "Generating CSR Certificate & Private Key with Common Name (CN) :" $DEFAULT_COMMON_NAME
openssl req -newkey rsa:2048 -nodes -keyout client.key -out client.csr -subj "/CN=$DEFAULT_COMMON_NAME"
echo "Receving Trusted Certificates from the CSL Server"

mkdir -p Certificates/csl
curl -k -u "$CSL_MGR_USERNAME:$CSL_MGR_PASSWORD" -H "Content-Type: multipart/form-data" "https://$CSL_MGR_IP:8443/api/v1/datastore/users/$DEFAULT_COMMON_NAME/csr" -F "csr=@client.csr" | jq -r '."cert.pem"' | base64 -d > Certificates/csl/cert.pem
curl -k -u "$CSL_MGR_USERNAME:$CSL_MGR_PASSWORD" -H "Content-Type: multipart/form-data" "https://$CSL_MGR_IP:8443/api/v1/datastore/users/$DEFAULT_COMMON_NAME/csr" -F "csr=@client.csr" | jq -r '."cacert.pem"' | base64 -d > Certificates/csl/cacert.pem
echo "Setting RW Permission for Root Bucket: $ROOT_BUCKET with User: $DEFAULT_COMMON_NAME User"
curl -k -v -H "Content-type: application/json" -u "$CSL_MGR_USERNAME:$CSL_MGR_PASSWORD" -XPOST https://$CSL_MGR_IP:8443/api/v1/datastore/users/$DEFAULT_COMMON_NAME/permissions -d '{"'$ROOT_BUCKET'": "RW"}'

mv client.key Certificates/csl
