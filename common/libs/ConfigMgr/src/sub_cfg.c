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


#include "eis/config_manager/c_sub_cfg.h"
#include <stdarg.h>

#define MAX_CONFIG_KEY_LENGTH 250

// To fetch endpoint from config
char* get_endpoint_sub(base_cfg_t* base_cfg) {
    config_value_t* sub_config = base_cfg->pub_sub_config;
    // Fetching EndPoint from config
    config_value_t* end_point = config_value_object_get(sub_config, "EndPoint");
    char* ep = end_point->body.string;
    return ep;
}

// To fetch topics from config
char** get_topics_sub(base_cfg_t* base_cfg) {
    config_value_t* sub_config = base_cfg->pub_sub_config;
    // Fetching Topics from config
    config_value_t* list_of_topics = config_value_object_get(sub_config, "Topics");
    config_value_t* topic_value;
    char **topics_list = calloc(config_value_array_len(list_of_topics), sizeof(char*));
    // Iterating through Topics and adding them to topics_list
    for (int i =0; i < config_value_array_len(list_of_topics); i++) {
        topic_value = config_value_array_get(list_of_topics, i);
        topics_list[i] = topic_value->body.string;
    }
    return topics_list;
}

// To set topics in config
int set_topics_sub(char** topics_list, base_cfg_t* base_cfg) {

    config_value_t* sub_config = base_cfg->pub_sub_config;

    // Fetching topics
    config_value_t* list_of_topics = config_value_object_get(sub_config, "Topics");
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
config_t* get_msgbus_config_sub(base_cfg_t* base_cfg) {

    // Initializing base_cfg variables
    config_value_t* sub_config = base_cfg->pub_sub_config;
    char* app_name = base_cfg->app_name;
    int dev_mode = base_cfg->dev_mode;
    kv_store_client_t* m_kv_store_handle = base_cfg->m_kv_store_handle;
    // Creating cJSON object
    cJSON* c_json = cJSON_CreateObject();

    // Fetching Type from config
    config_value_t* subscribe_json_type = config_value_object_get(sub_config, "Type");
    char* type = subscribe_json_type->body.string;
    cJSON_AddStringToObject(c_json, "type", type);

    // Fetching EndPoint from config
    config_value_t* subscribe_json_endpoint = config_value_object_get(sub_config, "EndPoint");
    char* end_point = subscribe_json_endpoint->body.string;

    if(!strcmp(type, "zmq_ipc")){
        // Add Endpoint directly to socket_dir if IPC mode
        cJSON_AddStringToObject(c_json, "socket_dir", end_point);
    } else if(!strcmp(type, "zmq_tcp")){

        // Fetching Topics from config
        config_value_t* topic_array = config_value_object_get(sub_config, "Topics");
        config_value_t* topic;
            
        // Create cJSON object for every topic
        cJSON* sub_topic = cJSON_CreateObject();
        for (int i = 0; i < config_value_array_len(topic_array); i++) {
            topic = config_value_array_get(topic_array, i);
            // Add host & port to cJSON object
            char** host_port = get_host_port(end_point);
            char* host = host_port[0];
            trim(host);
            char* port = host_port[1];
            trim(port);
            __int64_t i_port = atoi(port);

            cJSON_AddStringToObject(sub_topic, "host", host);
            cJSON_AddNumberToObject(sub_topic, "port", i_port);
            cJSON_AddItemToObject(c_json, topic->body.string, sub_topic);
        }

        if(dev_mode != 0) {

            // Initializing m_kv_store_handle to fetch public & private keys
            void *handle = m_kv_store_handle->init(m_kv_store_handle);

            // Fetching Topics from config
            config_value_t* topic_array = config_value_object_get(sub_config, "Topics");
            config_value_t* topic;

            // Create cJSON object for every topic
            for (int i = 0; i < config_value_array_len(topic_array); i++) {
                topic = config_value_array_get(topic_array, i);

                // Fetching Publisher AppName from config
                config_value_t* publisher_appname = config_value_object_get(sub_config, "AppName");

                size_t init_len = strlen("/Publickeys/") + strlen(publisher_appname->body.string) + 2;
                char* grab_public_key = concat_s(init_len, 2, "/Publickeys/", publisher_appname->body.string);
                const char* pub_public_key = m_kv_store_handle->get(handle, grab_public_key);
                if(pub_public_key == NULL){
                    LOG_ERROR("Value is not found for the key: %s", grab_public_key);
                }

                // Adding Publisher public key to config
                cJSON_AddStringToObject(sub_topic, "server_public_key", pub_public_key);

                // Adding Subscriber public key to config
                init_len = strlen("/Publickeys/") + strlen(app_name) + 2;
                char* s_sub_public_key = concat_s(init_len, 2, "/Publickeys/", app_name);
                const char* sub_public_key = m_kv_store_handle->get(handle, s_sub_public_key);
                if(sub_public_key == NULL){
                    LOG_ERROR("Value is not found for the key: %s", s_sub_public_key);
                }

                cJSON_AddStringToObject(sub_topic, "client_public_key", sub_public_key);

                // Adding Subscriber private key to config
                init_len = strlen("/") + strlen(app_name) + strlen("/private_key") + 2;
                char* s_sub_pri_key = concat_s(init_len, 3, "/", app_name, "/private_key");
                const char* sub_pri_key = m_kv_store_handle->get(handle, s_sub_pri_key);
                if(sub_pri_key == NULL){
                    LOG_ERROR("Value is not found for the key: %s", s_sub_pri_key);
                }

                cJSON_AddStringToObject(sub_topic, "client_secret_key", sub_pri_key);
            }
        }
    }

    // Constructing char* object from cJSON object
    char* config_value_cr = cJSON_Print(c_json);
    LOG_DEBUG("Env subscriber Config is : %s \n", config_value_cr);

    // Constructing config_t object from cJSON object
    config_t* m_config = config_new(
            (void*) c_json, free_json, get_config_value);
    if (m_config == NULL) {
        LOG_ERROR_0("Failed to initialize configuration object");
        return NULL;
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
    sub_cfg_mgr->get_msgbus_config_sub = get_msgbus_config_sub;
    sub_cfg_mgr->get_endpoint_sub = get_endpoint_sub;
    sub_cfg_mgr->get_topics_sub = get_topics_sub;
    sub_cfg_mgr->set_topics_sub = set_topics_sub;
    return sub_cfg_mgr;
}

// function to destroy sub_cfg_t
void sub_cfg_config_destroy(sub_cfg_t *pub_cfg_config) {
    if(pub_cfg_config != NULL) {
        free(pub_cfg_config);
    }
}