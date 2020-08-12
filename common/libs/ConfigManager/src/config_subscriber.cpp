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


#include "eis/config_manager/config_subscriber.h"

using namespace eis::config_manager;

SubscriberCfg::SubscriberCfg(config_value_t* sub_config):ConfigHandler(NULL, NULL, NULL) {
    subscriber_cfg = sub_config;
    fprintf(stderr,"in PublisherCfg class \n"); 
}

// getMsgBusConfig of Subscriber class
// Currently working for IPC mode only
config_t* SubscriberCfg::getMsgBusConfig(){

    config_value_t* subscribe_json_type = config_value_object_get(subscriber_cfg, "Type");

    char* type = subscribe_json_type->body.string;
    cJSON* json_1 = cJSON_CreateObject(); 
    cJSON_AddStringToObject(json_1, "type", type);
    config_value_t* subscribe_json_endpoint = config_value_object_get(subscriber_cfg, "EndPoint");
    char* EndPoint = subscribe_json_endpoint->body.string;

    if(!strcmp(type, "zmq_ipc")){
        cJSON_AddStringToObject(json_1, "socket_dir", EndPoint);
    } else if(!strcmp(type, "zmq_tcp")){

        if(m_dev_mode) {

            config_value_t* topic_Arr = config_value_object_get(subscriber_cfg, "Topics");
            config_value_t* topic ;
            
            for (int i = 0; i < config_value_array_len(topic_Arr); i++){
                cJSON* sub_topic = cJSON_CreateObject();
                topic = config_value_array_get(topic_Arr, i);
                std::vector<std::string> tokens = ConfigHandler::tokenizer(EndPoint, ":");
                cJSON_AddStringToObject(sub_topic, "host", tokens[0].c_str());
                cJSON_AddNumberToObject(sub_topic, "port", atoi(tokens[1].c_str()));

                cJSON_AddItemToObject(json_1, topic->body.string, sub_topic);
            }

        } else {

            config_value_t* topic_Arr = config_value_object_get(subscriber_cfg, "Topics");
            config_value_t* topic ;
            
            for (int i = 0; i < config_value_array_len(topic_Arr); i++){
                cJSON* sub_topic = cJSON_CreateObject();
                topic = config_value_array_get(topic_Arr, i);
                std::vector<std::string> tokens = ConfigHandler::tokenizer(EndPoint, ":");
                cJSON_AddStringToObject(sub_topic, "host", tokens[0].c_str());
                cJSON_AddNumberToObject(sub_topic, "port", atoi(tokens[1].c_str()));
                // cJSON_AddStringToObject(sub_topic, "host", EndPoint);
                // cJSON_AddNumberToObject(sub_topic, "port", 12345);
                cJSON_AddStringToObject(sub_topic, "ServerPublicKey", "server_public_key_value");
                cJSON_AddStringToObject(sub_topic, "ClientPublicKey", "client_public_key_value");
                cJSON_AddStringToObject(sub_topic, "ClientPrivateKey", "client_private_key_value");

                cJSON_AddItemToObject(json_1, topic->body.string, sub_topic);
            }

        }

    }
    
    char* config_value_1 = cJSON_Print(json_1);
    fprintf(stderr,"Env Subscribe Config is : %s \n", config_value_1);

    config = config_new(
            (void*) json_1, free_json, get_config_value);
    if (config == NULL) {
        LOG_ERROR_0("Failed to initialize configuration object");
        return NULL;
    }

    return config;
}

SubscriberCfg::~SubscriberCfg() {
    // if(m_app_name) {
    //     delete m_app_name;
    // }
    // Stop the thread (if it is running)
    LOG_INFO_0("SubscriberCfg destructor");
}