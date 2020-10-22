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
#define ETCD_HOST_IP    "127.0.0.1"
#define ETCD_PORT       "2379"


void* etcd_init(void* etcd_client);
char* etcd_get(void * handle, char *key);
config_value_t* etcd_get_prefix(void * handle, char *key);
int etcd_put(void* handle, char *key, char *value);
void etcd_watch(void* handle, char *key_test, callback_t cb, void* user_data);
void etcd_watch_prefix(void* handle, char *key_test, callback_t cb, void* user_data);
void etcd_client_free(void* handle);


kv_store_client_t* create_etcd_client(config_t *config) {
    kv_store_client_t *kv_store_client = NULL, *ret = NULL;
    etcd_config_t *etcd_config = NULL;
    config_value_t *cert_file, *key_file, *ca_file;
    char *host = NULL, *port = NULL;
    char *etcd_host = NULL, *etcd_port = NULL, *src_etcd_host = NULL, *src_etcd_port = NULL;
    
    cert_file = key_file = ca_file = NULL;

    etcd_config = (etcd_config_t*)malloc(sizeof(etcd_config_t));
    kv_store_client = (kv_store_client_t*)malloc(sizeof(kv_store_client_t));

    if (etcd_config == NULL && kv_store_client == NULL) {
        LOG_ERROR_0("KV Store: Failed to allocate Memory while creating etcd client");
        goto err;
    }

    config_value_t* conf_obj = config->get_config_value(
                config->cfg, ETCD_KV_STORE);

    if (conf_obj == NULL) {
        LOG_ERROR("Config missing key '%s'", ETCD_KV_STORE);
        goto err;

    } else if (conf_obj->type != CVT_OBJECT) {
        LOG_ERROR("Configuration for '%s' must be an object",
                    ETCD_KV_STORE);
        config_value_destroy(conf_obj);
        goto err;
    } else {
        // Fetching ETCD_HOST type from env
        size_t host_len = strlen(ETCD_HOST_IP);
        src_etcd_host = getenv("ETCD_HOST");
        if ((src_etcd_host == NULL) || (strlen((src_etcd_host)) == 0)) {
            LOG_DEBUG_0("ETCD_HOST env not set or set to empty, defaulting to 127.0.0.1");
            etcd_host = (char*)calloc((host_len + 1), sizeof(char));
            if (etcd_host == NULL) {
                LOG_ERROR("Failed to calloc %s", etcd_host);
                goto err;
            }
            int ret_cpy = strncpy_s(etcd_host, host_len + 1, ETCD_HOST_IP, host_len);
            if (ret_cpy != 0) {
                LOG_ERROR("Failed to copy %s", etcd_host);
                goto err;
            }
        } else {
            LOG_DEBUG("ETCD_HOST env is set to %s", src_etcd_host);
            size_t src_len = strlen(src_etcd_host);
            etcd_host = (char*)calloc((src_len + 1), sizeof(char));
            if (etcd_host == NULL) {
                LOG_ERROR("Failed to calloc %s", etcd_host);
                goto err;
            }
            int ret_cpy = strncpy_s(etcd_host, src_len + 1, src_etcd_host, src_len);
            if (ret_cpy != 0) {
                LOG_ERROR("Failed to copy %s", etcd_host);
                goto err;
            }
        }
        // Fetching ETCD_CLIENT_PORT type from env
        src_etcd_port = getenv("ETCD_CLIENT_PORT");
        size_t port_len = strlen(ETCD_PORT);
        if ((src_etcd_port == NULL) || (strlen(src_etcd_port) == 0)) {
            LOG_DEBUG_0("ETCD_CLIENT_PORT env not set or a empty string, defaulting to 2379");
            etcd_port = (char*)calloc((port_len + 1), sizeof(char));
            if (etcd_port == NULL) {
                LOG_ERROR("Failed to calloc %s", etcd_port);
                goto err;
            }
            int ret_cpy = strncpy_s(etcd_port, port_len + 1, ETCD_PORT, port_len);
            if (ret_cpy != 0) {
                LOG_ERROR("Failed to copy %s", etcd_port);
                goto err;
            }
        } else {
            LOG_DEBUG_0("ETCD_PORT env is set to some value, using the same");
            size_t src_len = strlen(src_etcd_port);
            etcd_port = (char*)calloc((src_len + 1), sizeof(char));
            if (etcd_port == NULL) {
                LOG_ERROR("Failed to calloc %s", etcd_port);
                goto err;
            }
            int ret_cpy = strncpy_s(etcd_port, port_len + 1, src_etcd_port, src_len);
            if (ret_cpy != 0) {
                LOG_ERROR("Failed to copy %s", etcd_port);
                goto err;
            }
        }
        // Fetching ETCD_ENDPOINT from env for CSL
        // If set over-rides ETCD_HOST & ETCD_CLIENT_PORT
        char* etcd_endpoint = getenv("ETCD_ENDPOINT");
        if (etcd_endpoint == NULL) {
            LOG_DEBUG_0("ETCD_ENDPOINT env not set, using ETCD_HOST & ETCD_CLIENT_PORT");
        } else {
            if (strlen(etcd_endpoint) == 0) {
                LOG_DEBUG_0("ETCD_ENDPOINT is empty, using ETCD_HOST & ETCD_CLIENT_PORT");
            } else {
                //Need to free etcd_host & etcd_port aloocated previously.
                free(etcd_host);
                free(etcd_port);
                size_t str_len = strlen(etcd_endpoint) + 1;
                char* c_etcd_endpoint = (char*)malloc(sizeof(char) * str_len);
                snprintf(c_etcd_endpoint, str_len, "%s", etcd_endpoint);
                LOG_DEBUG("ETCD endpoint: %s", c_etcd_endpoint);
                char** host_port = get_host_port(c_etcd_endpoint);
                host = host_port[0];
                trim(host);
                port = host_port[1];
                trim(port);
                etcd_host = host;
                etcd_port = port;
                free(c_etcd_endpoint);
            }
        }
        LOG_DEBUG("Obtained ETCD IP %s & port %s", etcd_host, etcd_port);

        cert_file = config->get_config_value(conf_obj->body.object->object, CERT_FILE);
        if (cert_file == NULL) {
            LOG_ERROR("Configuration for '%s' missing '%s'",
                        ETCD_KV_STORE, CERT_FILE);
            goto err;
        } else if (cert_file->type != CVT_STRING) {
            LOG_ERROR_0("CERT_FILE must be string");
            goto err;
        }

        key_file = config->get_config_value(conf_obj->body.object->object, KEY_FILE);
        if (key_file == NULL) {
            LOG_ERROR("Configuration for '%s' missing '%s'",
                        ETCD_KV_STORE, KEY_FILE);
            goto err;
        } else if (key_file->type != CVT_STRING) {
            LOG_ERROR_0("KEY_FILE must be string");
            goto err;
        }

        ca_file = config->get_config_value(conf_obj->body.object->object, CA_FILE);
        if (ca_file == NULL) {
            LOG_ERROR("Configuration for '%s' missing '%s'",
                        ETCD_KV_STORE, CA_FILE);
            goto err;
        } else if (ca_file->type != CVT_STRING) {
            LOG_ERROR_0("CA_FILE must be string");
            goto err;
        }

        if (conf_obj != NULL) {
            config_value_destroy(conf_obj);
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
        kv_store_client->deinit = etcd_values_destroy;
        ret = kv_store_client;
    }
    return ret;

err:
    if (etcd_config != NULL) {
        config_value_destroy(etcd_config);
    }
    if (kv_store_client != NULL) {
        config_value_destroy(kv_store_client);
    }
    if (host != NULL) {
        free(host);
    }
    if (port != NULL) {
        free(port);
    }
    if (conf_obj != NULL) {
        config_value_destroy(conf_obj);
    }
    if (cert_file != NULL) {
        config_value_destroy(cert_file);
    }
    if (key_file != NULL) {
        config_value_destroy(key_file);
    }
    if (ca_file != NULL) {
        config_value_destroy(ca_file);
    }
    return ret;
}

void etcd_values_destroy(kv_store_client_t* kv_store_client) {
    LOG_DEBUG_0("etcd_values_destroy function...");
    etcd_config_t* etcd_config = (etcd_config_t*)(kv_store_client->kv_store_config);
    if (etcd_config->hostname != NULL) {
        free(etcd_config->hostname);
    }
    if (etcd_config->port != NULL) {
        free(etcd_config->port);
    }
    if (etcd_config->cert_file != NULL) {
        config_value_destroy(etcd_config->cert_file);
    }
    if (etcd_config->key_file != NULL) {
        config_value_destroy(etcd_config->key_file);
    }
    if (etcd_config->ca_file != NULL) {
        config_value_destroy(etcd_config->ca_file);
    }
    if (kv_store_client->handler != NULL) {
        etcd_client_free(kv_store_client->handler);
    }
    LOG_DEBUG_0("freed Elements in etcd_values_destroy function...");
}
