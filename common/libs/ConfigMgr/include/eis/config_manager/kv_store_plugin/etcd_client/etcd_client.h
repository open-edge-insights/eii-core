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
 * to comminate with etcd server
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
#include <cjson/cJSON.h>
#include "eis/utils/json_config.h"
#include <grpcpp/grpcpp.h>
#include <grpc++/security/credentials.h>
#include <fstream>

#include <eis/config_manager/kv_store_plugin/etcd_client/protobuf/rpc.grpc.pb.h>
#include <eis/config_manager/kv_store_plugin/etcd_client/protobuf/kv.pb.h>

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

/**
 * Format for the user callback to notify the user when any update occurs on a key
 * when watch functions are being called for the key
 * @param key           key is being updated
 * @param value         updated value
 * @param cb_user_data  user data passed
 */
typedef void (*kv_store_watch_callback_t)(const char *key, config_t* value, void* cb_user_data);

class EtcdClient {
    public:
        /**
        * EtcdClient Constructor to connect to etcd server in dev mode
        * @param host - host name to connect to etcd server
        * @param port - port at which etcd server has started
        */
        EtcdClient(const std::string& host, const std::string& port);

        /**
        * EtcdClient Constructor to connect to etcd server in prod mode
        * @param host      - host name to connect to etcd server
        * @param port      - port at which etcd server has started
        * @param cert_file - etcd_client certificate file
        * @param key_file  - etcd_client private key file
        * @param ca_file   - ca_certificate file 
        */
        EtcdClient(const std::string& host, const std::string& port, const std::string& cert_file, const std::string& key_file, const std::string ca_file);
       
        /**
        * Destructor
        */
        ~EtcdClient();
        
        /**
        * Sends a get request to etcd server
        * @param key is the key to be read
        * @return value if found, string lieteral "(NULL)" on failure
        */
        std::string get(std::string& key);

        /**
        * Sends a get request to etcd server
        * @param key is the prefix of the key to be read
        * @return vector with all the values found
        */
        std::vector<std::string> get_prefix(std::string& key_prefix);

        /**
        * Saves the value of a key to etcd. The key will be modified if already exists or created
        * if it does not exist.
        * @param key is the key to be created or modified
        * @param value is the new value to be set
        * @return 0 on success, -1 on failure
        */
        int put(std::string& key, std::string& value);

        /**
        * Watches for changes of a key, registers user_callback and notify 
        * user if any change on key occured 
        * @param key is the value or directory to be watched
        * @param user_callback user_call back to register for a key
        * @param user_data user_data to be passed, it can be NULL also
        */
        void watch(std::string& key, kv_store_watch_callback_t user_cb, void *user_data);

        /**
        * Watches for changes of a prefix of a key and register user_callback and notify 
        * the user if any change on directory(prefix of key) occured 
        * @param key is the value or directory to be watched
        * @param user_callback user_call back to register for a key
        * @param user_data user_data to be passed, it can be NULL also
        */
        void watch_prefix(std::string& key, kv_store_watch_callback_t user_cb, void *user_data);
        
    private:
        char address[ADDRESS_LEN];
        grpc::SslCredentialsOptions ssl_opts;
        std::unique_ptr<KV::Stub> kv_stub;
};

#endif // _EIS_ETCD_CLIENT_H