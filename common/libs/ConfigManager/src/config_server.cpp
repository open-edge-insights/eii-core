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


#include "eis/config_manager/config_server.h"

using namespace eis::config_manager;

ServerCfg::ServerCfg(config_value_t* server_config):ConfigHandler(NULL, NULL, NULL) {
    server_cfg = server_config;
    fprintf(stderr,"in ServerCfg class \n");
}

// getMsgBusConfig of ServerCfg class
config_t* ServerCfg::getMsgBusConfig(){

    fprintf(stderr,"in getMsgBusConfig method \n");
  
    config_value_t* server_type = config_value_object_get(server_cfg, "Type");
    
    char* type = server_type->body.string;
    cJSON* json = cJSON_CreateObject(); 
    cJSON_AddStringToObject(json, "type", type);
    config_value_t* server_json_endpoint = config_value_object_get(server_cfg, "EndPoint");
    char* EndPoint = server_json_endpoint->body.string;
    bool dev_mode = true;
    if(!strcmp(type, "zmq_tcp")) {
        
        config_value_t* allowed_clients = config_value_object_get(server_cfg, "AllowedClients");

        cJSON* server_topic = cJSON_CreateObject();
        cJSON* all_clients = cJSON_CreateArray();

        config_value_t* array_value; 

        for (int i =0; i < config_value_array_len(allowed_clients); i++){
            array_value = config_value_array_get(allowed_clients, i);
            cJSON_AddItemToArray(all_clients, cJSON_CreateString(array_value->body.string));
        }

        

        config_value_t* name = config_value_object_get(server_cfg, "Name");
        
        char* ser_name = name->body.string;
        cJSON_AddStringToObject(server_topic, "host", "127.0.0.1");
        cJSON_AddNumberToObject(server_topic, "port", 8675);
        if(!dev_mode) {
            cJSON_AddItemToObject(json, "allowed_clients",  all_clients);
            cJSON_AddStringToObject(server_topic, "server_secret_key", "Appname_secret_key");
            
        }
        cJSON_AddItemToObject(json, ser_name,  server_topic);
    }
    char* config_value = cJSON_Print(json);
    fprintf(stderr,"Env publish Config is : %s \n", config_value);

    config = config_new(
            (void*) json, free_json, get_config_value);
    if (config == NULL) {
        LOG_ERROR_0("Failed to initialize configuration object");
        return NULL;
    }

    return config;
}

ServerCfg::~ServerCfg() {
    // if(m_app_name) {
    //     delete m_app_name;
    // }
    // Stop the thread (if it is running)
    LOG_INFO_0("ServerCfg destructor");
}