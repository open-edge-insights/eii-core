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
 * @brief SubscriberCfg Implementation
 * Holds the implementaion of APIs supported by SubscriberCfg class
 */


#include "eis/config_manager/subscriber_cfg.h"

using namespace eis::config_manager;

// Constructor
SubscriberCfg::SubscriberCfg(config_value_t* sub_config):AppCfg(NULL, NULL, NULL) {
    m_subscriber_cfg = sub_config;
}

// getMsgBusConfig of Subscriber class
config_t* SubscriberCfg::getMsgBusConfig(){

    // Creating cJSON object
    cJSON* c_json = cJSON_CreateObject();

    // Fetching Type from config
    config_value_t* subscribe_json_type = config_value_object_get(m_subscriber_cfg, "Type");
    char* type = subscribe_json_type->body.string;
    cJSON_AddStringToObject(c_json, "type", type);

    // Fetching EndPoint from config
    config_value_t* subscribe_json_endpoint = config_value_object_get(m_subscriber_cfg, "EndPoint");
    char* EndPoint = subscribe_json_endpoint->body.string;

    if(!strcmp(type, "zmq_ipc")){
        // Add Endpoint directly to socket_dir if IPC mode
        cJSON_AddStringToObject(c_json, "socket_dir", EndPoint);
    } else if(!strcmp(type, "zmq_tcp")){

        if(m_dev_mode) {

            // TCP DEV mode

            // Fetching Topics from config
            config_value_t* topic_array = config_value_object_get(m_subscriber_cfg, "Topics");
            config_value_t* topic;
            
            // Create cJSON object for every topic
            for (int i = 0; i < config_value_array_len(topic_array); i++){
                cJSON* sub_topic = cJSON_CreateObject();
                topic = config_value_array_get(topic_array, i);
                // Add host & port to cJSON object
                std::vector<std::string> tokens = AppCfg::tokenizer(EndPoint, ":");
                cJSON_AddStringToObject(sub_topic, "host", tokens[0].c_str());
                cJSON_AddNumberToObject(sub_topic, "port", atoi(tokens[1].c_str()));
                cJSON_AddItemToObject(c_json, topic->body.string, sub_topic);
            }
        } else {
            // TCP PROD mode

            // Initializing db_client handle to fetch public & private keys
            void *handle = m_kv_store_client_handle->init(m_kv_store_client_handle);

            // Fetching Topics from config
            config_value_t* topic_array = config_value_object_get(m_subscriber_cfg, "Topics");
            config_value_t* topic;

            // Create cJSON object for every topic
            for (int i = 0; i < config_value_array_len(topic_array); i++) {
                cJSON* sub_topic = cJSON_CreateObject();
                topic = config_value_array_get(topic_array, i);
                // Add host & port to cJSON object
                std::vector<std::string> tokens = AppCfg::tokenizer(EndPoint, ":");
                cJSON_AddStringToObject(sub_topic, "host", tokens[0].c_str());
                cJSON_AddNumberToObject(sub_topic, "port", atoi(tokens[1].c_str()));

                // Fetching Publisher AppName from config
                config_value_t* publisher_appname = config_value_object_get(m_subscriber_cfg, "AppName");
                std::string pub_app_name(publisher_appname->body.string);

                // Adding Publisher public key to config
                std::string retreive_pub_app_key = "/Publickeys/" + pub_app_name;
                const char* pub_public_key = m_kv_store_client_handle->get(handle, &retreive_pub_app_key[0]);
                cJSON_AddStringToObject(sub_topic, "server_public_key", pub_public_key);

                // Adding Subscriber public key to config
                std::string s_sub_public_key = "/Publickeys/" + m_app_name;
                const char* sub_public_key = m_kv_store_client_handle->get(handle, &s_sub_public_key[0]);
                cJSON_AddStringToObject(sub_topic, "client_public_key", sub_public_key);

                // Adding Subscriber private key to config
                std::string s_sub_pri_key = "/" + m_app_name + "/private_key";
                const char* sub_pri_key = m_kv_store_client_handle->get(handle, &s_sub_pri_key[0]);
                cJSON_AddStringToObject(sub_topic, "client_secret_key", sub_pri_key);

                // Creating the final cJSON config object
                cJSON_AddItemToObject(c_json, topic->body.string, sub_topic);
            }
        }
    }

    // Constructing char* object from cJSON object
    char* config_value_cr = cJSON_Print(c_json);
    LOG_DEBUG("Env subscriber Config is : %s \n", config_value_cr);

    // Constructing config_t object from cJSON object
    m_config = config_new(
            (void*) c_json, free_json, get_config_value);
    if (m_config == NULL) {
        LOG_ERROR_0("Failed to initialize configuration object");
        return NULL;
    }

    return m_config;
}

// To fetch endpoint from config
std::string SubscriberCfg::getEndpoint() {
    // Fetching EndPoint from config
    config_value_t* endpoint = config_value_object_get(m_subscriber_cfg, "EndPoint");
    char* type = endpoint->body.string;
    std::string s(type);
    return s;
}

// To fetch topics from config
std::vector<std::string> SubscriberCfg::getTopics() {
    // Fetching Topics from config
    config_value_t* list_of_topics = config_value_object_get(m_subscriber_cfg, "Topics");
    config_value_t* topic_value;
    std::vector<std::string> topic_list;
    // Iterating through Topics and adding them to topics_list vector
    for (int i =0; i < config_value_array_len(list_of_topics); i++) {
        topic_value = config_value_array_get(list_of_topics, i);
        char* topic = topic_value->body.string;
        std::string temp(topic);
        topic_list.push_back(temp);
    }
    return topic_list;
}

// To set topics in config
bool SubscriberCfg::setTopics(std::vector<std::string> topics_list) {

    // Fetching topics
    config_value_t* list_of_topics = config_value_object_get(m_subscriber_cfg, "Topics");
    config_value_t* topic_value;
    for (int i =0; i < config_value_array_len(list_of_topics); i++) {
        topic_value = config_value_array_get(list_of_topics, i);
    }

    // Creating cJSON object from topics to be set
    cJSON* obj = cJSON_CreateArray();
    for (int i =0; i < topics_list.size(); i++) {
        cJSON_AddItemToArray(obj, cJSON_CreateString(topics_list[i].c_str()));
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

    return true;
}

// Destructor
SubscriberCfg::~SubscriberCfg() {
    if(m_config) {
        delete m_config;
    }
    if(m_subscriber_cfg) {
        delete m_subscriber_cfg;
    }
    LOG_INFO_0("SubscriberCfg destructor");
}