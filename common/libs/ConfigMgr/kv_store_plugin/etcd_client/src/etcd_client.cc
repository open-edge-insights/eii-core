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

#include "etcd_client.h"
#include <exception>
#include <thread>

static std::string get_file_contents(const char *fpath) {
  std::ifstream finstream(fpath);
  std::string contents((std::istreambuf_iterator<char>(finstream)), std::istreambuf_iterator<char>());
  return contents;
}

EtcdClient::EtcdClient(const std::string& host, const std::string& port) {
    sprintf(address, "%s:%s", host.c_str(), port.c_str());
    kv_stub = KV::NewStub(grpc::CreateChannel(address, grpc::InsecureChannelCredentials()));
}

EtcdClient::EtcdClient(const std::string& host, const std::string& port, const std::string& cert_file, 
                       const std::string& key_file, const std::string ca_file) {
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

    kv_stub = KV::NewStub(grpc::CreateChannel(address, grpc::SslCredentials(ssl_opts)));
}

std::string EtcdClient::get(std::string& key_test) {
    mvccpb::KeyValue kvs;
    RangeRequest get_request;
    RangeResponse reply;
    Status status;
    ClientContext context;

    get_request.set_key(key_test);
    status = kv_stub->Range(&context,get_request,&reply);

    if (status.ok()) {
        kvs.CopyFrom(reply.kvs(0));
    }

    return kvs.value();
}


void register_watch(char* address, grpc::SslCredentialsOptions ssl_opts, 
                    WatchRequest watch_req, void (*user_callback)(char *key,
                    char *val, void *cb_user_data), void *user_data) {
    WatchResponse reply;
    mvccpb::KeyValue kvs;
    ClientContext context;

    std::unique_ptr<Watch::Stub> watch_stub;

    std::string empty_string("");

    if((ssl_opts.pem_root_certs ==  empty_string) && (ssl_opts.pem_private_key == empty_string) && (ssl_opts.pem_cert_chain == empty_string)) {
        watch_stub = Watch::NewStub(grpc::CreateChannel(address, grpc::InsecureChannelCredentials()));
    } else{
        watch_stub = Watch::NewStub(grpc::CreateChannel(address, grpc::SslCredentials(ssl_opts)));
    }

    std::unique_ptr<ClientReaderWriter<WatchRequest, WatchResponse> > stream = watch_stub->Watch(&context);
    std::cout << "thread id:" << std::this_thread::get_id() << std::endl;

    stream->Write(watch_req);

    while(stream->Read(&reply)){
        if(reply.events_size())
        { 
            for(int cnt =0; cnt < reply.events_size(); cnt++){
                auto event = reply.events(cnt);
                if(mvccpb::Event::EventType::Event_EventType_PUT == event.type())
                {
                    kvs = event.kv();
                    char *kvs_key = const_cast<char*>(kvs.key().c_str());
                    char *kvs_value = const_cast<char*>(kvs.value().c_str());
                    user_callback(kvs_key, kvs_value, user_data);
                    
                }
            }
        }
    }
}


void EtcdClient::watch_prefix(std::string& key, void (*user_cb)(char *key, char *val, void *cb_user_data), void *user_data) {
    WatchResponse reply;
    Status status;
    ClientContext context;
    mvccpb::KeyValue kvs;
    WatchRequest watch_req;
    WatchCreateRequest watch_create_req;

    int revision = 0;
    std::string& range_end = key;

    watch_create_req.set_key(key);
    watch_create_req.set_prev_kv(false);

    int ascii = (int)range_end[range_end.length()-1];
    range_end.back() = ascii+1;

    watch_create_req.set_range_end(range_end);
    watch_create_req.set_start_revision(revision);
    watch_req.mutable_create_request()->CopyFrom(watch_create_req);

    std::thread register_watch_prefix_thread(register_watch, address, ssl_opts, watch_req, user_cb, user_data);
    register_watch_prefix_thread.detach();
}


void EtcdClient::watch(std::string& key, void (*user_cb)(char *key, char *val, void *cb_user_data), void *user_data) {
    WatchResponse reply;
    Status status;
    ClientContext context;
    mvccpb::KeyValue kvs;
    WatchRequest watch_req;
    WatchCreateRequest watch_create_req;

    int revision = 0;
    watch_create_req.set_key(key);
    watch_create_req.set_prev_kv(false);
    watch_create_req.set_start_revision(revision);
    watch_req.mutable_create_request()->CopyFrom(watch_create_req);

    std::thread register_watch_thread(register_watch, address, ssl_opts, watch_req, user_cb, user_data);
    register_watch_thread.detach();
}


int EtcdClient::put(std::string& key_test, std::string& value_test) {
    int64_t leaseid = 0;
    mvccpb::KeyValue kvs;
    PutRequest put_request;
    PutResponse reply;
    Status status;
    ClientContext context;

    put_request.set_key(key_test);
    put_request.set_value(value_test);
    put_request.set_prev_kv(false);
    put_request.set_lease(leaseid);
   
    status = kv_stub->Put(&context,put_request,&reply);
   
    if (!status.ok()) {
        std::cout << "put_config() is failed....\nError:" << status.error_message() << std::endl;
        return -1;
    }
    return 0;
}

EtcdClient::~EtcdClient() {
    std::cout << "Destructor is called\n";
    if (kv_stub != NULL) {
        std::cout << "kv_stub is not NULL\n";
        kv_stub.reset();
    }
}