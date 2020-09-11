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
 * @brief ConfigMgr C Implementation
 * Holds the implementaion of APIs supported by ConfigMgr
 */


#include "eis/config_manager/pub_cfg.h"
#include <stdarg.h>

#define MAX_CONFIG_KEY_LENGTH 250

// To fetch endpoint from config
config_value_t* cfgmgr_get_endpoint_pub(base_cfg_t* base_cfg) {
    config_value_t* ep = get_endpoint_base(base_cfg);
    if (ep == NULL) {
        LOG_ERROR_0("Endpoint not found");
        return NULL;
    }
    return ep;
}

// To fetch topics from config
config_value_t* cfgmgr_get_topics_pub(base_cfg_t* base_cfg) {
    config_value_t* topics_list = get_topics_base(base_cfg);
    if (topics_list == NULL) {
        LOG_ERROR_0("topics_list initialization failed");
        return NULL;
    }
    return topics_list;
}

// To fetch list of allowed clients from config
config_value_t* cfgmgr_get_allowed_clients_pub(base_cfg_t* base_cfg) {
    config_value_t* client_list = get_allowed_clients_base(base_cfg);
    if (client_list == NULL) {
        LOG_ERROR_0("client_list initialization failed");
        return NULL;
    }
    return client_list;
}

// To set topics in config
int cfgmgr_set_topics_pub(char** topics_list, int len, base_cfg_t* base_cfg) {
    int result = set_topics_base(topics_list, len, PUBLISHERS, base_cfg);
    return result;
}

// To fetch msgbus config
config_t* cfgmgr_get_msgbus_config_pub(base_cfg_t* base_cfg) {

    // Initializing base_cfg variables
    config_value_t* pub_config = base_cfg->msgbus_config;
    char* app_name = base_cfg->app_name;
    int dev_mode = base_cfg->dev_mode;
    kv_store_client_t* m_kv_store_handle = base_cfg->m_kv_store_handle;

    // Creating cJSON object
    cJSON* c_json = cJSON_CreateObject();
    if (c_json == NULL) {
        LOG_ERROR_0("c_json initialization failed");
        return NULL;
    }

    // Fetching Type from config
    config_value_t* publish_json_type = config_value_object_get(pub_config, TYPE);
    if (publish_json_type == NULL) {
        LOG_ERROR_0("publish_json_type initialization failed");
        return NULL;
    }
    char* type = publish_json_type->body.string;
    cJSON_AddStringToObject(c_json, "type", type);

    // Fetching EndPoint from config
    config_value_t* publish_json_endpoint = config_value_object_get(pub_config, ENDPOINT);
    if (publish_json_endpoint == NULL) {
        LOG_ERROR_0("publish_json_endpoint initialization failed");
        return NULL;
    }
    const char* end_point = publish_json_endpoint->body.string;

    // Fetching Name from config
    config_value_t* publish_json_name = config_value_object_get(pub_config, NAME);
    if (publish_json_name == NULL) {
        LOG_ERROR_0("publish_json_name initialization failed");
        return NULL;
    }

    // // Over riding endpoint with PUBLISHER_<Name>_ENDPOINT if set
    size_t init_len = strlen("PUBLISHER_") + strlen(publish_json_name->body.string) + strlen("_ENDPOINT") + 2;
    char* ep_override_env = concat_s(init_len, 3, "PUBLISHER_", publish_json_name->body.string, "_ENDPOINT");
    char* ep_override = getenv(ep_override_env);
    if (ep_override != NULL) {
        if (strlen(ep_override) != 0) {
            LOG_DEBUG("Over riding endpoint with %s", ep_override_env);
            end_point = (const char*)ep_override;
        }
    }

    // Over riding endpoint with PUBLISHER_ENDPOINT if set
    // Note: This overrides all the publisher endpoints if set
    char* publisher_ep = getenv("PUBLISHER_ENDPOINT");
    if (publisher_ep != NULL) {
        LOG_DEBUG_0("Over riding endpoint with PUBLISHER_ENDPOINT");
        if (strlen(publisher_ep) != 0) {
            end_point = (const char*)publisher_ep;
        }
    }

    // Adding zmq_recv_hwm value
    config_value_t* zmq_recv_hwm_value = config_value_object_get(pub_config, ZMQ_RECV_HWM);
    if (zmq_recv_hwm_value != NULL) {
        cJSON_AddNumberToObject(c_json, ZMQ_RECV_HWM, zmq_recv_hwm_value->body.integer);
    }
    if (!strcmp(type, "zmq_ipc")) {
        // Add Endpoint directly to socket_dir if IPC mode
        cJSON_AddStringToObject(c_json, "socket_dir", end_point);
    } else if (!strcmp(type, "zmq_tcp")) {
        // Add host & port to zmq_tcp_publish cJSON object
        char** host_port = get_host_port(end_point);
        char* host = host_port[0];
        trim(host);
        char* port = host_port[1];
        trim(port);
        __int64_t i_port = atoi(port);
        cJSON* zmq_tcp_publish = cJSON_CreateObject();
        if (zmq_tcp_publish == NULL) {
            LOG_ERROR_0("zmq_tcp_publish initialization failed");
            return NULL;
        }
        cJSON_AddStringToObject(zmq_tcp_publish, "host", host);
        cJSON_AddNumberToObject(zmq_tcp_publish, "port", i_port);
        cJSON_AddItemToObject(c_json, "zmq_tcp_publish", zmq_tcp_publish);
        if (dev_mode != 0) {
            void *handle = m_kv_store_handle->init(m_kv_store_handle);

            // Fetching AllowedClients from config
            config_value_t* publish_json_clients = config_value_object_get(pub_config, ALLOWED_CLIENTS);
            if (publish_json_clients == NULL) {
                LOG_ERROR_0("publish_json_clients initialization failed");
                return NULL;
            }
            // Fetch the first item in allowed_clients
            config_value_t* temp_array_value = config_value_array_get(publish_json_clients, 0);
            if (temp_array_value == NULL) {
                LOG_ERROR_0("temp_array_value initialization failed");
                return NULL;
            }
            int result;
            strcmp_s(temp_array_value->body.string, strlen(temp_array_value->body.string), "*", &result);
            // If only one item in allowed_clients and it is *
            // Add all available Publickeys
            if ((config_value_array_len(publish_json_clients) == 1) && (result == 0)) {
                cJSON* all_clients = cJSON_CreateArray();
                if (all_clients == NULL) {
                    LOG_ERROR_0("all_clients initialization failed");
                    return NULL;
                }
                config_value_t* pub_key_values = m_kv_store_handle->get_prefix(handle, "/Publickeys");
                if (pub_key_values == NULL) {
                    LOG_ERROR_0("pub_key_values initialization failed");
                    return NULL;
                }
                config_value_t* value;
                for (int i = 0; i < config_value_array_len(pub_key_values); i++) {
                    value = config_value_array_get(pub_key_values, i);
                    if (value == NULL) {
                        LOG_ERROR_0("value initialization failed");
                        return NULL;
                    }
                    cJSON_AddItemToArray(all_clients, cJSON_CreateString(value->body.string));
                }
                cJSON_AddItemToObject(c_json, "allowed_clients",  all_clients);
            } else {
                config_value_t* array_value;
                cJSON* all_clients = cJSON_CreateArray();
                if (all_clients == NULL) {
                    LOG_ERROR_0("all_clients initialization failed");
                    return NULL;
                }
                for (int i =0; i < config_value_array_len(publish_json_clients); i++) {
                    // Fetching individual public keys of all AllowedClients
                    array_value = config_value_array_get(publish_json_clients, i);
                    if (array_value == NULL) {
                        LOG_ERROR_0("array_value initialization failed");
                        return NULL;
                    }
                    size_t init_len = strlen(PUBLIC_KEYS) + strlen(array_value->body.string) + 2;
                    char* grab_public_key = concat_s(init_len, 2, PUBLIC_KEYS, array_value->body.string);
                    const char* sub_public_key = m_kv_store_handle->get(handle, grab_public_key);
                    if (sub_public_key == NULL) {
                        LOG_ERROR("Value is not found for the key: %s", grab_public_key);
                    }

                    cJSON_AddItemToArray(all_clients, cJSON_CreateString(sub_public_key));
                }
                // Adding all public keys of clients to allowed_clients of config
                cJSON_AddItemToObject(c_json, "allowed_clients",  all_clients);
            }
            // Fetching Publisher private key & adding it to zmq_tcp_publish object
            size_t init_len = strlen("/") + strlen(app_name) + strlen(PRIVATE_KEY) + 2;
            char* pub_pri_key = concat_s(init_len, 3, "/", app_name, PRIVATE_KEY);
            const char* publisher_secret_key = m_kv_store_handle->get(handle, pub_pri_key);
            if (publisher_secret_key == NULL) {
                LOG_ERROR("Value is not found for the key: %s", pub_pri_key);
            }
            cJSON_AddStringToObject(zmq_tcp_publish, "server_secret_key", publisher_secret_key);
        }
    }
    // Constructing char* object from cJSON object
    char* config_value_cr = cJSON_Print(c_json);
    if (config_value_cr == NULL) {
        LOG_ERROR_0("config_value_cr initialization failed");
        return NULL;
    }
    LOG_DEBUG("Env publisher Config is : %s \n", config_value_cr);
    // Constructing config_t object from cJSON object
    config_t* m_config = config_new(
            (void*) c_json, free_json, get_config_value);
    if (m_config == NULL) {
        LOG_ERROR_0("Failed to initialize configuration object");
        return NULL;
    }
    return m_config;
}

// function to initialize pub_cfg_t
pub_cfg_t* pub_cfg_new() {
    LOG_DEBUG_0("In pub_cfg_new mthod");
    pub_cfg_t *pub_cfg_mgr = (pub_cfg_t *)malloc(sizeof(pub_cfg_t));
    if(pub_cfg_mgr == NULL) {
        LOG_ERROR_0("Malloc failed for pub_cfg_t");
        return NULL;
    }
    pub_cfg_mgr->cfgmgr_get_msgbus_config_pub = cfgmgr_get_msgbus_config_pub;
    pub_cfg_mgr->cfgmgr_get_endpoint_pub = cfgmgr_get_endpoint_pub;
    pub_cfg_mgr->cfgmgr_get_topics_pub = cfgmgr_get_topics_pub;
    pub_cfg_mgr->cfgmgr_set_topics_pub = cfgmgr_set_topics_pub;
    pub_cfg_mgr->cfgmgr_get_allowed_clients_pub = cfgmgr_get_allowed_clients_pub;
    return pub_cfg_mgr;
}

// function to destroy cli_cfg_t
void pub_cfg_config_destroy(pub_cfg_t *pub_cfg_config) {
    if(pub_cfg_config != NULL) {
        free(pub_cfg_config);
    }
}