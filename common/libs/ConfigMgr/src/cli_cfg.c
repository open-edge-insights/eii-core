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
 * @brief ConfigMgr C Client Implementation
 * Holds the implementaion of APIs supported by ConfigMgr Client
 */


#include "eis/config_manager/c_client_cfg.h"
#include <stdarg.h>

#define MAX_CONFIG_KEY_LENGTH 250

// To fetch endpoint from config
char* get_endpoint_client(base_cfg_t* base_cfg) {
    config_value_t* cli_config = base_cfg->pub_sub_config;
    // Fetching EndPoint from config
    config_value_t* end_point = config_value_object_get(cli_config, "EndPoint");
    char* ep = end_point->body.string;
    return ep;
}

// function to get msgbus config for client
config_t* get_msgbus_config_client(base_cfg_t* base_cfg) {

    // Initializing base_cfg variables
    config_value_t* cli_config = base_cfg->pub_sub_config;
    char* app_name = base_cfg->app_name;
    int dev_mode = base_cfg->dev_mode;
    kv_store_client_t* m_kv_store_handle = base_cfg->m_kv_store_handle;

    // Creating cJSON object
    cJSON* c_json = cJSON_CreateObject();

    // Fetching Type from config
    config_value_t* client_type = config_value_object_get(cli_config, "Type");
    char* type = client_type->body.string;
    cJSON_AddStringToObject(c_json, "type", type);

    // Fetching EndPoint from config
    config_value_t* client_endpoint = config_value_object_get(cli_config, "EndPoint");
    const char* end_point = client_endpoint->body.string;

    if(!strcmp(type, "zmq_ipc")){
        // Add Endpoint directly to socket_dir if IPC mode
        cJSON_AddStringToObject(c_json, "socket_dir", end_point);
    } else if(!strcmp(type, "zmq_tcp")) {

        // Add host & port to echo_service cJSON object
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

             // Fetching Publisher AppName from config
            config_value_t* server_appname = config_value_object_get(cli_config, "AppName");

            // Adding server public key to config
            size_t init_len = strlen("/Publickeys/") + strlen(server_appname->body.string) + 2;
            char* retreive_server_pub_key = concat_s(init_len, 2, "/Publickeys/", server_appname->body.string);
            const char* server_public_key = m_kv_store_handle->get(handle, retreive_server_pub_key);
            if(server_public_key == NULL){
                LOG_ERROR("Value is not found for the key: %s", retreive_server_pub_key);
            }

            cJSON_AddStringToObject(echo_service, "server_public_key", server_public_key);

            // Adding client public key to config
            init_len = strlen("/Publickeys/") + strlen(app_name) + strlen(app_name) + 2;
            char* s_client_public_key = concat_s(init_len, 2, "/Publickeys/", app_name);
            const char* sub_public_key = m_kv_store_handle->get(handle, s_client_public_key);
            if(sub_public_key == NULL){
                LOG_ERROR("Value is not found for the key: %s", s_client_public_key);
            }

            cJSON_AddStringToObject(echo_service, "client_public_key", sub_public_key);

            // Adding client private key to config
            init_len = strlen("/") + strlen(app_name) + strlen("/private_key") + 2;
            char* s_client_pri_key = concat_s(init_len, 3, "/", app_name, "/private_key");
            const char* sub_pri_key = m_kv_store_handle->get(handle, s_client_pri_key);
            if(sub_pri_key == NULL){
                LOG_ERROR("Value is not found for the key: %s", s_client_pri_key);
            }

            cJSON_AddStringToObject(echo_service, "client_secret_key", sub_pri_key);
        }
        // Creating the final cJSON config object
        cJSON_AddItemToObject(c_json, "echo_service", echo_service);
    }

    char* config_value = cJSON_Print(c_json);
    LOG_DEBUG("Env client Config is : %s \n", config_value);

    config_t* m_config = config_new(
            (void*) c_json, free_json, get_config_value);
    if (m_config == NULL) {
        LOG_ERROR_0("Failed to initialize configuration object");
        return NULL;
    }

    return m_config;
}

// function to initialize client_cfg_t
client_cfg_t* client_cfg_new() {
    LOG_DEBUG_0("In client_cfg_new mthod");
    client_cfg_t *cli_cfg_mgr = (client_cfg_t *)malloc(sizeof(client_cfg_t));
    if(cli_cfg_mgr == NULL) {
        LOG_ERROR_0("Malloc failed for client_cfg_t");
        return NULL;
    }
    cli_cfg_mgr->get_msgbus_config_client = get_msgbus_config_client;
    cli_cfg_mgr->get_endpoint_client = get_endpoint_client;
    return cli_cfg_mgr;
}

// function to destroy client_cfg_t
void client_cfg_config_destroy(client_cfg_t *cli_cfg_config) {
    if(cli_cfg_config != NULL) {
        free(cli_cfg_config);
    }
}