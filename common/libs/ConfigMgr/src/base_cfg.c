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
 * @brief ConfigMgr utility C Implementation
 * Holds the implementaion of utilities supported for ConfigMgr
 */

#include <stdarg.h>
#include "eii/config_manager/base_cfg.h"

#define MAX_CONFIG_KEY_LENGTH 250

// To fetch endpoint from config
config_value_t* get_endpoint_base(config_value_t* conf) {
    config_value_t* pub_config = conf;
    if (pub_config == NULL) {
        LOG_ERROR_0("pub_config initialization failed");
        return NULL;
    }
    // Fetching EndPoint from config
    config_value_t* end_point = config_value_object_get(pub_config, "EndPoint");
    if (end_point == NULL) {
        LOG_ERROR_0("end_point initialization failed");
        return NULL;
    }
    return end_point;
}

// To fetch topics from config
// Get topics base returns the value mapped to Topics key in the Applications Interface.
// If "*" is mentioned in topics, then it is replaced by empty string ,
// as our EIIMessageBus supports the prefix approach, empty prefix considers all/any the topics. 

config_value_t* get_topics_base(config_value_t* conf) {
    config_value_t* config = conf;
    cJSON *arr = NULL;

    if (config == NULL) {
        LOG_ERROR_0("config initialization failed");
        return NULL;
    }
    config_value_t* topics = config_value_object_get(conf, "Topics");
    if (topics == NULL) {
        LOG_ERROR_0("topics initialization failed");
        return NULL;
    }

    if (topics->type != CVT_ARRAY) {
        LOG_ERROR_0("Topics type mismatch, it should be array");
        return NULL;
    }

    arr = cJSON_CreateArray();
    if (arr == NULL) {
        LOG_ERROR_0("arr initialization failed");
        return NULL;
    }
    int ret;

    size_t arrlen = config_value_array_len(topics);
    if (arrlen == 0){
        LOG_ERROR_0("Empty String is not supported in Topics. Atleast one topic is required");
        return NULL;
    }

    config_value_t* topic_value = NULL;
    topic_value = config_value_array_get(topics, 0);
    if (topic_value == NULL) {
        LOG_ERROR_0("Extracting first topic from topics array failed ");
        return NULL;
    }

    // If only one item in Topics and it is *,
    // then add empty string in order to allow all clients to subscribe
    strcmp_s(topic_value->body.string, strlen(topic_value->body.string), "*", &ret);

    if((arrlen == 1) && (ret == 0 )){
        cJSON_AddItemToArray(arr, cJSON_CreateString(""));
        config_value_destroy(topics);
        topics = config_value_new_array(
                (void*) arr , cJSON_GetArraySize(arr), get_array_item, NULL);
        if (topics == NULL){
            LOG_ERROR_0("config value new array for topic failed");
            return NULL;
        }
        return topics;
    }

    if (topic_value != NULL) {
        config_value_destroy(topic_value);
    }

    if (arr != NULL) { 
        cJSON_free(arr);
    }

    return topics;
}

// To get number of elements in interfaces
int cfgmgr_get_num_elements_base(const char* type, base_cfg_t* base_cfg) {
    // Fetching list of interface elements
    config_value_t* interfaces = config_get(base_cfg->m_app_interface, type);
    if (interfaces == NULL) {
        LOG_ERROR_0("Failed to fetch number of elements in interface");
        return -1;
    }
    if (interfaces->type != CVT_ARRAY) {
        LOG_ERROR("%s interface is not an array type", type);
        return -1;
    }
    int result = config_value_array_len(interfaces);
    config_value_destroy(interfaces);
    return result;
}

// To check whether environment is dev mode or prod mode
int cfgmgr_is_dev_mode_base(base_cfg_t* base_cfg) {
    // Fetching dev mode from base_cfg
    int result = base_cfg->dev_mode;
    return result;
}

// To fetch appname of any service
config_value_t* cfgmgr_get_appname_base(base_cfg_t* base_cfg) {
    // Fetching app name from base_cfg
    char* appname = base_cfg->app_name;
    config_value_t* app_name = config_value_new_string(appname);
    if (app_name == NULL){
        LOG_ERROR_0("Fetching appname failed");
        return NULL;
    }
    return app_name;
}

// To fetch list of allowed clients from config
// Get Allowed Clients returns the value mapped to AllowedClients key in the Applications Interface.
// If "*" is mentioned in the allowed clients, the return value will still be "*" notifying user
// that all the provisioned applications are allowed to get the topics.
config_value_t* get_allowed_clients_base(config_value_t* conf) {
    config_value_t* config = conf;
    if (config == NULL) {
        LOG_ERROR_0("config initialization failed");
        return NULL;
    }
    config_value_t* clients = config_value_object_get(config, "AllowedClients");
    if (clients == NULL) {
        LOG_ERROR_0("topics initialization failed");
        return NULL;
    }
    if (clients->type != CVT_ARRAY) {
        LOG_ERROR_0("Allowed Clients type mismatch, it should be array");
        return NULL;
    }
    return clients;
}

// To set topics in config
int set_topics_base(char** topics_list, int len, const char* type, base_cfg_t* base_cfg, config_value_t* conf) {
    int ret_val = 0;
    // Fetching the name of pub/sub interface
    config_value_t* config_name = config_value_object_get(conf, "Name");
    if (config_name == NULL) {
        LOG_ERROR_0("config_name initialization failed");
        goto err;
    }
    // Constructing cJSON object from obtained interface
    cJSON* temp = (cJSON*)base_cfg->m_app_interface->cfg;
    if (temp == NULL) {
        LOG_ERROR_0("cJSON temp object is NULL");
        ret_val = -1;
        goto err;
    }

    // Creating cJSON object of new topics
    cJSON* obj = cJSON_CreateArray();
    if (obj == NULL) {
        LOG_ERROR_0("cJSON obj initialization failed");
        ret_val = -1;
        goto err;
    }
    for (int i = 0; i < len; i++) {
        cJSON_AddItemToArray(obj, cJSON_CreateString(topics_list[i]));
    }

    // Fetching list of publishers/subscribers
    cJSON* config_list = cJSON_GetObjectItem(temp, type);
    if (config_list == NULL) {
        LOG_ERROR_0("cJSON config_list initialization failed");
        ret_val = -1;
        goto err;
    }
    // Iterating & validating name
    int config_size = cJSON_GetArraySize(config_list);
    for (int i = 0; i < config_size; i++) {
        cJSON* config_list_name = cJSON_GetArrayItem(config_list, i);
        if (config_list_name == NULL) {
            LOG_ERROR_0("config_list_name initialization failed");
            ret_val = -1;
            goto err;
        }
        char* name_to_check = cJSON_GetStringValue(cJSON_GetObjectItem(config_list_name, "Name"));
        if (name_to_check == NULL) {
            LOG_ERROR_0("name_to_check initialization failed");
            ret_val = -1;
            goto err;
        }
        // Replace topics if name matches
        if (strcmp(name_to_check, config_name->body.string) == 0) {
            cJSON_ReplaceItemInObject(config_list_name, "Topics", obj);
        }
    }

err:
    if (config_name != NULL) {
        config_value_destroy(config_name);
    }

    return ret_val;
}

// Helper function to convert config_t object to char*
char* configt_to_char(config_t* config) {
    cJSON* temp = (cJSON*)config->cfg;
    if(temp == NULL) {
        LOG_ERROR_0("cJSON temp object is NULL");
        return NULL;
    }
    char* config_value_cr = cJSON_Print(temp);
    if(config_value_cr == NULL) {
        LOG_ERROR_0("config_value_cr object is NULL");
        return NULL;
    }
    return config_value_cr;
}

// Helper function to convert config_value_t object to char*
char* cvt_to_char(config_value_t* config) {

    cJSON* c_json = cJSON_CreateObject();
    if (c_json == NULL) {
        LOG_ERROR_0("c_json initialization failed");
        return NULL;
    }

    c_json = (cJSON*)config->body.object->object;
    char* config_value_cr = cJSON_Print(c_json);
    if(config_value_cr == NULL) {
        LOG_ERROR_0("config_value_cr object is NULL");
        return NULL;
    }
    return config_value_cr;
}

// base C function to fetch app config
config_t* get_app_config(base_cfg_t* base_cfg) {
    return base_cfg->m_app_config;
}

// base C function to fetch app config
config_t* get_app_interface(base_cfg_t* base_cfg) {
    return base_cfg->m_app_interface;
}

// base C function to watch on a given key
void cfgmgr_watch(base_cfg_t* base_cfg, char* key, callback_t watch_callback, void* user_data) {
    // Calling the base watch API
    base_cfg->m_kv_store_handle->watch(base_cfg->cfgmgr_handle, key, watch_callback, user_data);
    return;
}

// base C function to watch on a given key prefix
void cfgmgr_watch_prefix(base_cfg_t* base_cfg, char* prefix, callback_t watch_callback, void* user_data) {
    // Calling the base watch_prefix API
    base_cfg->m_kv_store_handle->watch_prefix(base_cfg->cfgmgr_handle, prefix, watch_callback, user_data);
    return;
}

// base C function to fetch app config value
config_value_t* get_app_config_value(base_cfg_t* base_cfg, char* key) {
    return base_cfg->m_app_config->get_config_value(base_cfg->m_app_config->cfg, key);
}

// base C function to fetch app interface value
config_value_t* get_app_interface_value(base_cfg_t* base_cfg, char* key) {
    return base_cfg->m_app_interface->get_config_value(base_cfg->m_app_interface->cfg, key);
}

// Function to initialize required objects & functions
base_cfg_t* base_cfg_new(char* app_name, int dev_mode,
                        kv_store_client_t* m_kv_store_handle,
                        config_value_t* datastore) {
    base_cfg_t* base_cfg = NULL;
    base_cfg = (base_cfg_t *)malloc(sizeof(base_cfg_t));
    if (base_cfg == NULL) {
        LOG_ERROR_0("base_cfg initialization failed");
        return NULL;
    }
    base_cfg->app_name = app_name;
    base_cfg->dev_mode = dev_mode;
    base_cfg->m_kv_store_handle = m_kv_store_handle;
    // Assigining this to NULL as its currently not being used
    base_cfg->m_data_store = datastore;
    return base_cfg;
}

// Destructor
void base_cfg_config_destroy(base_cfg_t *base_cfg) {
    LOG_DEBUG_0("base_cfg_config_destroy");
    if (base_cfg != NULL) {
        if (base_cfg->m_app_config) {
            config_destroy(base_cfg->m_app_config);
        }
        if (base_cfg->m_app_interface) {
            config_destroy(base_cfg->m_app_interface);
        }
        if (base_cfg->m_data_store) {
            config_destroy(base_cfg->m_data_store);
        }
        if (base_cfg->cfgmgr_handle) {
            free(base_cfg->cfgmgr_handle);
        }
        if (base_cfg->app_name) {
            free(base_cfg->app_name);
        }
        if (base_cfg->m_kv_store_handle) {
            kv_client_free(base_cfg->m_kv_store_handle);
        }
        free(base_cfg);
    }
    LOG_DEBUG_0("base_cfg_config_destroy: Done");
}
