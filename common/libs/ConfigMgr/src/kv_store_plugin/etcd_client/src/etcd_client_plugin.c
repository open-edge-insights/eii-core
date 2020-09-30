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

#include <eis/utils/config.h>
#include <eis/utils/string.h>
#include <stdlib.h>

#include <eis/config_manager/kv_store_plugin.h>
#include <eis/config_manager/etcd_client_plugin.h>

#define PORT            "port"
#define HOST            "host"
#define CERT_FILE       "cert_file"
#define KEY_FILE        "key_file"
#define CA_FILE         "ca_file"

typedef void (*callback_t)(char *key, char *value, void* cb_user_data);

void* etcd_init(void* etcd_client);
char* etcd_get(void * handle, char *key);
config_value_t* etcd_get_prefix(void * handle, char *key);
int etcd_put(void* handle, char *key, char *value);
void etcd_watch(void* handle, char *key_test, callback_t cb, void* user_data);
void etcd_watch_prefix(void* handle, char *key_test, callback_t cb, void* user_data);
void etcd_client_free(void* handle);

kv_store_client_t* create_etcd_client(config_t *config) {
    kv_store_client_t *kv_store_client = NULL;
    etcd_config_t *etcd_config = NULL;

    etcd_config = (etcd_config_t*)malloc(sizeof(etcd_config_t));
    kv_store_client = (kv_store_client_t*)malloc(sizeof(kv_store_client_t));

    if (etcd_config == NULL || kv_store_client == NULL) {
        LOG_ERROR_0("KV Store: Out of memory initializing");
        return NULL;
    }

    config_value_t* conf_obj = config->get_config_value(
                config->cfg, ETCD_KV_STORE);

    if(conf_obj == NULL) {
        LOG_ERROR("Config missing key '%s'", ETCD_KV_STORE);
        return NULL;
    } else if(conf_obj->type != CVT_OBJECT) {
        LOG_ERROR("Configuration for '%s' must be an object",
                    ETCD_KV_STORE);
        config_value_destroy(conf_obj);
        goto err;
    } else {

        // Fetching ETCD_HOST type from env
        char* etcd_host = getenv("ETCD_HOST");
        if (etcd_host == NULL) {
            LOG_DEBUG_0("ETCD_HOST env not set, defaulting to 127.0.0.1");
            etcd_host = "127.0.0.1";
        } else {
            if (strlen(etcd_host) == 0) {
                LOG_DEBUG_0("ETCD_HOST env is set to empty, defaulting to 127.0.0.1");
                etcd_host = "127.0.0.1";
            }
        }

        // Fetching ETCD_PORT type from env
        char* etcd_port = getenv("ETCD_PORT");
        if (etcd_port == NULL) {
            LOG_DEBUG_0("ETCD_PORT env not set, defaulting to 2379");
            etcd_port = "2379";
        } else {
            if (strlen(etcd_port) == 0) {
                LOG_DEBUG_0("ETCD_PORT env is set to empty, defaulting to 2379");
                etcd_port = "2379";
            }
        }

        // Fetching ETCD_ENDPOINT from env for CSL
        // If set over-rides ETCD_HOST & ETCD_PORT
        char* etcd_endpoint = getenv("ETCD_ENDPOINT");
        if (etcd_endpoint == NULL) {
            LOG_DEBUG_0("ETCD_ENDPOINT env not set, using ETCD_HOST & ETCD_PORT");
        } else {
            if (strlen(etcd_endpoint) == 0) {
                LOG_DEBUG_0("ETCD_ENDPOINT is empty, using ETCD_HOST & ETCD_PORT");
            } else {
                size_t str_len = strlen(etcd_endpoint) + 1;
                char* c_etcd_endpoint = (char*)malloc(sizeof(char) * str_len);
                snprintf(c_etcd_endpoint, str_len, "%s", etcd_endpoint);
                LOG_DEBUG("ETCD endpoint: %s", c_etcd_endpoint);
                char** host_port = get_host_port(c_etcd_endpoint);
                char* host = host_port[0];
                trim(host);
                char* port = host_port[1];
                trim(port);
                etcd_host = host;
                etcd_port = port;
            }
        }

        config_value_t* cert_file = config->get_config_value(conf_obj->body.object->object, CERT_FILE);
        if(cert_file == NULL) {
            LOG_ERROR("Configuration for '%s' missing '%s'",
                        ETCD_KV_STORE, CERT_FILE);
            config_value_destroy(cert_file);
            config_value_destroy(conf_obj);
            goto err;
        } else if(cert_file->type != CVT_STRING) {
            LOG_ERROR_0("CERT_FILE must be string");
            config_value_destroy(cert_file);
            config_value_destroy(conf_obj);
            goto err;
        }

        config_value_t* key_file = config->get_config_value(conf_obj->body.object->object, KEY_FILE);
        if(key_file == NULL) {
            LOG_ERROR("Configuration for '%s' missing '%s'",
                        ETCD_KV_STORE, KEY_FILE);
            config_value_destroy(key_file);
            config_value_destroy(conf_obj);
            goto err;
        } else if(key_file->type != CVT_STRING) {
            LOG_ERROR_0("KEY_FILE must be string");
            config_value_destroy(key_file);
            config_value_destroy(conf_obj);
            goto err;
        }

        config_value_t* ca_file = config->get_config_value(conf_obj->body.object->object, CA_FILE);
        if(ca_file == NULL) {
            LOG_ERROR("Configuration for '%s' missing '%s'",
                        ETCD_KV_STORE, CA_FILE);
            config_value_destroy(ca_file);
            config_value_destroy(conf_obj);
            goto err;
        } else if(ca_file->type != CVT_STRING) {
            LOG_ERROR_0("CA_FILE must be string");
            config_value_destroy(ca_file);
            config_value_destroy(conf_obj);
            goto err;
        }

        etcd_config->hostname = etcd_host;
        etcd_config->port = etcd_port;
        etcd_config->cert_file = cert_file->body.string;
        etcd_config->key_file = key_file->body.string;
        etcd_config->ca_file = ca_file->body.string;

        kv_store_client->kv_store_config = etcd_config;
        kv_store_client->get = etcd_get;
        kv_store_client->get_prefix = etcd_get_prefix;
        kv_store_client->put = etcd_put;
        kv_store_client->watch = etcd_watch;
        kv_store_client->watch_prefix = etcd_watch_prefix;
        kv_store_client->init = etcd_init;
        kv_store_client->deinit = etcd_client_free;
        return kv_store_client;
    }
err:
    return NULL;
}
