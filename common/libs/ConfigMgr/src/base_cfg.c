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
#include "eis/config_manager/base_cfg.h"

#define MAX_CONFIG_KEY_LENGTH 250

// To fetch endpoint from config
config_value_t* get_endpoint_base(base_cfg_t* base_cfg) {
    config_value_t* pub_config = base_cfg->msgbus_config;
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
config_value_t* get_topics_base(base_cfg_t* base_cfg) {
    config_value_t* config = base_cfg->msgbus_config;
    if (config == NULL) {
        LOG_ERROR_0("config initialization failed");
        return NULL;
    }
    config_value_t* topics = config_value_object_get(config, "Topics");
    if (topics == NULL) {
        LOG_ERROR_0("topics initialization failed");
        return NULL;
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

// To fetch list of allowed clients from config
config_value_t* get_allowed_clients_base(base_cfg_t* base_cfg) {
    config_value_t* config = base_cfg->msgbus_config;
    if (config == NULL) {
        LOG_ERROR_0("config initialization failed");
        return NULL;
    }
    config_value_t* clients = config_value_object_get(config, "AllowedClients");
    if (clients == NULL) {
        LOG_ERROR_0("topics initialization failed");
        return NULL;
    }
    return clients;
}

// To set topics in config
int set_topics_base(char** topics_list, int len, const char* type, base_cfg_t* base_cfg) {

    // Fetching the name of pub/sub interface
    config_value_t* config_name = config_value_object_get(base_cfg->msgbus_config, "Name");
    if (config_name == NULL) {
        LOG_ERROR_0("config_name initialization failed");
        return -1;
    }
    // Constructing cJSON object from obtained interface
    cJSON* temp = (cJSON*)base_cfg->m_app_interface->cfg;
    if (temp == NULL) {
        LOG_ERROR_0("cJSON temp object is NULL");
        return -1;
    }

    // Creating cJSON object of new topics
    cJSON* obj = cJSON_CreateArray();
    if (obj == NULL) {
        LOG_ERROR_0("cJSON obj initialization failed");
        return -1;
    }
    for (int i = 0; i < len; i++) {
        cJSON_AddItemToArray(obj, cJSON_CreateString(topics_list[i]));
    }

    // Fetching list of publishers/subscribers
    cJSON* config_list = cJSON_GetObjectItem(temp, type);
    if (config_list == NULL) {
        LOG_ERROR_0("cJSON config_list initialization failed");
        return -1;
    }
    // Iterating & validating name
    int config_size = cJSON_GetArraySize(config_list);
    for (int i = 0; i < config_size; i++) {
        cJSON* config_list_name = cJSON_GetArrayItem(config_list, i);
        if (config_list_name == NULL) {
            LOG_ERROR_0("config_list_name initialization failed");
            return -1;
        }
        char* name_to_check = cJSON_GetStringValue(cJSON_GetObjectItem(config_list_name, "Name"));
        if (name_to_check == NULL) {
            LOG_ERROR_0("name_to_check initialization failed");
            return -1;
        }
        // Replace topics if name matches
        if (strcmp(name_to_check, config_name->body.string) == 0) {
            cJSON_ReplaceItemInObject(config_list_name, "Topics", obj);
        }
    }
    return 0;
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

// base C function to fetch app config
config_t* get_app_config(base_cfg_t* base_cfg) {
    return base_cfg->m_app_config;
}

// base C function to fetch app config
config_t* get_app_interface(base_cfg_t* base_cfg) {
    return base_cfg->m_app_interface;
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
base_cfg_t* base_cfg_new(config_value_t* pub_config, char* app_name, int dev_mode, kv_store_client_t* m_kv_store_handle) {
    base_cfg_t *base_cfg = (base_cfg_t *)malloc(sizeof(base_cfg_t));
    if (base_cfg == NULL) {
        LOG_ERROR_0("base_cfg initialization failed");
        return NULL;
    }
    base_cfg->msgbus_config = pub_config;
    base_cfg->app_name = app_name;
    base_cfg->dev_mode = dev_mode;
    base_cfg->m_kv_store_handle = m_kv_store_handle;
    return base_cfg;
}

// Destructor
void base_cfg_config_destroy(base_cfg_t *base_cfg_config) {
    if(base_cfg_config != NULL) {
        free(base_cfg_config);
    }
}