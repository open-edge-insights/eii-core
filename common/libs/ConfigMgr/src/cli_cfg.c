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


#include "eii/config_manager/client_cfg.h"
#include "eii/config_manager/util_cfg.h"
#include <stdarg.h>

#define MAX_CONFIG_KEY_LENGTH 250

// To fetch endpoint from config
config_value_t* cfgmgr_get_endpoint_client(void* cli_conf) {
    client_cfg_t* client_cfg = (client_cfg_t*) cli_conf;
    config_value_t* ep = get_endpoint_base(client_cfg->client_config);
    if (ep == NULL) {
        LOG_ERROR_0("Endpoint not found");
        return NULL;
    }
    return ep;
}

config_value_t* cfgmgr_get_interface_value_client(void* cli_conf, const char* key) {
    client_cfg_t* client_cfg = (client_cfg_t*)cli_conf;
    config_value_t* interface_value = config_value_object_get(client_cfg->client_config, key);
    if (interface_value == NULL){
        LOG_DEBUG_0("interface_value initialization failed");
        return NULL;
    }
    return interface_value;
}

// function to get msgbus config for client
config_t* cfgmgr_get_msgbus_config_client(base_cfg_t* base_cfg, void* cli_conf) {

    client_cfg_t* client_cfg = (client_cfg_t*)cli_conf;
    // Initializing base_cfg variables
    config_value_t* cli_config = client_cfg->client_config;
    char* app_name = base_cfg->app_name;
    int dev_mode = base_cfg->dev_mode;
    kv_store_client_t* m_kv_store_handle = base_cfg->m_kv_store_handle;
    void* cfgmgr_handle = base_cfg->cfgmgr_handle;
    config_t* m_config = NULL;
    config_value_t* server_appname = NULL;
    config_value_t* zmq_recv_hwm_value = NULL;
    config_value_t* client_endpoint = NULL;
    config_value_t* client_config_type = NULL;
    char** host_port = NULL;
    char* host = NULL;
    char* port = NULL;
    char* s_client_pri_key = NULL;
    char* s_client_public_key = NULL;
    char* retreive_server_pub_key = NULL;
    char* sub_public_key = NULL;
    char* sub_pri_key = NULL;
    char* type_override_env = NULL;
    char* type_override = NULL;
    char* ep_override_env = NULL;
    char* config_value = NULL;

    // Creating cJSON object
    cJSON* c_json = cJSON_CreateObject();
    if (c_json == NULL) {
        LOG_ERROR_0("c_json object initialization failed");
        return NULL;
    }

    // Fetching name from config
    config_value_t* client_name = config_value_object_get(cli_config, NAME);
    if (client_name == NULL || client_name->body.string == NULL) {
        LOG_ERROR_0("client_name initialization failed");
        goto err;
    }

    // Fetching Type from config
    client_config_type = config_value_object_get(cli_config, TYPE);
    if (client_config_type == NULL || client_config_type->body.string == NULL) {
        LOG_ERROR_0("client_config_type object initialization failed");
        goto err;
    }

    if(client_config_type->type != CVT_STRING || client_config_type->body.string == NULL){
        LOG_ERROR_0("client_config_type type mismatch or the string value is NULL");
        goto err;
    }

    char* type = client_config_type->body.string;

    // Fetching EndPoint from config
    client_endpoint = config_value_object_get(cli_config, ENDPOINT);
    if (client_endpoint == NULL) {
        LOG_ERROR_0("client_endpoint object initialization failed");
        goto err;
    }
    const char* end_point = client_endpoint->body.string;

    // Overriding endpoint with CLIENT_<Name>_ENDPOINT if set
    size_t init_len = strlen("CLIENT_") + strlen(client_name->body.string) + strlen("_ENDPOINT") + 2;
    ep_override_env = concat_s(init_len, 3, "CLIENT_", client_name->body.string, "_ENDPOINT");
    if (ep_override_env == NULL) {
        LOG_ERROR_0("concatenation for ep_override_env failed");
        goto err;
    }
    char* ep_override = getenv(ep_override_env);
    if (ep_override != NULL) {
        if (strlen(ep_override) != 0) {
            LOG_DEBUG("Overriding endpoint with %s", ep_override_env);
            end_point = (const char*)ep_override;
        }
    } else {
        LOG_DEBUG("env not set for overridding %s, and hence endpoint taking from interface ", ep_override_env);
    }

    // Overriding endpoint with CLIENT_ENDPOINT if set
    // Note: This overrides all the client endpoints if set
    char* client_ep = getenv("CLIENT_ENDPOINT");
    if (client_ep != NULL) {
        LOG_DEBUG_0("Overriding endpoint with CLIENT_ENDPOINT");
        if (strlen(client_ep) != 0) {
            end_point = (const char*)client_ep;
        }
    } else {
        LOG_DEBUG_0("env not set for overridding CLIENT_ENDPOINT, and hence endpoint taking from interface ");
    }

    // Overriding endpoint with CLIENT_<Name>_TYPE if set
    init_len = strlen("CLIENT_") + strlen(client_name->body.string) + strlen("_TYPE") + 2;
    type_override_env = concat_s(init_len, 3, "CLIENT_", client_name->body.string, "_TYPE");
    if (type_override_env == NULL) {
        LOG_ERROR_0("concatenation for type_override_env failed");
        goto err;
    }
    type_override = getenv(type_override_env);
    if (type_override != NULL) {
        if (strlen(type_override) != 0) {
            LOG_DEBUG("Overriding endpoint with %s", type_override_env);
            type = type_override;
        }
    } else {
        LOG_DEBUG("env not set for overridding %s, and hence type taking from interface ", type_override_env);
    }

    // Overriding endpoint with CLIENT_TYPE if set
    // Note: This overrides all the client type if set
    char* client_type = getenv("CLIENT_TYPE");
    if (client_type != NULL) {
        LOG_DEBUG_0("Overriding endpoint with CLIENT_TYPE");
        if (strlen(client_type) != 0) {
            type = client_type;
        }
    } else {
        LOG_DEBUG("env not set for overridding CLIENT_TYPE, and hence type taking from interface ");
    }
    cJSON_AddStringToObject(c_json, "type", type);

    // Adding zmq_recv_hwm value if available
    zmq_recv_hwm_value = config_value_object_get(cli_config, ZMQ_RECV_HWM);
    if (zmq_recv_hwm_value != NULL) {
        if (zmq_recv_hwm_value->type != CVT_INTEGER) {
            LOG_ERROR_0("zmq_recv_hwm type is not integer");
            goto err;
        }
        cJSON_AddNumberToObject(c_json, ZMQ_RECV_HWM, zmq_recv_hwm_value->body.integer);
    }

    if (!strcmp(type, "zmq_ipc")) {
        bool ret = get_ipc_config(c_json, cli_config, end_point, CLIENT);
        if (ret == false){
            LOG_ERROR_0("IPC configuration for client failed");
            return NULL;
        }
    } else if (!strcmp(type, "zmq_tcp")) {

        // Add host & port to client_topic cJSON object
        cJSON* client_topic = cJSON_CreateObject();
        if (client_topic == NULL) {
            LOG_ERROR_0("client_topic object initialization failed");
            goto err;
        }
        host_port = get_host_port(end_point);
        if (host_port == NULL){
            LOG_ERROR_0("Get host and port failed");
            goto err;
        }
        host = host_port[0];
        trim(host);
        port = host_port[1];
        trim(port);
        __int64_t i_port = atoi(port);

        cJSON_AddStringToObject(client_topic, "host", host);
        cJSON_AddNumberToObject(client_topic, "port", i_port);

        if(dev_mode != 0) {

            // Fetching Server AppName from config
            server_appname = config_value_object_get(cli_config, SERVER_APPNAME);
            if (server_appname == NULL) {
                LOG_ERROR_0("server_appname object initialization failed");
                goto err;
            }

            if (server_appname->type != CVT_STRING || server_appname->body.string == NULL) {
                LOG_ERROR_0("server_appname type miss match, it should be string or srting value is NULL");
                goto err;
            }

            // Adding server public key to config
            size_t init_len = strlen(PUBLIC_KEYS) + strlen(server_appname->body.string) + 2;
            retreive_server_pub_key = concat_s(init_len, 2, PUBLIC_KEYS, server_appname->body.string);
            if (retreive_server_pub_key == NULL) {
                LOG_ERROR_0("concatenation PUBLIC_KEYS and server_appname string failed");
                goto err;
            }
            char* server_public_key = m_kv_store_handle->get(cfgmgr_handle, retreive_server_pub_key);
            if(server_public_key == NULL){
                LOG_DEBUG("Value is not found for the key: %s", retreive_server_pub_key);
            }

            cJSON_AddStringToObject(client_topic, "server_public_key", server_public_key);

            // free server_pub_key
            if (server_public_key != NULL) {
                free(server_public_key);
            }

            // Adding client public key to config
            init_len = strlen(PUBLIC_KEYS) + strlen(app_name) + strlen(app_name) + 2;
            s_client_public_key = concat_s(init_len, 2, PUBLIC_KEYS, app_name);
            if (s_client_public_key == NULL) {
                LOG_ERROR_0("concatenation PUBLIC_KEYS and appname string failed");
                goto err;
            }
            sub_public_key = m_kv_store_handle->get(cfgmgr_handle, s_client_public_key);
            if(sub_public_key == NULL){
                LOG_ERROR("Value is not found for the key: %s", s_client_public_key);
                goto err;
            }

            cJSON_AddStringToObject(client_topic, "client_public_key", sub_public_key);

            // Adding client private key to config
            init_len = strlen("/") + strlen(app_name) + strlen(PRIVATE_KEY) + 2;
            s_client_pri_key = concat_s(init_len, 3, "/", app_name, PRIVATE_KEY);
            if (s_client_pri_key == NULL) {
                LOG_ERROR_0("concatenation PRIVATE_KEY and appname string failed");
                goto err;
            }
            sub_pri_key = m_kv_store_handle->get(cfgmgr_handle, s_client_pri_key);
            if(sub_pri_key == NULL){
                LOG_ERROR("Value is not found for the key: %s", s_client_pri_key);
                goto err;
            }
            
            cJSON_AddStringToObject(client_topic, "client_secret_key", sub_pri_key);
        }
        // Creating the final cJSON config object
        cJSON_AddItemToObject(c_json, client_name->body.string, client_topic);
    } else {
        LOG_ERROR_0("Type should be either \"zmq_ipc\" or \"zmq_tcp\"");
        goto err;
    }

    config_value = cJSON_Print(c_json);
    if (config_value == NULL) {
        LOG_ERROR_0("config_value object initialization failed");
        goto err;
    }
    LOG_DEBUG("Env client Config is : %s \n", config_value);

    m_config = config_new(
            (void*) c_json, free_json, get_config_value);
    if (m_config == NULL) {
        LOG_ERROR_0("Failed to initialize configuration object");
    }

err:
    if (s_client_pri_key != NULL) {
        free(s_client_pri_key);
    }
    if (s_client_public_key != NULL) {
        free(s_client_public_key);
    }
    if (retreive_server_pub_key != NULL) {
        free(retreive_server_pub_key);
    }
    if (sub_public_key != NULL) {
        free(sub_public_key);
    }
    if (sub_pri_key != NULL) {
        free(sub_pri_key);
    }
    if (config_value != NULL) {
        free(config_value);
    }
    if (host_port != NULL) {
        free_mem(host_port);
    }
    if (client_config_type != NULL) {
        config_value_destroy(client_config_type);
    }
    if (client_endpoint  != NULL) {
        config_value_destroy(client_endpoint);
    }
    if (client_name != NULL) {
        config_value_destroy(client_name);
    }
    if (ep_override_env != NULL) {
        free(ep_override_env);
    }
    if (type_override_env != NULL) {
        free(type_override_env);
    }
    if (zmq_recv_hwm_value != NULL) {
        config_value_destroy(zmq_recv_hwm_value);
    }
    if (server_appname != NULL) {
        config_value_destroy(server_appname);
    }
    return m_config;
}

// function to initialize client_cfg_t
client_cfg_t* client_cfg_new() {
    LOG_DEBUG_0("In client_cfg_new mthod");
    client_cfg_t *cli_cfg_mgr = (client_cfg_t *)malloc(sizeof(client_cfg_t));
    if (cli_cfg_mgr == NULL) {
        LOG_ERROR_0("Malloc failed for client_cfg_t");
        return NULL;
    }
    cli_cfg_mgr->cfgmgr_get_msgbus_config_client = cfgmgr_get_msgbus_config_client;
    cli_cfg_mgr->cfgmgr_get_interface_value_client = cfgmgr_get_interface_value_client;
    cli_cfg_mgr->cfgmgr_get_endpoint_client = cfgmgr_get_endpoint_client;
    return cli_cfg_mgr;
}

// function to destroy client_cfg_t
void client_cfg_config_destroy(client_cfg_t *cli_cfg_config) {
    if (cli_cfg_config != NULL) {
        if (cli_cfg_config->client_config != NULL) {
            config_value_destroy(cli_cfg_config->client_config);
        }
        free(cli_cfg_config);
    }
}