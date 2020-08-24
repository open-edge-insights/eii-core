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
 * @brief ServerCfg Implementation
 * Holds the implementaion of APIs supported by ServerCfg class
 */


#include "eis/config_manager/server_cfg.h"

using namespace eis::config_manager;

// Constructor
ServerCfg::ServerCfg(config_value_t* server_config):AppCfg(NULL, NULL, NULL) {
    m_server_cfg = server_config;
}

// getMsgBusConfig of ServerCfg class
config_t* ServerCfg::getMsgBusConfig() {
  
    // Creating cJSON object
    cJSON* c_json = cJSON_CreateObject();

    // Fetching Type from config
    config_value_t* server_type = config_value_object_get(m_server_cfg, "Type");
    char* type = server_type->body.string;
    cJSON_AddStringToObject(c_json, "type", type);

    // Fetching EndPoint from config
    config_value_t* server_endpoint = config_value_object_get(m_server_cfg, "EndPoint");
    const char* end_point = server_endpoint->body.string;

    if(!strcmp(type, "zmq_ipc")){
        // Add Endpoint directly to socket_dir if IPC mode
        cJSON_AddStringToObject(c_json, "socket_dir", end_point);
    } else if(!strcmp(type, "zmq_tcp")) {

        // TCP DEV mode
        // Add host & port to zmq_tcp_publish cJSON object
        cJSON* echo_service = cJSON_CreateObject();
        std::vector<std::string> tokens = AppCfg::tokenizer(end_point, ":");
        cJSON_AddStringToObject(echo_service, "host", tokens[0].c_str());
        cJSON_AddNumberToObject(echo_service, "port", atoi(tokens[1].c_str()));

        if(!m_dev_mode) {

            // Initializing db_client handle to fetch public & private keys
            void *handle = m_kv_store_client_handle->init(m_kv_store_client_handle);

            // Fetching AllowedClients from config
            config_value_t* server_json_clients = config_value_object_get(m_server_cfg, "AllowedClients");
            config_value_t* array_value;
            cJSON* all_clients = cJSON_CreateArray();
            for (int i =0; i < config_value_array_len(server_json_clients); i++) {
                // Fetching individual public keys of all AllowedClients
                array_value = config_value_array_get(server_json_clients, i);
                std::string sub_app_name(array_value->body.string);
                std::string grab_public_key = "/Publickeys/" + sub_app_name;
                const char* sub_public_key = m_kv_store_client_handle->get(handle, &grab_public_key[0]);
                cJSON_AddItemToArray(all_clients, cJSON_CreateString(sub_public_key));
            }

            // Adding all public keys of clients to allowed_clients of config
            cJSON_AddItemToObject(c_json, "allowed_clients",  all_clients);

            // Fetching Publisher private key & adding it to zmq_tcp_publish object
            std::string pub_pri_key = "/" + m_app_name + "/private_key";
            const char* server_secret_key = m_kv_store_client_handle->get(handle, &pub_pri_key[0]);
            cJSON_AddStringToObject(echo_service, "server_secret_key", server_secret_key);
        }
        // Creating the final cJSON config object
        cJSON_AddItemToObject(c_json, "echo_service", echo_service);
    }

    // Constructing char* object from cJSON object
    char* config_value_cr = cJSON_Print(c_json);
    LOG_DEBUG("Env server Config is : %s \n", config_value_cr);

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
std::string ServerCfg::getEndpoint() {
    // Fetching EndPoint from config
    config_value_t* endpoint = config_value_object_get(m_server_cfg, "EndPoint");
    char* type = endpoint->body.string;
    std::string s(type);
    return s;
}

// To fetch list of allowed clients from config
std::vector<std::string> ServerCfg::getAllowedClients() {
    // Fetching AllowedClients from config
    config_value_t* list_of_allowed_clients = config_value_object_get(m_server_cfg, "AllowedClients");
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
ServerCfg::~ServerCfg() {
    if(m_config) {
        delete m_config;
    }
    if(m_server_cfg) {
        delete m_server_cfg;
    }
    LOG_INFO_0("ServerCfg destructor");
}