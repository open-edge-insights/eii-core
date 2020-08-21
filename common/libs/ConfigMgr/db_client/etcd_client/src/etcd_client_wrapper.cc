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

#include <cstdlib>
#include "db_client.h"
#include "etcd_client.h"

using namespace eis::etcdcli;

#ifdef __cplusplus
extern "C" {
#endif

void* etcd_init(void *etcd_client) {
    EtcdClient *etcd_cli = NULL;
    db_client_t *db_client = static_cast<db_client_t *>(etcd_client);
    etcd_config_t *etcd_config = static_cast<etcd_config_t *>(db_client->db_config);
    std::string host = etcd_config->hostname;
    std::string port = etcd_config->port;

    if(strcmp(etcd_config->cert_file, "") && strcmp(etcd_config->key_file, "") && strcmp(etcd_config->ca_cert_file, "")){
        std::cout << "Running in prod mode\n";
        etcd_cli = new EtcdClient(host, port,  etcd_config->cert_file, etcd_config->key_file, etcd_config->ca_cert_file);
    } else{
        std::cout << "Running in dev mode\n";
        etcd_cli = new EtcdClient(host, port);
    }

    db_client->handler = etcd_cli;
    return etcd_cli;
}

char* etcd_get(void *handle, char *key) {
    std::string str_key = key;
    EtcdClient *cli = static_cast<EtcdClient *>(handle);
    std::string str_val = cli->get(str_key);
    // char *val = const_cast<char*>(cli->get(str_key));
    char *value = const_cast<char*>(str_val.data());
    // char *val = &*str_val.begin();
    char *val = (char *)malloc(strlen(value) + 1);
    strcpy(val, value);
    return val;
}

int etcd_put(void *handle, char *key, char *value){
    std::string str_key = key;
    std::string str_value = value;
    EtcdClient *cli = static_cast<EtcdClient *>(handle);
    int status = cli->put(str_key, str_value);
    // std::cout << "put status:" << status << std::endl;
    return status;
}

void etcd_watch(void *handle, char *key, void (*user_cb)(char* watch_key, char* val, void *cb_user_data), void *user_data) {
    std::string str_key = key;
    EtcdClient *cli = static_cast<EtcdClient *>(handle);
    cli->watch(str_key, user_cb, user_data);
}

void etcd_watch_prefix(void *handle, char *key, void (*user_cb)(char* watch_key, char* val, void *cb_user_data), void *user_data) {
    std::string str_key = key;
    EtcdClient *cli = static_cast<EtcdClient *>(handle);
    cli->watch_prefix(str_key, user_cb, user_data);
}

void etcd_client_free(void *handle){
    EtcdClient *cli = static_cast<EtcdClient *>(handle);
    cli->~EtcdClient();
}

#ifdef __cplusplus
}
#endif
