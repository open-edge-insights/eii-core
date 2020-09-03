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


#include "eis/config_manager/c_pub_cfg.h"
#include <stdarg.h>

#define MAX_CONFIG_KEY_LENGTH 250

// To fetch endpoint from config
char* get_endpoint(base_cfg_t* base_cfg) {
    config_value_t* pub_config = base_cfg->pub_sub_config;
    // Fetching EndPoint from config
    config_value_t* end_point = config_value_object_get(pub_config, "EndPoint");
    char* ep = end_point->body.string;
    return ep;
}

// To fetch topics from config
char** get_topics(base_cfg_t* base_cfg) {
    config_value_t* pub_config = base_cfg->pub_sub_config;
    // Fetching Topics from config
    config_value_t* list_of_topics = config_value_object_get(pub_config, "Topics");
    config_value_t* topic_value;
    char **topics_list = calloc(config_value_array_len(list_of_topics), sizeof(char*));
    // Iterating through Topics and adding them to topics_list
    for (int i =0; i < config_value_array_len(list_of_topics); i++) {
        topic_value = config_value_array_get(list_of_topics, i);
        topics_list[i] = topic_value->body.string;
    }
    return topics_list;
}

// To fetch list of allowed clients from config
char** get_allowed_clients(base_cfg_t* base_cfg) {
    config_value_t* pub_config = base_cfg->pub_sub_config;
    // Fetching AllowedClients from config
    config_value_t* list_of_allowed_clients = config_value_object_get(pub_config, "AllowedClients");
    config_value_t* value;
    char **client_list = calloc(config_value_array_len(list_of_allowed_clients), sizeof(char*));
    for (int i =0; i < config_value_array_len(list_of_allowed_clients); i++) {
        value = config_value_array_get(list_of_allowed_clients, i);
        client_list[i] = value->body.string;
    }
    return client_list;
}

// To set topics in config
int set_topics(char** topics_list, base_cfg_t* base_cfg) {

    config_value_t* pub_config = base_cfg->pub_sub_config;

    // Fetching topics
    config_value_t* list_of_topics = config_value_object_get(pub_config, "Topics");
    config_value_t* topic_value;
    for (int i =0; i < config_value_array_len(list_of_topics); i++) {
        topic_value = config_value_array_get(list_of_topics, i);
    }

    // Creating cJSON object from topics to be set
    cJSON* obj = cJSON_CreateArray();
    for (char* c = *topics_list; c; c=*++topics_list) {
        LOG_INFO("topic set : %s", c);
        cJSON_AddItemToArray(obj, cJSON_CreateString(c));
    }

    // Creating config_value_t object from cJSON object
    config_value_t* new_config_value = config_value_new_array(
                (void*) obj, cJSON_GetArraySize(obj), get_array_item, NULL);

    // Removing previously set topics
    for (int i =0; i < config_value_array_len(list_of_topics); i++) {
        topic_value = config_value_array_get(list_of_topics, i);
        topic_value->body.string = NULL;
        list_of_topics->body.array->length = list_of_topics->body.array->length - 1;
    }

    // Setting topics
    list_of_topics = new_config_value;
    for (int i =0; i < config_value_array_len(list_of_topics); i++) {
        topic_value = config_value_array_get(list_of_topics, i);
    }
    return 0;
}

// To fetch msgbus config
config_t* get_msgbus_config(base_cfg_t* base_cfg) {

    // Initializing base_cfg variables
    config_value_t* pub_config = base_cfg->pub_sub_config;
    char* app_name = base_cfg->app_name;
    int dev_mode = base_cfg->dev_mode;
    kv_store_client_t* m_kv_store_handle = base_cfg->m_kv_store_handle;

    // Creating cJSON object
    cJSON* c_json = cJSON_CreateObject();

    // Fetching Type from config
    config_value_t* publish_json_type = config_value_object_get(pub_config, "Type");
    char* type = publish_json_type->body.string;
    cJSON_AddStringToObject(c_json, "type", type);

    // Fetching EndPoint from config
    config_value_t* publish_json_endpoint = config_value_object_get(pub_config, "EndPoint");
    const char* end_point = publish_json_endpoint->body.string;

    if(!strcmp(type, "zmq_ipc")) {
        // Add Endpoint directly to socket_dir if IPC mode
        cJSON_AddStringToObject(c_json, "socket_dir", end_point);
    } else if(!strcmp(type, "zmq_tcp")) {

        // Add host & port to zmq_tcp_publish cJSON object
        char** host_port = get_host_port(end_point);
        char* host = host_port[0];
        trim(host);
        char* port = host_port[1];
        trim(port);
        __int64_t i_port = atoi(port);

        cJSON* zmq_tcp_publish = cJSON_CreateObject();
        cJSON_AddStringToObject(zmq_tcp_publish, "host", host);
        cJSON_AddNumberToObject(zmq_tcp_publish, "port", i_port);
        cJSON_AddItemToObject(c_json, "zmq_tcp_publish", zmq_tcp_publish);

        // TODO
        // To override with PUBLISHER_0_ENDPOINT / PUBLISHER__ENDPOINT for CSL usecase
        if(dev_mode != 0) {
            void *handle = m_kv_store_handle->init(m_kv_store_handle);

            // Fetching AllowedClients from config
            config_value_t* publish_json_clients = config_value_object_get(pub_config, "AllowedClients");
            config_value_t* array_value; 
            cJSON* all_clients = cJSON_CreateArray();
            for (int i =0; i < config_value_array_len(publish_json_clients); i++) {
                // Fetching individual public keys of all AllowedClients
                array_value = config_value_array_get(publish_json_clients, i);
                size_t init_len = strlen("/Publickeys/") + strlen(array_value->body.string) + 2;
                char* grab_public_key = concat_s(init_len, 2, "/Publickeys/", array_value->body.string);
                const char* sub_public_key = m_kv_store_handle->get(handle, grab_public_key);
                cJSON_AddItemToArray(all_clients, cJSON_CreateString(sub_public_key));
            }

            // Adding all public keys of clients to allowed_clients of config
            cJSON_AddItemToObject(c_json, "allowed_clients",  all_clients);

            // Fetching Publisher private key & adding it to zmq_tcp_publish object
            size_t init_len = strlen("/") + strlen(app_name) + strlen("/private_key") + 2;
            char* pub_pri_key = concat_s(init_len, 3, "/", app_name, "/private_key");
            const char* publisher_secret_key = m_kv_store_handle->get(handle, pub_pri_key);
            
            cJSON_AddStringToObject(zmq_tcp_publish, "server_secret_key", publisher_secret_key);

        } 
    }

    // Constructing char* object from cJSON object
    char* config_value_cr = cJSON_Print(c_json);
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
    pub_cfg_mgr->get_msgbus_config = get_msgbus_config;
    pub_cfg_mgr->get_endpoint = get_endpoint;
    pub_cfg_mgr->get_topics = get_topics;
    pub_cfg_mgr->set_topics = set_topics;
    pub_cfg_mgr->get_allowed_clients = get_allowed_clients;
    return pub_cfg_mgr;
}

// function to destroy cli_cfg_t
void pub_cfg_config_destroy(pub_cfg_t *pub_cfg_config) {
    if(pub_cfg_config != NULL) {
        free(pub_cfg_config);
    }
}