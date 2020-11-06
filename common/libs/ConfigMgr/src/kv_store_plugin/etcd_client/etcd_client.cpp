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

#include <exception>
#include <thread>
#include <stdlib.h>

#include <safe_lib.h>
#include <eis/utils/logger.h>
#include <eis/config_manager/kv_store_plugin/etcd_client/etcd_client.h>

#define NO_VALUE_ERROR    "CHECK failed: (index) < (current_size_): "

static std::string get_file_contents(const char *fpath) {
  std::ifstream finstream(fpath);
  std::string contents((std::istreambuf_iterator<char>(finstream)), std::istreambuf_iterator<char>());
  return contents;
}


EtcdClient::EtcdClient(const std::string& host, const std::string& port) {
    LOG_INFO("Initialize EtcdClient in Dev mode");
    kv_stub = NULL;

    LOG_DEBUG("host:%s and port:%s", host.c_str(), port.c_str());
    // TODO: Add port check availability function
    sprintf(address, "%s:%s", host.c_str(), port.c_str());
    
    try {
        kv_stub = KV::NewStub(grpc::CreateChannel(address, grpc::InsecureChannelCredentials()));
    }catch(...) {
        LOG_ERROR("Exception Occurred while creating grpc channel for KV Store");
        throw "KV Channel Creation Failed";
    }
}

EtcdClient::EtcdClient(const std::string& host, const std::string& port, const std::string& cert_file, 
                       const std::string& key_file, const std::string ca_file) {
    LOG_INFO("Initialize EtcdClient in Prod mode");
    LOG_DEBUG("host:%s and port:%s", host.c_str(), port.c_str());
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

    try {
        kv_stub = KV::NewStub(grpc::CreateChannel(address, grpc::SslCredentials(ssl_opts)));
    }catch(...) {
        LOG_ERROR("Exception Occurred while creating grpc channel for KV Store");
        throw "KV Channel Creation Failed";
    }
}

/**
* Sends a get request to the etcd server
* @param key is the key to be read
*/
std::string EtcdClient::get(std::string& key) {
    LOG_DEBUG_0("In get() API");
    LOG_DEBUG("get value for the key %s", key.c_str());
    mvccpb::KeyValue kvs;
    RangeRequest get_request;
    RangeResponse reply;
    Status status;
    ClientContext context;

    try {
        char* etcd_prefix = getenv("ETCD_PREFIX");
        if (etcd_prefix == NULL) {
            LOG_DEBUG_0("ETCD_PREFIX env not set, fetching key without ETCD_PREFIX");
        } else {
            if (strlen(etcd_prefix) != 0) {
                std::string prefix(etcd_prefix);
                key = prefix + key;
            }
        }
        get_request.set_key(key);
        status = kv_stub->Range(&context,get_request,&reply);
        if (status.ok()) {
            kvs.CopyFrom(reply.kvs(0));
        }else {
            LOG_DEBUG("get() API Failed with Error:%s and Error Code: %d", 
                status.error_message().c_str(), status.error_code());
            return "(NULL)";
        }
    } catch(std::exception const & ex) {
        LOG_DEBUG("Exception Occurred in get() API with the Error: %s", ex.what());
        int no_val_error;
        strcmp_s(NO_VALUE_ERROR, strlen(NO_VALUE_ERROR), ex.what(), &no_val_error);
        if(no_val_error == 0) {
            LOG_DEBUG("Value for the key %s is not found", key.c_str());
        }
        return "(NULL)";
    }
    return kvs.value();
}

std::vector<std::string> EtcdClient::get_prefix(std::string& key_prefix) {
    LOG_DEBUG_0("In get_prefix() API");
    LOG_DEBUG("get all values for keys starting from %s", key_prefix.c_str());
    mvccpb::KeyValue kvs;
    RangeRequest get_request;
    RangeResponse reply;
    Status status;
    ClientContext context;
    std::vector<std::string> values;
    std::vector<std::string>::iterator it;

    std::string& range_end = key_prefix;

    try {
        char* etcd_prefix = getenv("ETCD_PREFIX");
        if (etcd_prefix == NULL) {
            LOG_DEBUG_0("ETCD_PREFIX env not set, fetching key without ETCD_PREFIX");
        } else {
            if (strlen(etcd_prefix) != 0) {
                std::string prefix(etcd_prefix);
                key_prefix = prefix + key_prefix;
                range_end = key_prefix;
            }
        }
        get_request.set_key(key_prefix);
       
        int ascii = (int)range_end[range_end.length()-1];
        range_end.back() = ascii+1;

        get_request.set_range_end(range_end);

        status = kv_stub->Range(&context,get_request,&reply);

        if (status.ok()) {
            for(int i=0; i<reply.count(); i++) {
                kvs.CopyFrom(reply.kvs(i));
                values.push_back( kvs.value());
            }
        }else {
            LOG_DEBUG("get() API Failed with Error:%s and Error Code: %d", 
                status.error_message().c_str(), status.error_code());
        }
    } catch(std::exception const & ex) {
        int no_val_error;
        LOG_DEBUG("Exception Occurred in get() API with the Error: %s", ex.what());
        strcmp_s(NO_VALUE_ERROR, strlen(NO_VALUE_ERROR), ex.what(), &no_val_error);
        if(no_val_error == 0) {
            LOG_DEBUG("Value for the key %s is not found", key_prefix.c_str());
        }
    }

    return values;
}

void register_watch(char* address, grpc::SslCredentialsOptions ssl_opts, 
                    WatchRequest watch_req, callback_t user_callback, void *user_data) {
    WatchResponse reply;
    mvccpb::KeyValue kvs;
    ClientContext context;

    std::unique_ptr<Watch::Stub> watch_stub;

    if((ssl_opts.pem_root_certs.empty()) && (ssl_opts.pem_private_key.empty()) \
            && (ssl_opts.pem_cert_chain.empty())) {
        watch_stub = Watch::NewStub(grpc::CreateChannel(address, grpc::InsecureChannelCredentials()));
    }else {
        watch_stub = Watch::NewStub(grpc::CreateChannel(address, grpc::SslCredentials(ssl_opts)));
    }

    // TODO: make use of AsyncWatch instead of Watch
    std::unique_ptr<ClientReaderWriter<WatchRequest, WatchResponse> > stream = watch_stub->Watch(&context);

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
                    LOG_DEBUG("key:%s is updated with the value %s", kvs_key, kvs_value);

                    cJSON* val_json;
                    // Checking if the value updated is not in Json format
                    if (kvs_value[0] != '{'){
                       if(strlen(kvs_value) == 0){
                           LOG_ERROR_0("Value shouldn't be empty. Empty string is not supported");
                           return;
                       }
                       // Creating the cJSON object with Key as kvs_key and value as kvs_value
                       val_json = cJSON_CreateObject();
                       if(val_json == NULL){
                           LOG_ERROR_0("Create json object failed");
                           return;
                       }
                       cJSON_AddStringToObject(val_json, kvs_key, kvs_value);
                    } else{
                        // char* to cJSON conversion
                        val_json = cJSON_Parse(kvs_value);
                        if(val_json == NULL){
                           LOG_ERROR_0("cJSON Parse failed");
                           return;
                       }
                    }                  

                    // cJSON to config_t conversion
                    config_t* config = config_new(
                        (void*) val_json, free_json, get_config_value);
                    if (config == NULL) {
                        LOG_ERROR_0("Failed to initialize configuration object");
                        return;
                    }
                    user_callback(kvs_key, config, user_data);         
                }
            }
        }
    }
}

/**
* Watches for changes of a prefix of a key and register user_callback and notify 
* the user if any change on directory(prefix of key) occured 
* @param key is the value or directory to be watched
* @param user_callback user_call back to register for a key
* @param user_data user_data to be passed, it can be NULL also
*/
void EtcdClient::watch_prefix(std::string& key, callback_t user_callback, void *user_data) {
    LOG_DEBUG_0("In watch_prefix() API");
    LOG_DEBUG("Register the prefix of the the key %s to watch on", key.c_str());

    WatchResponse reply;
    ClientContext context;
    mvccpb::KeyValue kvs;
    WatchRequest watch_req;
    WatchCreateRequest watch_create_req;

    int revision = 0;
    std::string& range_end = key;
    
    try{
        char* etcd_prefix = getenv("ETCD_PREFIX");
        if (etcd_prefix == NULL) {
            LOG_DEBUG_0("ETCD_PREFIX env not set, fetching key without ETCD_PREFIX");
        } else {
            if (strlen(etcd_prefix) != 0) {
                std::string prefix(etcd_prefix);
                key = prefix + key;
            }
        }
        watch_create_req.set_key(key);
        watch_create_req.set_prev_kv(false);

        int ascii = (int)range_end[range_end.length()-1];
        range_end.back() = ascii+1;

        watch_create_req.set_range_end(range_end);
        watch_create_req.set_start_revision(revision);
        watch_req.mutable_create_request()->CopyFrom(watch_create_req);

        std::thread register_watch_prefix_thread(register_watch, address, ssl_opts, watch_req, user_callback, user_data);
        register_watch_prefix_thread.detach();
    } catch(std::exception const & ex) {
        LOG_ERROR("Exception Occurred in watch_prefix() API with the Error: %s", ex.what());
        return;
    }
}

/**
* Watches for changes of a key, registers user_callback and notify 
* user if any change on key occured 
* @param key is the value or directory to be watched
* @param user_callback user_call back to register for a key
* @param user_data user_data to be passed, it can be NULL also
*/
void EtcdClient::watch(std::string& key, callback_t user_callback, void *user_data) {
    LOG_DEBUG_0("In watch() API");
    LOG_DEBUG("Register the key %s to watch on", key.c_str());

    WatchResponse reply;
    ClientContext context;
    mvccpb::KeyValue kvs;
    WatchRequest watch_req;
    WatchCreateRequest watch_create_req;

    int revision = 0;

    try{
        char* etcd_prefix = getenv("ETCD_PREFIX");
        if (etcd_prefix == NULL) {
            LOG_DEBUG_0("ETCD_PREFIX env not set, fetching key without ETCD_PREFIX");
        } else {
            if (strlen(etcd_prefix) != 0) {
                std::string prefix(etcd_prefix);
                key = prefix + key;
            }
        }
        watch_create_req.set_key(key);
        watch_create_req.set_prev_kv(false);
        watch_create_req.set_start_revision(revision);
        watch_req.mutable_create_request()->CopyFrom(watch_create_req);

        std::thread register_watch_thread(register_watch, address, ssl_opts, watch_req, user_callback, user_data);
        LOG_DEBUG("Thread created to wait on any change on the key %s", key.c_str());
        register_watch_thread.detach();
    } catch(std::exception const & ex) {
        LOG_ERROR("Exception Occurred in watch() API with the Error: %s", ex.what());
        return;
    }
}

/**
* Saves the value of a key to etcd. The key will be modified if already exists or created
* if it does not exist.
* @param key is the key to be created or modified
* @param value is the new value to be set
*/
int EtcdClient::put(std::string& key, std::string& value) {
    LOG_DEBUG_0("In put() API");

    int64_t leaseid = 0;
    mvccpb::KeyValue kvs;
    PutRequest put_request;
    PutResponse reply;
    Status status;
    ClientContext context;
   
    LOG_DEBUG("Store the value %s for the key %s", value.c_str(), key.c_str());

    try {
        char* etcd_prefix = getenv("ETCD_PREFIX");
        if (etcd_prefix == NULL) {
            LOG_DEBUG_0("ETCD_PREFIX env not set, fetching key without ETCD_PREFIX");
        } else {
            if (strlen(etcd_prefix) != 0) {
                std::string prefix(etcd_prefix);
                key = prefix + key;
            }
        }
        put_request.set_key(key);
        put_request.set_value(value);
        put_request.set_prev_kv(false);
        put_request.set_lease(leaseid);
        status = kv_stub->Put(&context,put_request,&reply);
    
        if (!status.ok()) {
            LOG_ERROR("Failed to put value %s for key %s", value.c_str(), key.c_str());
            LOG_ERROR("put() API Failed with Error:%s", status.error_message().c_str());
            return -1;
        }
    } catch(std::exception const & ex) {
        LOG_ERROR("Exception Occurred in put() API with the Error: %s", ex.what());
        return -1;
    }
    LOG_DEBUG_0("put() is successful");
    LOG_INFO("key:%s has been created/updated with the value:%s", key.c_str(), value.c_str());
    return 0;
}

EtcdClient::~EtcdClient() {
    LOG_DEBUG_0("EtcdClient Destructor is called");
    if (kv_stub != NULL) {
        kv_stub.reset();
    }
}
