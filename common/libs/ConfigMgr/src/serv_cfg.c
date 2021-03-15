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


#include "eii/config_manager/server_cfg.h"
#include <stdarg.h>
#include "eii/config_manager/util_cfg.h"

#define MAX_CONFIG_KEY_LENGTH 250

// To fetch endpoint from config
config_value_t* cfgmgr_get_endpoint_server(void* server_conf) {
    server_cfg_t* server_cfg = (server_cfg_t*)server_conf;
    config_value_t* ep = get_endpoint_base(server_cfg->server_config);
    if (ep == NULL) {
        LOG_ERROR_0("Endpoint not found");
        return NULL;
    }
    return ep;
}

// To fetch list of allowed clients from config
config_value_t* cfgmgr_get_allowed_clients_server(void* server_conf) {
    server_cfg_t* server_cfg = (server_cfg_t*)server_conf;
    config_value_t* client_list = get_allowed_clients_base(server_cfg->server_config);
    if (client_list == NULL) {
        LOG_ERROR_0("client_list initialization failed");
        return NULL;
    }
    return client_list;
}

config_value_t* cfgmgr_get_interface_value_server(void* serv_conf, const char* key) {
    server_cfg_t* server_cfg = (server_cfg_t*)serv_conf;
    config_value_t* interface_value = config_value_object_get(server_cfg->server_config, key);
    if (interface_value == NULL){
        LOG_DEBUG_0("interface_value initialization failed");
        return NULL;
    }
    return interface_value;
}

// To fetch msgbus config
config_t* cfgmgr_get_msgbus_config_server(base_cfg_t* base_cfg, void* server_conf) {

    server_cfg_t* server_cfg = (server_cfg_t*)server_conf;
    // Initializing base_cfg variables
    config_value_t* serv_config = server_cfg->server_config;
    char* app_name = base_cfg->app_name;
    int dev_mode = base_cfg->dev_mode;
    kv_store_client_t* m_kv_store_handle = base_cfg->m_kv_store_handle;
    void* cfgmgr_handle = base_cfg->cfgmgr_handle;
    config_t* ret = NULL;
    config_value_t* temp_array_value = NULL;
    config_value_t* server_name = NULL;
    config_value_t* server_config_type = NULL;
    char* ep_override_env = NULL;
    char** host_port = NULL;
    char* type_override_env = NULL;
    config_value_t* server_json_clients = NULL;
    config_value_t* pub_key_values = NULL;
    char* pub_pri_key = NULL;
    char* server_secret_key = NULL;
    config_value_t* server_endpoint = NULL;
    char* grab_public_key = NULL;
    char* client_public_key = NULL;
    char* config_value_cr = NULL;

    // Creating cJSON object
    cJSON* c_json = cJSON_CreateObject();
    if (c_json == NULL) {
        LOG_ERROR_0("c_json initialization failed");
        goto err;
    }

    // Fetching Name from name
    server_name = config_value_object_get(serv_config, NAME);
    if (server_name == NULL) {
        LOG_ERROR_0("server_name initialization failed");
        goto err;
    }

    // Fetching Type from config
    server_config_type = config_value_object_get(serv_config, TYPE);
    if (server_config_type == NULL || server_config_type->body.string == NULL) {
        LOG_ERROR_0("server_config_type initialization failed");
        goto err;
    }

    if(server_config_type->type != CVT_STRING || server_config_type->body.string == NULL){
        LOG_ERROR_0("server_config_type type mismatch or the string value is NULL");
        goto err;
    }

    char* type = server_config_type->body.string;

    // Fetching EndPoint from config
    server_endpoint = config_value_object_get(serv_config, ENDPOINT);
    if (server_endpoint == NULL || server_endpoint->body.string == NULL) {
        LOG_ERROR_0("server_endpoint initialization failed");
        goto err;
    }
    const char* end_point = server_endpoint->body.string;

    // // Overriding endpoint with SERVER_<Name>_ENDPOINT if set
    size_t init_len = strlen("SERVER_") + strlen(server_name->body.string) + strlen("_ENDPOINT") + 2;
    ep_override_env = concat_s(init_len, 3, "SERVER_", server_name->body.string, "_ENDPOINT");
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
        LOG_DEBUG_0("env not set for overridding endpoint, hence endpoint is taken from interface");
    }

    // Overriding endpoint with SERVER_ENDPOINT if set
    // Note: This overrides all the server endpoints if set
    char* server_ep = getenv("SERVER_ENDPOINT");
    if (server_ep != NULL) {
        LOG_DEBUG_0("Overriding endpoint with SERVER_ENDPOINT");
        if (strlen(server_ep) != 0) {
            end_point = (const char*)server_ep;
        }
    } else {
        LOG_DEBUG_0("env not set for overridding SERVER_ENDPOINT, and hence endpoint taking from interface ");
    }

    // Overriding endpoint with SERVER_<Name>_TYPE if set
    init_len = strlen("SERVER_") + strlen(server_name->body.string) + strlen("_TYPE") + 2;
    type_override_env = concat_s(init_len, 3, "SERVER_", server_name->body.string, "_TYPE");
    if (type_override_env == NULL) {
        LOG_ERROR_0("concatenation for type_override_env failed");
        goto err;
    }
    char* type_override = getenv(type_override_env);
    if (type_override != NULL) {
        if (strlen(type_override) != 0) {
            LOG_DEBUG("Overriding endpoint with %s", type_override_env);
            type = type_override;
        }
    } else {
        LOG_DEBUG_0("env not set for type overridding, and hence type is taken from interface");
    }

    // Overriding endpoint with SERVER_TYPE if set
    // Note: This overrides all the server type if set
    char* server_type = getenv("SERVER_TYPE");
    if (server_type != NULL) {
        LOG_DEBUG_0("Overriding endpoint with SERVER_TYPE");
        if (strlen(server_type) != 0) {
            type = server_type;
        }
    }  else {
        LOG_DEBUG_0("env not set for overridding SERVER_TYPE, and hence type taking from interface ");
    }
    cJSON_AddStringToObject(c_json, "type", type);

    // Adding zmq_recv_hwm value if available
    config_value_t* zmq_recv_hwm_value = config_value_object_get(serv_config, ZMQ_RECV_HWM);
    if (zmq_recv_hwm_value != NULL) {
        if (zmq_recv_hwm_value->type != CVT_INTEGER) {
            LOG_ERROR_0("zmq_recv_hwm type is not integer");
            goto err;
        }
        cJSON_AddNumberToObject(c_json, ZMQ_RECV_HWM, zmq_recv_hwm_value->body.integer);
    }

    if (!strcmp(type, "zmq_ipc")) {
        bool ret = get_ipc_config(c_json, serv_config, end_point, SERVER);
        if (ret == false){
            LOG_ERROR_0("IPC configuration for server failed");
            goto err;
        }
    } else if (!strcmp(type, "zmq_tcp")) {
        // Add host & port to zmq_tcp_publish cJSON object
        cJSON* server_topic = cJSON_CreateObject();
        if (server_topic == NULL) {
            LOG_ERROR_0("server_topic initialization failed");
            goto err;
        }
        host_port = get_host_port(end_point);
        if (host_port == NULL){
            LOG_ERROR_0("Get host and port failed");
            goto err;
        }
        char* host = host_port[0];
        trim(host);
        char* port = host_port[1];
        trim(port);
        __int64_t i_port = atoi(port);

        cJSON_AddStringToObject(server_topic, "host", host);
        cJSON_AddNumberToObject(server_topic, "port", i_port);

        if (dev_mode != 0) {

            // Fetching AllowedClients from config
            server_json_clients = config_value_object_get(serv_config, ALLOWED_CLIENTS);
            if (server_json_clients == NULL) {
                LOG_ERROR_0("server_json_clients initialization failed");
                goto err;
            }

            // Checking if Allowed clients is empty string
            if (config_value_array_len(server_json_clients) == 0){
                LOG_ERROR_0("Empty String is not supported in AllowedClients. Atleast one allowed clients is required");
                goto err;
            }

            // Fetch the first item in allowed_clients
            temp_array_value = config_value_array_get(server_json_clients, 0);
            if (temp_array_value == NULL || temp_array_value->body.string == NULL) {
                LOG_ERROR_0("temp_array_value initialization failed");
                goto err;
            }
            int result;
            strcmp_s(temp_array_value->body.string, strlen(temp_array_value->body.string), "*", &result);
            // If only one item in allowed_clients and it is *
            // Add all available Publickeys
            if ((config_value_array_len(server_json_clients) == 1) && (result == 0)) {
                cJSON* all_clients = cJSON_CreateArray();
                if (all_clients == NULL) {
                    LOG_ERROR_0("all_clients initialization failed");
                    goto err;
                }
                pub_key_values = m_kv_store_handle->get_prefix(cfgmgr_handle, "/Publickeys/");
                if (pub_key_values == NULL) {
                    LOG_ERROR_0("pub_key_values initialization failed");
                    goto err;
                }
                config_value_t* value;
                size_t arr_len = config_value_array_len(pub_key_values);
                if(arr_len == 0){
                    LOG_ERROR_0("Empty array is not supported, atleast one value should be given.");
                    goto err;
                }

                for (int i = 0; i < arr_len; i++) {
                    value = config_value_array_get(pub_key_values, i);
                    if (value == NULL) {
                        LOG_ERROR_0("value initialization failed");
                        goto err;
                    }
                    cJSON_AddItemToArray(all_clients, cJSON_CreateString(value->body.string));
                    config_value_destroy(value);
                }
                cJSON_AddItemToObject(c_json, "allowed_clients",  all_clients);
            } else {
                config_value_t* array_value;
                cJSON* all_clients = cJSON_CreateArray();
                if (all_clients == NULL) {
                    LOG_ERROR_0("all_clients initialization failed");
                    goto err;
                }
                size_t arr_len = config_value_array_len(server_json_clients);
                if(arr_len == 0){
                    LOG_ERROR_0("Empty array is not supported, atleast one value should be given.");
                    goto err;
                }
                for (int i =0; i < arr_len; i++) {
                    // Fetching individual public keys of all AllowedClients
                    array_value = config_value_array_get(server_json_clients, i);
                    if (array_value == NULL) {
                        LOG_ERROR_0("array_value initialization failed");
                        goto err;
                    }
                    size_t init_len = strlen(PUBLIC_KEYS) + strlen(array_value->body.string) + 2;
                    char* grab_public_key = concat_s(init_len, 2, PUBLIC_KEYS, array_value->body.string);
                    if (grab_public_key == NULL) {
                        LOG_ERROR_0("concatenation for grab_public_key failed");
                        goto err;
                    }
                    const char* client_public_key = m_kv_store_handle->get(cfgmgr_handle, grab_public_key);
                    if(client_public_key == NULL){
                        // If any service isn't provisioned, ignore if key not found
                        LOG_DEBUG("Value is not found for the key: %s", grab_public_key);
                    }
                    free(grab_public_key);

                    config_value_destroy(array_value);
                    cJSON_AddItemToArray(all_clients, cJSON_CreateString(client_public_key));
                    free(client_public_key);
                }
                // Adding all public keys of clients to allowed_clients of config
                cJSON_AddItemToObject(c_json, "allowed_clients",  all_clients);
            }

            // Fetching Publisher private key & adding it to server_topic object
            size_t init_len = strlen("/") + strlen(PRIVATE_KEY) + strlen(app_name) + 2;
            pub_pri_key = concat_s(init_len, 3, "/", app_name, PRIVATE_KEY);
            if (pub_pri_key == NULL) {
                LOG_ERROR_0("concatenation for pub_pri_key failed");
                goto err;
            }
            server_secret_key = m_kv_store_handle->get(cfgmgr_handle, pub_pri_key);
            if (server_secret_key == NULL) {
                LOG_ERROR("Value is not found for the key: %s", pub_pri_key);
                goto err;
            }
            cJSON_AddStringToObject(server_topic, "server_secret_key", server_secret_key);
        }
        // Creating the final cJSON config object
        // server_name
        cJSON_AddItemToObject(c_json, server_name->body.string, server_topic);
    } else {
        LOG_ERROR_0("Type should be either \"zmq_ipc\" or \"zmq_tcp\"");
        goto err;
    }

    // Constructing char* object from cJSON object
    config_value_cr = cJSON_Print(c_json);
    if (config_value_cr == NULL) {
        LOG_ERROR_0("config_value_cr initialization failed");
        goto err;
    }
    LOG_DEBUG("Env server Config is : %s \n", config_value_cr);

    // Constructing config_t object from cJSON object
    config_t* m_config = config_new(
            (void*) c_json, free_json, get_config_value);
    if (m_config == NULL) {
        LOG_ERROR_0("Failed to initialize configuration object");
        goto err;
    }
    ret = m_config;

err:
    if (temp_array_value != NULL) {
        config_value_destroy(temp_array_value);
    }
    if (server_name != NULL) {
        config_value_destroy(server_name);
    }
    if (server_config_type != NULL) {
        config_value_destroy(server_config_type);
    }
    if (ep_override_env != NULL) {
        free(ep_override_env);
    }
    if (host_port != NULL) {
        free_mem(host_port);
    }
    if (type_override_env != NULL) {
        free(type_override_env);
    }
    if (server_json_clients != NULL) {
        config_value_destroy(server_json_clients);
    }
    if (pub_key_values != NULL) {
        config_value_destroy(pub_key_values);
    }
    if (server_endpoint != NULL) {
        config_value_destroy(server_endpoint);
    }
    if (pub_pri_key != NULL) {
        free(pub_pri_key);
    }
    if (server_secret_key != NULL) {
        free(server_secret_key);
    }
    if (config_value_cr != NULL) {
        free(config_value_cr);
    }
    if (grab_public_key != NULL) {
        free(grab_public_key);
    }
    if (client_public_key != NULL) {
        free(client_public_key);
    }
    return ret;
}

// function to initialize server_cfg_t
server_cfg_t* server_cfg_new() {
    LOG_DEBUG_0("In server_cfg_new mthod");
    server_cfg_t *serv_cfg_mgr = (server_cfg_t *)malloc(sizeof(server_cfg_t));
    if (serv_cfg_mgr == NULL) {
        LOG_ERROR_0("Malloc failed for pub_cfg_t");
        return NULL;
    }
    serv_cfg_mgr->cfgmgr_get_msgbus_config_server = cfgmgr_get_msgbus_config_server;
    serv_cfg_mgr->cfgmgr_get_interface_value_server = cfgmgr_get_interface_value_server;
    serv_cfg_mgr->cfgmgr_get_endpoint_server = cfgmgr_get_endpoint_server;
    serv_cfg_mgr->cfgmgr_get_allowed_clients_server = cfgmgr_get_allowed_clients_server;
    return serv_cfg_mgr;
}

// function to destroy server_cfg_t
void server_cfg_config_destroy(server_cfg_t *server_cfg_config) {
    if (server_cfg_config != NULL) {
        if (server_cfg_config->server_config != NULL) {
            config_value_destroy(server_cfg_config->server_config);
        }
        free(server_cfg_config);
    }
}
