// Copyright (c) 2020 Intel Corporation.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM,OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.

#include <iostream>
#include <memory>
#include <string>
#include <stdlib.h>
#include <unistd.h>
#include <thread>
#include <sstream>
#include <fstream>

#include <grpcpp/grpcpp.h>
#include <grpc++/security/credentials.h>
#include <fstream>

#include "rpc.grpc.pb.h"
#include "kv.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using grpc::ClientReaderWriter;

using etcdserverpb::KV;
using etcdserverpb::Watch;
using etcdserverpb::RangeResponse;
using etcdserverpb::RangeRequest;
using etcdserverpb::PutRequest;
using etcdserverpb::RequestOp;
using etcdserverpb::PutResponse;
using etcdserverpb::WatchCreateRequest;
using etcdserverpb::WatchRequest;
using etcdserverpb::WatchResponse;


static std::string get_file_contents(const char *fpath)
{
  std::ifstream finstream(fpath);
  std::string contents((std::istreambuf_iterator<char>(finstream)), std::istreambuf_iterator<char>());
  return contents;
}

class EtcdClient {
    public:

        EtcdClient(const std::string& host, const std::string& port){
            sprintf(address, "%s:%s", host.c_str(), port.c_str());
            kv_stub = KV::NewStub(grpc::CreateChannel(address, grpc::InsecureChannelCredentials()));
        }

        EtcdClient(const std::string& host, const std::string& port, const std::string& cert_file, const std::string& key_file, const std::string ca_file) {
            sprintf(address, "%s:%s", host.c_str(), port.c_str());
            const char* croot = ca_file.c_str();
            const char* ckey = key_file.c_str();
            const char* ccert = cert_file.c_str();

            auto ca_pem = get_file_contents(croot);
            auto key_pem = get_file_contents(ckey);
            auto cert_pem = get_file_contents(ccert);

            ssl_opts.pem_root_certs = ca_pem;
            ssl_opts.pem_private_key = key_pem;
            ssl_opts.pem_cert_chain = cert_pem;

            kv_stub = KV::NewStub(grpc::CreateChannel(address, grpc::SslCredentials( ssl_opts )));
        }
       
        std::string get(std::string& key);
        int put(std::string& key, std::string& value);
        void watch(std::string& key, void (*user_cb)(char *watch_key, char *value, void *cb_user_data), void *user_data);
        void watch_prefix(std::string& key, void (*user_cb)(char *watch_key, char *val, void *cb_user_data), void *user_data);
        
    private:
        char address[30];
        grpc::SslCredentialsOptions ssl_opts;
        std::unique_ptr<KV::Stub> kv_stub;
};
