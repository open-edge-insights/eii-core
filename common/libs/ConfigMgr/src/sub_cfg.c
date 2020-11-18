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


#include "eis/config_manager/sub_cfg.h"
#include "eis/config_manager/util_cfg.h"
#include <stdarg.h>

#define MAX_CONFIG_KEY_LENGTH 250

// To fetch endpoint from config
config_value_t* cfgmgr_get_endpoint_sub(void* sub_conf) {
    sub_cfg_t* sub_cfg = (sub_cfg_t*)sub_conf;
    config_value_t* ep = get_endpoint_base(sub_cfg->sub_config);
    if (ep == NULL) {
        LOG_ERROR_0("Endpoint not found");
        return NULL;
    }
    return ep;
}

// To fetch topics from config
config_value_t* cfgmgr_get_topics_sub(void* sub_conf) {
    sub_cfg_t* sub_cfg = (sub_cfg_t*)sub_conf;
    config_value_t* topics_list = get_topics_base(sub_cfg->sub_config);
    if (topics_list == NULL) {
        LOG_ERROR_0("topics_list initialization failed");
        return NULL;
    }
    return topics_list;
}

config_value_t* cfgmgr_get_interface_value_sub(void* sub_conf, const char* key) {
    sub_cfg_t* sub_cfg = (sub_cfg_t*)sub_conf;
    config_value_t* interface_value = config_value_object_get(sub_cfg->sub_config, key);
    if (interface_value == NULL){
        LOG_DEBUG_0("interface_value initialization failed");
        return NULL;
    }
    return interface_value;
}

// To set topics in config
int cfgmgr_set_topics_sub(char** topics_list, int len, base_cfg_t* base_cfg, void* sub_conf) {
    sub_cfg_t* sub_cfg = (sub_cfg_t*) sub_conf;
    int result = set_topics_base(topics_list, len, SUBSCRIBERS, base_cfg, sub_cfg->sub_config);
    if(result != 0){
        LOG_ERROR_0("Error in setting new topics")
    }
    return result;
}

// To fetch msgbus config
config_t* cfgmgr_get_msgbus_config_sub(base_cfg_t* base_cfg, void* sub_conf) {
    char** host_port = NULL;
    char* host = NULL;
    char* port = NULL;
    sub_cfg_t* sub_cfg = (sub_cfg_t*)sub_conf;
    // Initializing base_cfg variables
    config_value_t* sub_config = sub_cfg->sub_config;
    char* app_name = base_cfg->app_name;
    bool dev_mode = false;
    config_t* m_config = NULL;
    config_value_t* topic_array = NULL;
    config_value_t* publisher_appname = NULL;
    config_value_t* topic = NULL;
    char* config_value_cr = NULL;
    config_value_t* subscribe_config_name = NULL;
    config_value_t* subscribe_config_endpoint = NULL;
    char* type_override_env = NULL;
    char* type_override = NULL;
    config_value_t* subscribe_config_type = NULL;
    char* ep_override_env = NULL;
    char* ep_override = NULL;
    config_value_t* zmq_recv_hwm_value = NULL;

    int devmode = base_cfg->dev_mode;
    if(devmode == 0) {
        dev_mode = true;
    }
 
    kv_store_client_t* kv_store_handle = base_cfg->m_kv_store_handle;
    void* cfgmgr_handle = base_cfg->cfgmgr_handle;
    // Creating cJSON object
    cJSON* c_json = cJSON_CreateObject();
    if (c_json == NULL) {
        LOG_ERROR_0("c_json initialization failed");
        goto err;
    }

    // Fetching Type from config
    subscribe_config_type = config_value_object_get(sub_config, TYPE);
    if (subscribe_config_type == NULL || subscribe_config_type->body.string == NULL) {
        LOG_ERROR_0("subscribe_config_type initialization failed");
        goto err;
    }

    if(subscribe_config_type->type != CVT_STRING || subscribe_config_type->body.string == NULL){
        LOG_ERROR_0("subscribe_config_type type mismatch or the string value is NULL");
        goto err;
    }
    char* type = subscribe_config_type->body.string;

    // Fetching EndPoint from config
    subscribe_config_endpoint = config_value_object_get(sub_config, ENDPOINT);
    if (subscribe_config_endpoint == NULL) {
        LOG_ERROR_0("subscribe_config_endpoint initialization failed");
        goto err;
    }

    const char* end_point = NULL;
    if(subscribe_config_endpoint->type == CVT_OBJECT){
        end_point = cvt_to_char(subscribe_config_endpoint);
    }else{
        end_point = subscribe_config_endpoint->body.string;
    }

    if(end_point == NULL){
        LOG_ERROR_0("Failed to get endpoint, its NULL");
        goto err;
    }

    // Fetching Name from config
    subscribe_config_name = config_value_object_get(sub_config, NAME);
    if (subscribe_config_name == NULL) {
        LOG_ERROR_0("subscribe_config_name initialization failed");
        goto err;
    }

    // Overriding endpoint with SUBSCRIBER_<Name>_ENDPOINT if set
    size_t init_len = strlen("SUBSCRIBER_") + strlen(subscribe_config_name->body.string) + strlen("_ENDPOINT") + 2;
    ep_override_env = concat_s(init_len, 3, "SUBSCRIBER_", subscribe_config_name->body.string, "_ENDPOINT");
    if (ep_override_env == NULL) {
        LOG_ERROR_0("concatenation for ep_override_env failed");
        goto err;
    }
    ep_override = getenv(ep_override_env);   
    if (ep_override != NULL) {
        if (strlen(ep_override) != 0) {
            LOG_DEBUG("Overriding endpoint with %s", ep_override_env);
            end_point = (const char*)ep_override;
        }
    } else {
        LOG_DEBUG("env not set for overridding %s, and hence endpoint taking from interface ", ep_override_env);
    }

    // Overriding endpoint with SUBSCRIBER_ENDPOINT if set
    // Note: This overrides all the subscriber type if set
    char* subscriber_ep = getenv("SUBSCRIBER_ENDPOINT");
    if (subscriber_ep != NULL) {
        LOG_DEBUG_0("Overriding endpoint with SUBSCRIBER_ENDPOINT");
        if (strlen(subscriber_ep) != 0) {
            end_point = (const char*)subscriber_ep;
        }
    } else {
        LOG_DEBUG_0("env not set for overridding SUBSCRIBER_ENDPOINT, and hence endpoint taking from interface ");
    }

    // Overriding endpoint with SUBSCRIBER_<Name>_TYPE if set
    init_len = strlen("SUBSCRIBER_") + strlen(subscribe_config_name->body.string) + strlen("_TYPE") + 2;
    type_override_env = concat_s(init_len, 3, "SUBSCRIBER_", subscribe_config_name->body.string, "_TYPE");
    if (type_override_env == NULL) {
        LOG_ERROR_0("concatenation for type_override_env failed");
        goto err;
    }
    type_override = getenv(type_override_env);
    if (type_override != NULL) {
        if (strlen(type_override) != 0) {
            LOG_DEBUG("Overriding endpoint with %s", type_override_env);
            type = type_override;
        }
    } else {
        LOG_DEBUG("env not set for overridding %s, and hence type taking from interface ", type_override_env);
    }

    // Overriding endpoint with SUBSCRIBER_TYPE if set
    // Note: This overrides all the subscriber endpoints if set
    char* subscriber_type = getenv("SUBSCRIBER_TYPE");
    if (subscriber_type != NULL) {
        LOG_DEBUG_0("Overriding endpoint with SUBSCRIBER_TYPE");
        if (strlen(subscriber_type) != 0) {
            type = subscriber_type;
        }
    } else {
        LOG_DEBUG("env not set for overridding SUBSCRIBER_TYPE, and hence type taking from interface ");
    }
    cJSON_AddStringToObject(c_json, "type", type);

    // Adding zmq_recv_hwm value if available
    zmq_recv_hwm_value = config_value_object_get(sub_config, ZMQ_RECV_HWM);
    if (zmq_recv_hwm_value != NULL) {
        if (zmq_recv_hwm_value->type != CVT_INTEGER) {
            LOG_ERROR_0("zmq_recv_hwm type is not integer");
            goto err;
        }
        cJSON_AddNumberToObject(c_json, ZMQ_RECV_HWM, zmq_recv_hwm_value->body.integer);
    }

    if(!strcmp(type, "zmq_ipc")) {
        bool ret = get_ipc_config(c_json, sub_config, end_point);
        if (ret == false) {
            LOG_ERROR_0("IPC configuration for subscriber failed");
            goto err;
        }
    } else if(!strcmp(type, "zmq_tcp")) {

        // Fetching Topics from config
        topic_array = config_value_object_get(sub_config, TOPICS);
        if (topic_array == NULL) {
            LOG_ERROR_0("topic_array initialization failed");
            goto err;
        }

        size_t arr_len = config_value_array_len(topic_array);
        if(arr_len == 0){
            LOG_ERROR_0("Empty array is not supported, atleast one value should be given.");
            goto err;
        }

        host_port = get_host_port(end_point);
        if (host_port == NULL){
            LOG_ERROR_0("Get host and port failed");
            goto err;
        }
        host = host_port[0];
        trim(host);
        port = host_port[1];
        trim(port);
        int64_t i_port = atoi(port);
        
        int ret;
        int topicret;
        // comparing the first topic in the array of subscribers topic with "*"
        topic = config_value_array_get(topic_array, 0);
        if (topic == NULL || topic->body.string == NULL){
            LOG_ERROR_0("topic initialization failed");
            goto err;
        }
        strcmp_s(topic->body.string, strlen(topic->body.string), "*", &topicret);
        if (topic != NULL) {
            config_value_destroy(topic);
        }

        publisher_appname = config_value_object_get(sub_config, PUBLISHER_APPNAME);
        if (publisher_appname == NULL) {
            LOG_ERROR("%s initialization failed", PUBLISHER_APPNAME);
            goto err;
        }

        if(publisher_appname->type != CVT_STRING || publisher_appname->body.string == NULL){
            LOG_ERROR("PublisherAppName type mismatch or the string is NULL");
            goto err;
        }

        for (int i = 0; i < arr_len; i++) {
            // Create cJSON object for every topic
            cJSON* topics = cJSON_CreateObject();
            if (topics == NULL) {
                LOG_ERROR_0("c_json initialization failed");
                goto err;
            }
            topic = config_value_array_get(topic_array, i);
            if (topic == NULL){
                LOG_ERROR_0("topic initialization failed");
                goto err;
            }
            // Add host & port to cJSON object
            cJSON_AddStringToObject(topics, "host", host);
            cJSON_AddNumberToObject(topics, "port", i_port);

            // if topics lenght is not 1 or the topic is not equal to "*",
            //then we are adding that topic for subscription.
            if(!dev_mode) {
                bool ret_val;
                // This is EISZmqBroker usecase, where in "PublisherAppname" will be specified as "*"
                // hence comparing for "PublisherAppname" and "*"
                strcmp_s(publisher_appname->body.string, strlen(publisher_appname->body.string), "*", &ret);
                if(ret == 0) {
                    // In case of EISZmqBroker, it is "X-SUB" which needs "publishers" way of
                    // messagebus config, hence calling "construct_tcp_publisher_prod()" function
                    ret_val = construct_tcp_publisher_prod(app_name, c_json, topics, cfgmgr_handle, sub_config, kv_store_handle);
                     if(!ret_val) {
                        LOG_ERROR_0("Failed in construct_tcp_publisher_prod()");
                        goto err;
                    }
                }else {
                    ret_val = add_keys_to_config(topics, app_name, kv_store_handle, cfgmgr_handle, publisher_appname, sub_config);
                    if(!ret_val) {
                        LOG_ERROR_0("Failed in add_keys_to_config()");
                        goto err;
                    }
                }
            }
            if((arr_len == 1) && (topicret == 0)) {
                cJSON_AddItemToObject(c_json, "", topics);
            } else {
                cJSON_AddItemToObject(c_json, topic->body.string, topics);
            }
            if (topic != NULL) {
                config_value_destroy(topic);
            }
        }
    } else {
        LOG_ERROR_0("Type should be either \"zmq_ipc\" or \"zmq_tcp\"");
        goto err;
    }

    // Constructing char* object from cJSON object
    config_value_cr = cJSON_Print(c_json);
    if (config_value_cr == NULL) {
        LOG_ERROR_0("c_json initialization failed");
        goto err;
    }
    LOG_DEBUG("Env subscriber Config is : %s \n", config_value_cr);

    // Constructing config_t object from cJSON object
    m_config = config_new(
            (void*) c_json, free_json, get_config_value);
    if (m_config == NULL) {
        LOG_ERROR_0("Failed to initialize configuration object");
        goto err;
    }
err:
    if (host_port != NULL) {
        free_mem(host_port);
    }
    if (subscribe_config_type != NULL) {
        config_value_destroy(subscribe_config_type);
    }
    if (subscribe_config_endpoint != NULL) {
        config_value_destroy(subscribe_config_endpoint);
    }
    if (subscribe_config_name != NULL) {
        config_value_destroy(subscribe_config_name);
    }
    if (ep_override_env != NULL) {
        free(ep_override_env);
    }
    if (type_override_env != NULL) {
        free(type_override_env);
    }
    if (zmq_recv_hwm_value != NULL) {
        config_value_destroy(zmq_recv_hwm_value);
    }
    if (topic_array != NULL) {
        config_value_destroy(topic_array);
    }
    if (publisher_appname != NULL) {
        config_value_destroy(publisher_appname);
    }
    if (config_value_cr != NULL) {
        free(config_value_cr);
    }
    return m_config;
}

// function to initialize sub_cfg_t
sub_cfg_t* sub_cfg_new() {
    LOG_DEBUG_0("In server_cfg_new mthod");
    sub_cfg_t *sub_cfg_mgr = (sub_cfg_t *)malloc(sizeof(sub_cfg_t));
    if(sub_cfg_mgr == NULL) {
        LOG_ERROR_0("Malloc failed for sub_cfg_t");
        return NULL;
    }
    sub_cfg_mgr->cfgmgr_get_msgbus_config_sub = cfgmgr_get_msgbus_config_sub;
    sub_cfg_mgr->cfgmgr_get_endpoint_sub = cfgmgr_get_endpoint_sub;
    sub_cfg_mgr->cfgmgr_get_interface_value_sub = cfgmgr_get_interface_value_sub;
    sub_cfg_mgr->cfgmgr_get_topics_sub = cfgmgr_get_topics_sub;
    sub_cfg_mgr->cfgmgr_set_topics_sub = cfgmgr_set_topics_sub;
    return sub_cfg_mgr;
}

// function to destroy sub_cfg_t
void sub_cfg_config_destroy(sub_cfg_t *pub_cfg_config) {
    if(pub_cfg_config != NULL) {
        if (pub_cfg_config->sub_config != NULL) {
            config_value_destroy(pub_cfg_config->sub_config);
        }
        free(pub_cfg_config);
    }
}
