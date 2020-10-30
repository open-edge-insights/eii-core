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
#include "eis/config_manager/util_cfg.h"
#include <stdarg.h>

#define MAX_CONFIG_KEY_LENGTH 250

// To fetch endpoint from config
config_value_t* cfgmgr_get_endpoint_pub(void* pub_conf) {
    pub_cfg_t* pub_cfg = (pub_cfg_t*)pub_conf;
    config_value_t* ep = get_endpoint_base(pub_cfg->pub_config);
    if (ep == NULL) {
        LOG_ERROR_0("Endpoint not found");
        return NULL;
    }
    return ep;
}

config_value_t* cfgmgr_get_interface_value_pub(void* pub_conf, const char* key) {
    pub_cfg_t* pub_cfg = (pub_cfg_t*)pub_conf;
    return config_value_object_get(pub_cfg->pub_config, key);
}

// To fetch topics from config
config_value_t* cfgmgr_get_topics_pub(void* pub_conf) {
    pub_cfg_t* pub_cfg = (pub_cfg_t*)pub_conf;
    config_value_t* topics_list = get_topics_base(pub_cfg->pub_config);
    if (topics_list == NULL) {
        LOG_ERROR_0("topics_list initialization failed");
        return NULL;
    }
    return topics_list;
}

// To fetch list of allowed clients from config
config_value_t* cfgmgr_get_allowed_clients_pub(void* pub_conf) {
    pub_cfg_t* pub_cfg = (pub_cfg_t*)pub_conf;
    config_value_t* client_list = get_allowed_clients_base(pub_cfg->pub_config);
    if (client_list == NULL) {
        LOG_ERROR_0("client_list initialization failed");
        return NULL;
    }
    return client_list;
}

// To set topics in config
int cfgmgr_set_topics_pub(char** topics_list, int len, base_cfg_t* base_cfg, void* pub_conf) {
    pub_cfg_t* pub_cfg = (pub_cfg_t*)pub_conf;
    int result = set_topics_base(topics_list, len, PUBLISHERS, base_cfg, pub_cfg->pub_config);
    return result;
}

// To fetch msgbus config
config_t* cfgmgr_get_msgbus_config_pub(base_cfg_t* base_cfg, void* pub_conf) {

    config_t* m_config = NULL;
    config_value_t* broker_app_name = NULL;
    // Initializing base_cfg variables
    pub_cfg_t* pub_cfg = (pub_cfg_t*) pub_conf;
    config_value_t* pub_config = pub_cfg->pub_config;
    char* app_name = base_cfg->app_name;
    int dev_mode = base_cfg->dev_mode;
    kv_store_client_t* m_kv_store_handle = base_cfg->m_kv_store_handle;
    void* cfgmgr_handle = base_cfg->cfgmgr_handle;

    // Creating cJSON object
    cJSON* c_json = cJSON_CreateObject();
    if (c_json == NULL) {
        LOG_ERROR_0("c_json initialization failed");
        goto err;
    }

    // Fetching Type from config
    config_value_t* publish_config_type = config_value_object_get(pub_config, TYPE);
    if (publish_config_type == NULL) {
        LOG_ERROR_0("publish_config_type initialization failed");
        goto err;
    }
    char* type = publish_config_type->body.string;

    // Fetching EndPoint from config
    config_value_t* publish_config_endpoint = config_value_object_get(pub_config, ENDPOINT);
    if (publish_config_endpoint == NULL) {
        LOG_ERROR_0("publish_config_endpoint initialization failed");
        goto err;
    }

    const char* end_point;
    if(publish_config_endpoint->type == CVT_OBJECT){
        end_point = cvt_to_char(publish_config_endpoint);
    }else{
        end_point = publish_config_endpoint->body.string;
    }

    // Fetching Name from config
    config_value_t* publish_config_name = config_value_object_get(pub_config, NAME);
    if (publish_config_name == NULL) {
        LOG_ERROR_0("publish_config_name initialization failed");
        goto err;
    }

    // Overriding endpoint with PUBLISHER_<Name>_ENDPOINT if set
    size_t init_len = strlen("PUBLISHER_") + strlen(publish_config_name->body.string) + strlen("_ENDPOINT") + 2;
    char* ep_override_env = concat_s(init_len, 3, "PUBLISHER_", publish_config_name->body.string, "_ENDPOINT");
    if (ep_override_env == NULL) {
        LOG_ERROR_0("concatenation for ep_override_env failed");
        goto err;
    }
    char* ep_override = getenv(ep_override_env);
    if (ep_override != NULL) {
        if (strlen(ep_override) != 0) {
            LOG_DEBUG("Overriding endpoint with %s", ep_override_env);
            end_point = (const char*)ep_override;
        }
    }
    free(ep_override_env);

    // Overriding endpoint with PUBLISHER_ENDPOINT if set
    // Note: This overrides all the publisher endpoints if set
    char* publisher_ep = getenv("PUBLISHER_ENDPOINT");
    if (publisher_ep != NULL) {
        LOG_DEBUG_0("Overriding endpoint with PUBLISHER_ENDPOINT");
        if (strlen(publisher_ep) != 0) {
            end_point = (const char*)publisher_ep;
        }
    }

    // Overriding endpoint with PUBLISHER_<Name>_TYPE if set
    init_len = strlen("PUBLISHER_") + strlen(publish_config_name->body.string) + strlen("_TYPE") + 2;
    char* type_override_env = concat_s(init_len, 3, "PUBLISHER_", publish_config_name->body.string, "_TYPE");
    if (type_override_env == NULL) {
        LOG_ERROR_0("concatenation for type_override_env failed");
        goto err;
    }
    char* type_override = getenv(type_override_env);
    if (type_override != NULL) {
        if (strlen(type_override) != 0) {
            LOG_DEBUG("Overriding endpoint with %s", type_override_env);
            type = type_override;
        }
    }

    free(type_override_env);

    // Overriding endpoint with PUBLISHER_TYPE if set
    // Note: This overrides all the publisher type if set
    char* publisher_type = getenv("PUBLISHER_TYPE");
    if (publisher_type != NULL) {
        LOG_DEBUG_0("Overriding endpoint with PUBLISHER_TYPE");
        if (strlen(publisher_type) != 0) {
            type = publisher_type;
        }
    }

    cJSON_AddStringToObject(c_json, "type", type);
    // TODO: type has to be freed in destructor
    // free(type);

    // Adding zmq_recv_hwm value if available
    config_value_t* zmq_recv_hwm_value = config_value_object_get(pub_config, ZMQ_RECV_HWM);
    if (zmq_recv_hwm_value != NULL) {
        if (zmq_recv_hwm_value->type != CVT_INTEGER) {
            LOG_ERROR_0("zmq_recv_hwm type is not integer");
            goto err;
        }
        cJSON_AddNumberToObject(c_json, ZMQ_RECV_HWM, zmq_recv_hwm_value->body.integer);
    }

    // Adding brokered value if available
    config_value_t* brokered_value = config_value_object_get(pub_config, BROKERED);
    if (brokered_value != NULL) {
        if (brokered_value->type != CVT_BOOLEAN) {
            LOG_ERROR_0("brokered_value type is not boolean");
            goto err;
        }
    }

    if (!strcmp(type, "zmq_ipc")) {
        c_json = get_ipc_config(c_json, pub_config, end_point);
        if (c_json == NULL){
            LOG_ERROR_0("IPC configuration for publisher failed");
            goto err;
        }
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
            goto err;
        }
        cJSON_AddStringToObject(zmq_tcp_publish, "host", host);
        cJSON_AddNumberToObject(zmq_tcp_publish, "port", i_port);
        if (brokered_value != NULL) {
            if (brokered_value->body.boolean) {
                cJSON_AddBoolToObject(zmq_tcp_publish, BROKERED, true);
            } else {
                cJSON_AddBoolToObject(zmq_tcp_publish, BROKERED, false);
            }
        }
        cJSON_AddItemToObject(c_json, "zmq_tcp_publish", zmq_tcp_publish);
        if (dev_mode != 0) {
            bool ret_val;

            // Checking if Publisher is using Broker to publish or not and constructing
            // the publisher messagebus config based on the check. 
            broker_app_name = config_value_object_get(pub_config, BROKER_APPNAME);
            if(broker_app_name != NULL){

                if(broker_app_name->type != CVT_STRING){
                    LOG_ERROR_0("[Type Missmatch]: BrokerAppName should be of type String");
                    goto err;
                }
                // If a publisher is using broker to communicate with its respective subscriber
                // then publisher will act as a subscriber to X-SUB, hence publishers 
                // message bus config looks like a subscriber one.  
                ret_val = add_keys_to_config(zmq_tcp_publish, app_name, m_kv_store_handle, cfgmgr_handle, broker_app_name, pub_config);
                if(!ret_val) {
                    LOG_ERROR_0("Failed to add respective cert keys");
                    goto err;
                }
            } else{
                ret_val = construct_tcp_publisher_prod(app_name, c_json, zmq_tcp_publish, cfgmgr_handle, pub_config, m_kv_store_handle);
                if(!ret_val) {
                    LOG_ERROR_0("Failed to construct tcp config struct");
                    goto err;
                }
            }
        }
    }
    // Constructing char* object from cJSON object
    char* config_value_cr = cJSON_Print(c_json);
    if (config_value_cr == NULL) {
        LOG_ERROR_0("config_value_cr initialization failed");
        goto err;
    }
    LOG_DEBUG("Env publisher Config is : %s \n", config_value_cr);
    // Constructing config_t object from cJSON object
    m_config = config_new(
            (void*) c_json, free_json, get_config_value);
    if (m_config == NULL) {
        LOG_ERROR_0("Failed to initialize configuration object");
        goto err;
    }

err:
    if (publish_config_type != NULL){
        config_value_destroy(publish_config_type);
    }
    if (publish_config_name != NULL){
        config_value_destroy(publish_config_name);
    }
    if (publish_config_endpoint != NULL){
        config_value_destroy(publish_config_endpoint);
    }
    if (broker_app_name != NULL){
        config_value_destroy(broker_app_name);
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
    pub_cfg_mgr->cfgmgr_get_interface_value_pub = cfgmgr_get_interface_value_pub;
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