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


#include "eis/config_manager/c_server_cfg.h"
#include <stdarg.h>

#define MAX_CONFIG_KEY_LENGTH 250

// To fetch endpoint from config
char* get_endpoint_server(base_cfg_t* base_cfg) {
    config_value_t* serv_config = base_cfg->pub_sub_config;
    // Fetching EndPoint from config
    config_value_t* end_point = config_value_object_get(serv_config, "EndPoint");
    char* ep = end_point->body.string;
    return ep;
}

// To fetch list of allowed clients from config
char** get_allowed_clients_server(base_cfg_t* base_cfg) {
    config_value_t* serv_config = base_cfg->pub_sub_config;
    // Fetching AllowedClients from config
    config_value_t* list_of_allowed_clients = config_value_object_get(serv_config, "AllowedClients");
    config_value_t* value;
    char **client_list = calloc(config_value_array_len(list_of_allowed_clients), sizeof(char*));
    for (int i =0; i < config_value_array_len(list_of_allowed_clients); i++) {
        value = config_value_array_get(list_of_allowed_clients, i);
        client_list[i] = value->body.string;
    }
    return client_list;
}

// To fetch msgbus config
config_t* get_msgbus_config_server(base_cfg_t* base_cfg) {

    // Initializing base_cfg variables
    config_value_t* serv_config = base_cfg->pub_sub_config;
    char* app_name = base_cfg->app_name;
    int dev_mode = base_cfg->dev_mode;
    kv_store_client_t* m_kv_store_handle = base_cfg->m_kv_store_handle;

    // Creating cJSON object
    cJSON* c_json = cJSON_CreateObject();

    // Fetching Type from config
    config_value_t* server_type = config_value_object_get(serv_config, "Type");
    char* type = server_type->body.string;
    cJSON_AddStringToObject(c_json, "type", type);

    // Fetching EndPoint from config
    config_value_t* server_endpoint = config_value_object_get(serv_config, "EndPoint");
    const char* end_point = server_endpoint->body.string;

    if(!strcmp(type, "zmq_ipc")){
        // Add Endpoint directly to socket_dir if IPC mode
        cJSON_AddStringToObject(c_json, "socket_dir", end_point);
    } else if(!strcmp(type, "zmq_tcp")) {

        // Add host & port to zmq_tcp_publish cJSON object
        cJSON* echo_service = cJSON_CreateObject();
        char** host_port = get_host_port(end_point);
        char* host = host_port[0];
        trim(host);
        char* port = host_port[1];
        trim(port);
        __int64_t i_port = atoi(port);

        cJSON_AddStringToObject(echo_service, "host", host);
        cJSON_AddNumberToObject(echo_service, "port", i_port);

        if(dev_mode != 0) {

            // Initializing m_kv_store_handle to fetch public & private keys
            void *handle = m_kv_store_handle->init(m_kv_store_handle);

            // Fetching AllowedClients from config
            config_value_t* server_json_clients = config_value_object_get(serv_config, "AllowedClients");
            config_value_t* array_value;
            cJSON* all_clients = cJSON_CreateArray();
            for (int i =0; i < config_value_array_len(server_json_clients); i++) {
                // Fetching individual public keys of all AllowedClients
                array_value = config_value_array_get(server_json_clients, i);
                size_t init_len = strlen("/Publickeys/") + strlen(array_value->body.string) + 2;
                char* grab_public_key = concat_s(init_len, 2, "/Publickeys/", array_value->body.string);
                const char* sub_public_key = m_kv_store_handle->get(handle, grab_public_key);
                cJSON_AddItemToArray(all_clients, cJSON_CreateString(sub_public_key));
            }

            // Adding all public keys of clients to allowed_clients of config
            cJSON_AddItemToObject(c_json, "allowed_clients",  all_clients);

            // Fetching Publisher private key & adding it to echo_service object
            size_t init_len = strlen("/") + strlen("/private_key") + strlen(app_name) + 2;
            char* pub_pri_key = concat_s(init_len, 3, "/", app_name, "/private_key");
            const char* server_secret_key = m_kv_store_handle->get(handle, pub_pri_key);
            cJSON_AddStringToObject(echo_service, "server_secret_key", server_secret_key);
        }
        // Creating the final cJSON config object
        cJSON_AddItemToObject(c_json, "echo_service", echo_service);
    }

    // Constructing char* object from cJSON object
    char* config_value_cr = cJSON_Print(c_json);
    LOG_DEBUG("Env server Config is : %s \n", config_value_cr);

    // Constructing config_t object from cJSON object
    config_t* m_config = config_new(
            (void*) c_json, free_json, get_config_value);
    if (m_config == NULL) {
        LOG_ERROR_0("Failed to initialize configuration object");
        return NULL;
    }
    return m_config;
}

// function to initialize server_cfg_t
server_cfg_t* server_cfg_new() {
    LOG_DEBUG_0("In server_cfg_new mthod");
    server_cfg_t *serv_cfg_mgr = (server_cfg_t *)malloc(sizeof(server_cfg_t));
    if(serv_cfg_mgr == NULL) {
        LOG_ERROR_0("Malloc failed for pub_cfg_t");
        return NULL;
    }
    serv_cfg_mgr->get_msgbus_config_server = get_msgbus_config_server;
    serv_cfg_mgr->get_endpoint_server = get_endpoint_server;
    serv_cfg_mgr->get_allowed_clients_server = get_allowed_clients_server;
    return serv_cfg_mgr;
}

// function to destroy server_cfg_t
void server_cfg_config_destroy(server_cfg_t *server_cfg_config) {
    if(server_cfg_config != NULL) {
        free(server_cfg_config);
    }
}