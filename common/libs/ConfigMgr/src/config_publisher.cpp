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
 * @file
 * @brief PublisherCfg Implementation
 * Holds the implementaion of APIs supported by PublisherCfg class
 */


#include "eis/config_manager/config_publisher.h"

using namespace eis::config_manager;

// Constructor
PublisherCfg::PublisherCfg(config_value_t* pub_config):AppCfg(NULL, NULL, NULL) {
    publisher_cfg = pub_config;
}

// getMsgBusConfig of Publisher class
config_t* PublisherCfg::getMsgBusConfig(){

    // Creating cJSON object
    cJSON* c_json = cJSON_CreateObject();

    // Fetching Type from config
    config_value_t* publish_json_type = config_value_object_get(publisher_cfg, "Type");
    char* type = publish_json_type->body.string;
    cJSON_AddStringToObject(c_json, "type", type);

    // Fetching EndPoint from config
    config_value_t* publish_json_endpoint = config_value_object_get(publisher_cfg, "EndPoint");
    const char* end_point = publish_json_endpoint->body.string;

    if(!strcmp(type, "zmq_ipc")){
        // Add Endpoint directly to socket_dir if IPC mode
        cJSON_AddStringToObject(c_json, "socket_dir", end_point);
    } else if(!strcmp(type, "zmq_tcp")){

        if(m_dev_mode) {

            // TCP DEV mode
            // Add host & port to zmq_tcp_publish cJSON object
            cJSON* zmq_tcp_publish = cJSON_CreateObject();
            std::vector<std::string> tokens = AppCfg::tokenizer(end_point, ":");
            cJSON_AddStringToObject(zmq_tcp_publish, "host", tokens[0].c_str());
            cJSON_AddNumberToObject(zmq_tcp_publish, "port", atoi(tokens[1].c_str()));
            cJSON_AddItemToObject(c_json, "zmq_tcp_publish", zmq_tcp_publish);

        } else {
            // TCP PROD mode

            // Initializing db_client handle to fetch public & private keys
            void *handle = m_db_client_handle->init(m_db_client_handle);

            // Fetching AllowedClients from config
            config_value_t* publish_json_clients = config_value_object_get(publisher_cfg, "AllowedClients");
            config_value_t* array_value; 
            cJSON* all_clients = cJSON_CreateArray();
            for (int i =0; i < config_value_array_len(publish_json_clients); i++) {
                // Fetching individual public keys of all AllowedClients
                array_value = config_value_array_get(publish_json_clients, i);
                std::string sub_app_name(array_value->body.string);
                std::string grab_public_key = "/Publickeys/" + sub_app_name;
                const char* sub_public_key = m_db_client_handle->get(handle, &grab_public_key[0]);
                cJSON_AddItemToArray(all_clients, cJSON_CreateString(sub_public_key));
            }

            // Adding all public keys of clients to allowed_clients of config
            cJSON_AddItemToObject(c_json, "allowed_clients",  all_clients);

            // Creating zmq_tcp_publish object
            cJSON* zmq_tcp_publish = cJSON_CreateObject();
            std::vector<std::string> tokens = AppCfg::tokenizer(end_point, ":");
            // Adding host & port to zmq_tcp_publish object
            cJSON_AddStringToObject(zmq_tcp_publish, "host", tokens[0].c_str());
            cJSON_AddNumberToObject(zmq_tcp_publish, "port", atoi(tokens[1].c_str()));

            // Fetching Publisher private key & adding it to zmq_tcp_publish object
            std::string pub_pri_key = "/" + m_app_name + "/private_key";
            const char* publisher_secret_key = m_db_client_handle->get(handle, &pub_pri_key[0]);
            cJSON_AddStringToObject(zmq_tcp_publish, "server_secret_key", publisher_secret_key);

            // Creating the final cJSON config object
            cJSON_AddItemToObject(c_json, "zmq_tcp_publish", zmq_tcp_publish);
        }
    }

    // Constructing char* object from cJSON object
    char* config_value_cr = cJSON_Print(c_json);
    LOG_DEBUG("Env publisher Config is : %s \n", config_value_cr);

    // Constructing config_t object from cJSON object
    config = config_new(
            (void*) c_json, free_json, get_config_value);
    if (config == NULL) {
        LOG_ERROR_0("Failed to initialize configuration object");
        return NULL;
    }
    return config;
}

// To fetch endpoint from config
std::string PublisherCfg::getEndpoint() {
    // Fetching EndPoint from config
    config_value_t* end_point = config_value_object_get(publisher_cfg, "EndPoint");
    char* type = end_point->body.string;
    std::string s(type);
    return s;
}

// To fetch topics from config
std::vector<std::string> PublisherCfg::getTopics() {
    // Fetching Topics from config
    config_value_t* list_of_topics = config_value_object_get(publisher_cfg, "Topics");
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
bool PublisherCfg::setTopics(std::vector<std::string> topics_list) {

    // Fetching topics
    config_value_t* list_of_topics = config_value_object_get(publisher_cfg, "Topics");
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

// To fetch list of allowed clients from config
std::vector<std::string> PublisherCfg::getAllowedClients() {
    // Fetching AllowedClients from config
    config_value_t* list_of_allowed_clients = config_value_object_get(publisher_cfg, "AllowedClients");
    config_value_t* value;
    std::vector<std::string> client_list;
    // Iterating through AllowedClients and adding them to client_list vector
    for (int i =0; i < config_value_array_len(list_of_allowed_clients); i++) {
        value = config_value_array_get(list_of_allowed_clients, i);
        char* cli = value->body.string;
        std::string temp(cli);
        client_list.push_back(temp);
    }
    return client_list;
}

// Destructor
PublisherCfg::~PublisherCfg() {
    if(publisher_cfg) {
        delete publisher_cfg;
    }
    if(config) {
        delete config;
    }
    LOG_INFO_0("PublisherCfg destructor");
}