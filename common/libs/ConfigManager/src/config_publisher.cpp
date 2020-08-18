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
 * @brief ConfigMgr Implementation
 */


#include "eis/config_manager/config_publisher.h"

using namespace eis::config_manager;

// Constructor
PublisherCfg::PublisherCfg(config_value_t* pub_config):ConfigHandler(NULL, NULL, NULL) {
    publisher_cfg = pub_config;
    fprintf(stderr,"in PublisherCfg class \n"); 
}

// getMsgBusConfig of Publisher class
// WIP for TCP PROD mode
config_t* PublisherCfg::getMsgBusConfig(){

    config_value_t* publish_json_type = config_value_object_get(publisher_cfg, "Type");

    char* type = publish_json_type->body.string;

    cJSON* json_1 = cJSON_CreateObject();
    cJSON_AddStringToObject(json_1, "type", type);
    config_value_t* publish_json_endpoint = config_value_object_get(publisher_cfg, "EndPoint");
    const char* EndPoint = publish_json_endpoint->body.string;

    if(!strcmp(type, "zmq_ipc")){
        cJSON_AddStringToObject(json_1, "socket_dir", EndPoint);
    } else if(!strcmp(type, "zmq_tcp")){

        if(m_dev_mode) {

            // TCP DEV mode
            cJSON* zmq_tcp_publish = cJSON_CreateObject();
            std::vector<std::string> tokens = ConfigHandler::tokenizer(EndPoint, ":");
            cJSON_AddStringToObject(zmq_tcp_publish, "host", tokens[0].c_str());
            cJSON_AddNumberToObject(zmq_tcp_publish, "port", atoi(tokens[1].c_str()));
            cJSON_AddItemToObject(json_1, "zmq_tcp_publish", zmq_tcp_publish);

        } else {
            // TCP PROD mode
            config_value_t* publish_json_clients = config_value_object_get(publisher_cfg, "AllowedClients");

            config_value_t* array_value; 
            cJSON* all_clients_1 = cJSON_CreateArray();

            for (int i =0; i < config_value_array_len(publish_json_clients); i++) {
                array_value = config_value_array_get(publish_json_clients, i);
                cJSON_AddItemToArray(all_clients_1, cJSON_CreateString(array_value->body.string));
            }
            
            cJSON_AddItemToObject(json_1, "allowed_clients",  all_clients_1);
            cJSON* zmq_tcp_publish = cJSON_CreateObject();

            std::vector<std::string> tokens = ConfigHandler::tokenizer(EndPoint, ":");
            cJSON_AddStringToObject(zmq_tcp_publish, "host", tokens[0].c_str());
            cJSON_AddNumberToObject(zmq_tcp_publish, "port", atoi(tokens[1].c_str()));
            cJSON_AddItemToObject(json_1, "zmq_tcp_publish", zmq_tcp_publish);
            
        }
    }
    
    char* config_value_1 = cJSON_Print(json_1);
    fprintf(stderr,"Env publish Config is : %s \n", config_value_1);

    config = config_new(
            (void*) json_1, free_json, get_config_value);
    if (config == NULL) {
        LOG_ERROR_0("Failed to initialize configuration object");
        return NULL;
    }

    return config;
}

// To fetch endpoint from config
std::string PublisherCfg::getEndpoint() {
    config_value_t* endpoint = config_value_object_get(publisher_cfg, "EndPoint");
    char* type = endpoint->body.string;
    std::string s(type);
    return s;
}

// To fetch topics from config
std::vector<std::string> PublisherCfg::getTopics() {
    config_value_t* list_of_topics = config_value_object_get(publisher_cfg, "Topics");
    config_value_t* topic_value;
    std::vector<std::string> topic_list;
    for (int i =0; i < config_value_array_len(list_of_topics); i++) {
        topic_value = config_value_array_get(list_of_topics, i);
        char* topic = topic_value->body.string;
        std::string temp(topic);
        topic_list.push_back(temp);
    }
    return topic_list;
}

// To set topics in config
bool PublisherCfg::setTopics(std::vector<std::string>) {
    // TODO
    // Implement setTopics()
    return true;
}

// To fetch list of allowed clients from config
std::vector<std::string> PublisherCfg::getAllowedClients() {
    config_value_t* list_of_allowed_clients = config_value_object_get(publisher_cfg, "AllowedClients");
    config_value_t* value;
    std::vector<std::string> client_list;
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
    if(config) {
        delete config;
    }
    if(publisher_cfg) {
        delete publisher_cfg;
    }
    LOG_INFO_0("PublisherCfg destructor");
}