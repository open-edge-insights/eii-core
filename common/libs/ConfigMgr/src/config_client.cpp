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
 * @brief ClientCfg Implementation
 * Holds the implementaion of APIs supported by ClientCfg class
 */

#include "eis/config_manager/config_client.h"

using namespace eis::config_manager;

// Constructor
ClientCfg::ClientCfg(config_value_t* client_config):AppCfg(NULL, NULL, NULL) {
    client_cfg = client_config;
}

// getMsgBusConfig of ClientCfg class
config_t* ClientCfg::getMsgBusConfig(){

    // Creating cJSON object
    cJSON* c_json = cJSON_CreateObject();

    // Fetching Type from config
    config_value_t* client_type = config_value_object_get(client_cfg, "Type");
    char* type = client_type->body.string;
    cJSON_AddStringToObject(c_json, "type", type);

    // Fetching EndPoint from config
    config_value_t* client_endpoint = config_value_object_get(client_cfg, "EndPoint");
    const char* end_point = client_endpoint->body.string;

    if(!strcmp(type, "zmq_ipc")){
        // Add Endpoint directly to socket_dir if IPC mode
        cJSON_AddStringToObject(c_json, "socket_dir", end_point);
    } else if(!strcmp(type, "zmq_tcp")) {

        // TCP DEV mode
        // Add host & port to echo_service cJSON object
        cJSON* echo_service = cJSON_CreateObject();
        std::vector<std::string> tokens = AppCfg::tokenizer(end_point, ":");
        cJSON_AddStringToObject(echo_service, "host", tokens[0].c_str());
        cJSON_AddNumberToObject(echo_service, "port", atoi(tokens[1].c_str()));

        if(!m_dev_mode) {

            // Initializing db_client handle to fetch public & private keys
            void *handle = m_db_client_handle->init(m_db_client_handle);

             // Fetching Publisher AppName from config
            config_value_t* server_appname = config_value_object_get(client_cfg, "AppName");
            std::string pub_app_name(server_appname->body.string);

            // Adding server public key to config
            std::string retreive_server_pub_key = "/Publickeys/" + pub_app_name;
            const char* server_public_key = m_db_client_handle->get(handle, &retreive_server_pub_key[0]);
            cJSON_AddStringToObject(echo_service, "server_public_key", server_public_key);

            // Adding client public key to config
            std::string s_client_public_key = "/Publickeys/" + m_app_name;
            const char* sub_public_key = m_db_client_handle->get(handle, &s_client_public_key[0]);
            cJSON_AddStringToObject(echo_service, "client_public_key", sub_public_key);

            // Adding client private key to config
            std::string s_client_pri_key = "/" + m_app_name + "/private_key";
            const char* sub_pri_key = m_db_client_handle->get(handle, &s_client_pri_key[0]);
            cJSON_AddStringToObject(echo_service, "client_secret_key", sub_pri_key);
        }
        // Creating the final cJSON config object
        cJSON_AddItemToObject(c_json, "echo_service", echo_service);
    }

    char* config_value = cJSON_Print(c_json);
    LOG_DEBUG("Env client Config is : %s \n", config_value);

    config = config_new(
            (void*) c_json, free_json, get_config_value);
    if (config == NULL) {
        LOG_ERROR_0("Failed to initialize configuration object");
        return NULL;
    }

    return config;
}

// To fetch endpoint from config
std::string ClientCfg::getEndpoint() {
    config_value_t* endpoint = config_value_object_get(client_cfg, "EndPoint");
    char* type = endpoint->body.string;
    std::string s(type);
    return s;
}

// Destructor
ClientCfg::~ClientCfg() {
    if(config) {
        delete config;
    }
    if(client_cfg) {
        delete client_cfg;
    }
    LOG_INFO_0("ClientCfg destructor");
}