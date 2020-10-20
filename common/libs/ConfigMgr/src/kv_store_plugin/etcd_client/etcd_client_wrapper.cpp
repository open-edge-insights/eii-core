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
#include <stdlib.h>

#include <safe_lib.h>
#include <eis/config_manager/kv_store_plugin.h>
#include <eis/config_manager/etcd_client_plugin.h>
#include <eis/config_manager/etcd_client.h>
#include <eis/utils/config.h>
#include <cjson/cJSON.h>
#include <eis/utils/json_config.h>

#ifdef __cplusplus
extern "C" {
#endif

void* etcd_init(void* etcd_client) {
    EtcdClient *etcd_cli = NULL;
    kv_store_client_t *kv_store_client = static_cast<kv_store_client_t *>(etcd_client);
    etcd_config_t *etcd_config = static_cast<etcd_config_t *>(kv_store_client->kv_store_config);
    std::string host = etcd_config->hostname;
    std::string port = etcd_config->port;
    int cmp_cert_file, cmp_key_file, cmp_ca_file;

    strcmp_s(etcd_config->cert_file, strlen(etcd_config->cert_file), "", &cmp_cert_file);
    strcmp_s(etcd_config->key_file, strlen(etcd_config->key_file), "", &cmp_key_file);
    strcmp_s(etcd_config->ca_file, strlen(etcd_config->ca_file), "", &cmp_ca_file);

    try {
        if(cmp_cert_file != 0 && cmp_key_file != 0 && cmp_ca_file != 0)
            etcd_cli = new EtcdClient(host, port,  etcd_config->cert_file, etcd_config->key_file, etcd_config->ca_file);
        else
            etcd_cli = new EtcdClient(host, port);
        kv_store_client->handler = etcd_cli;
    }catch(std::exception const & ex) {
            LOG_ERROR("Exception Occurred in etcd_init with error:%s", ex.what());
            return NULL;
    }
    return etcd_cli;
}

char* etcd_get(void* handle, char *key) {
    std::string str_key = key;
    EtcdClient *cli = static_cast<EtcdClient *>(handle);
    std::string str_val = cli->get(str_key);
    char *value = const_cast<char*>(str_val.data());
    int cmp_value;

    strcmp_s(value, strlen(value), "(NULL)", &cmp_value);
    if (cmp_value == 0)
        return NULL;

    size_t len = strlen(value) + 1;

    // TODO: Find a way to dealloc allocated mem here
    char *val = (char *)malloc(len);
    memset(val, '\0', len);
    strcpy_s(val, len, value);
    return val;
}

config_value_t* etcd_get_prefix(void* handle, char *key) {
    std::string str_key = key;
    config_value_t* values;
    EtcdClient *cli = static_cast<EtcdClient *>(handle);
    std::vector<std::string> vec = cli->get_prefix(str_key);

    if(!vec.size()){
        LOG_ERROR("Key not found %s",key);
        return NULL;
    }

    cJSON* all_values = cJSON_CreateArray();

    for(size_t i = 0; i < vec.size(); i++){
        cJSON_AddItemToArray(all_values, cJSON_CreateString(vec[i].c_str()));
    }

    values = config_value_new_array(
                (void*) all_values , cJSON_GetArraySize(all_values), get_array_item, NULL);

    return values;
}

int etcd_put(void* handle, char *key, char *value){
    std::string str_key = key;
    std::string str_value = value;
    EtcdClient *cli = static_cast<EtcdClient *>(handle);
    int status = cli->put(str_key, str_value);
    return status;
}

void etcd_watch(void* handle, char *key, callback_t user_cb, void* user_data) {
    std::string str_key = key;
    EtcdClient *cli = static_cast<EtcdClient *>(handle);
    cli->watch(str_key, user_cb, user_data);
}

void etcd_watch_prefix(void* handle, char *key, callback_t user_cb, void* user_data) {
    std::string str_key = key;
    EtcdClient *cli = static_cast<EtcdClient *>(handle);
    cli->watch_prefix(str_key, user_cb, user_data);
}

void etcd_client_free(void* handle){
    EtcdClient *cli = static_cast<EtcdClient *>(handle);
    cli->~EtcdClient();
}

#ifdef __cplusplus
}
#endif
