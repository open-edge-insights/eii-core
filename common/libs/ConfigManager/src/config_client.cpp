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

#include "eis/config_manager/config_client.h"

using namespace eis::config_manager;

ClientCfg::ClientCfg(config_value_t* client_config):ConfigHandler(NULL, NULL, NULL) {
    client_cfg = client_config;
    fprintf(stderr,"in PublisherCfg class \n");
}

// getMsgBusConfig of ClientCfg class
config_t* ClientCfg::getMsgBusConfig(){
    config_value_t* clientappname = config_value_object_get(client_cfg, "AppName");
    char* appname_ = clientappname->body.string;

    config_value_t* client_json_type = config_value_object_get(client_cfg, "Type");

    char* type = client_json_type->body.string;
    cJSON* json = cJSON_CreateObject(); 
    cJSON_AddStringToObject(json, "type", type);

    config_value_t* client_json_endpoint = config_value_object_get(client_cfg, "EndPoint");
    char* EndPoint = client_json_endpoint->body.string;
    bool dev_mode = true;

    if(!strcmp(type, "zmq_tcp")){

        cJSON* client_app = cJSON_CreateObject();
        cJSON_AddStringToObject(client_app, "host", "127.0.0.1");
        cJSON_AddNumberToObject(client_app, "port", 8675);
        if(!dev_mode) {
            cJSON_AddStringToObject(client_app, "server_secret_key", "appname_ privatekey");
            cJSON_AddStringToObject(client_app, "client_secret_key", "client app privatekey");
            cJSON_AddStringToObject(client_app, "client_public_key", "client app publickey");
        }
        cJSON_AddItemToObject(json, appname_, client_app);
        
    }

    char* config_value = cJSON_Print(json);
    fprintf(stderr,"Env Config client is : %s \n", config_value);

    config = config_new(
            (void*) json, free_json, get_config_value);
    if (config == NULL) {
        LOG_ERROR_0("Failed to initialize configuration object");
        return NULL;
    }

    return config;
}

ClientCfg::~ClientCfg() {
    // if(m_app_name) {
    //     delete m_app_name;
    // }
    // Stop the thread (if it is running)
    LOG_INFO_0("ClientCfg destructor");
}