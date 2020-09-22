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

/**
 * @file
 * @brief Etcd Client library provides APIs with underlying grpc calls
**/

#ifndef _EIS_ETCD_CLIENT_H
#define _EIS_ETCD_CLIENT_H

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

#include <eis/config_manager/protobuf/rpc.grpc.pb.h>
#include <eis/config_manager/protobuf/kv.pb.h>

#define ADDRESS_LEN 30
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

typedef void (*callback_t)(char *key, char *value, void* cb_user_data);

class EtcdClient {
    public:
 
        EtcdClient(const std::string& host, const std::string& port);

        EtcdClient(const std::string& host, const std::string& port, const std::string& cert_file, const std::string& key_file, const std::string ca_file);
       
        /**
        * Destructor
        */
        ~EtcdClient();
        
        std::string get(std::string& key);
        std::vector<std::string> get_prefix(std::string& key_prefix);
        int put(std::string& key, std::string& value);
        void watch(std::string& key, callback_t user_cb, void *user_data);
        void watch_prefix(std::string& key, callback_t user_cb, void *user_data);
        
    private:
        char address[ADDRESS_LEN];
        grpc::SslCredentialsOptions ssl_opts;
        std::unique_ptr<KV::Stub> kv_stub;
};

#endif // _EIS_ETCD_CLIENT_H