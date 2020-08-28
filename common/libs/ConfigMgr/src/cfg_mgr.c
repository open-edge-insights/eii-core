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

#include <stdarg.h>
#include "eis/config_manager/c_cfg_mgr.h"

#define MAX_CONFIG_KEY_LENGTH 250

// function to generate kv_store_config from env
config_t* create_kv_store_config() {
    // Creating cJSON object
    cJSON* c_json = cJSON_CreateObject();

    // Fetching ConfigManager type from env
    char* config_manager_type = getenv("KVStore");
    if(config_manager_type == NULL) {
        LOG_DEBUG_0("KVStore env not set, defaulting to etcd");
        cJSON_AddStringToObject(c_json, "type", "etcd");
    } else {
        size_t str_len = strlen(config_manager_type) + 1;
        char* c_type_name = (char*)malloc(sizeof(char) * str_len);
        snprintf(c_type_name, str_len, "%s", config_manager_type);
        LOG_INFO("ConfigManager selected is %s", c_type_name);
        cJSON_AddStringToObject(c_json, "type", c_type_name);
    }

    // Creating etcd_kv_store object
    cJSON* etcd_kv_store = cJSON_CreateObject();
    cJSON_AddItemToObject(c_json, "etcd_kv_store", etcd_kv_store);

    // Fetching ETCD_HOST type from env
    char* etcd_host = getenv("ETCD_HOST");
    int ind_etcd_host;
    strcmp_s(etcd_host, strlen(etcd_host), "", &ind_etcd_host);

    if(etcd_host == NULL ) {
        LOG_DEBUG_0("ETCD_HOST env not set, defaulting to localhost");
        cJSON_AddStringToObject(etcd_kv_store, "host", "localhost");
    } else if(ind_etcd_host == 0){
        LOG_DEBUG_0("ETCD_HOST env is set to empty, defaulting to localhost");
        cJSON_AddStringToObject(etcd_kv_store, "host", "localhost");
    } else {
        size_t str_len = strlen(etcd_host) + 1;
        char* c_etcd_host = (char*)malloc(sizeof(char) * str_len);
        snprintf(c_etcd_host, str_len, "%s", etcd_host);
        LOG_DEBUG("ETCD host: %s", c_etcd_host);
        cJSON_AddStringToObject(etcd_kv_store, "host", c_etcd_host);
    }

    // Fetching ETCD_HOST type from env
    char* etcd_port = getenv("ETCD_PORT");
    if(etcd_port == NULL) {
        cJSON_AddStringToObject(etcd_kv_store, "port", "2379");
    } else {
        size_t str_len = strlen(etcd_port) + 1;
        char* c_etcd_port = (char*)malloc(sizeof(char) * str_len);
        snprintf(c_etcd_port, str_len, "%s", etcd_port);
        LOG_DEBUG("ETCD port: %s", c_etcd_port);
        cJSON_AddStringToObject(etcd_kv_store, "host", c_etcd_port);
    }

    // Fetching & intializing dev mode variable
    char* dev_mode_var = getenv("DEV_MODE");
    to_lower(dev_mode_var);
    int result = strcmp(dev_mode_var, "true");

    // Fetching & intializing AppName
    char* app_name_var = getenv("AppName");
    if(app_name_var == NULL) {
        LOG_ERROR_0("AppName env not set");
        goto err;
    }
    size_t str_len = strlen(app_name_var) + 1;
    char* c_app_name = (char*)malloc(sizeof(char) * str_len);
    snprintf(c_app_name, str_len, "%s", app_name_var);
    LOG_DEBUG("AppName: %s", c_app_name);

    int ret = 0;
    char pub_cert_file[MAX_CONFIG_KEY_LENGTH] = "";
    char pri_key_file[MAX_CONFIG_KEY_LENGTH] = "";
    char trust_file[MAX_CONFIG_KEY_LENGTH] = "";
    if(result == 0) {
        cJSON_AddStringToObject(etcd_kv_store, "cert_file", "");
        cJSON_AddStringToObject(etcd_kv_store, "key_file", "");
        cJSON_AddStringToObject(etcd_kv_store, "ca_file", "");
    } else {
        ret = snprintf(pub_cert_file, MAX_CONFIG_KEY_LENGTH,
                 "/run/secrets/etcd_%s_cert", c_app_name);
        if (ret < 0) {
            LOG_ERROR_0("failed to create pub_cert_file");
            goto err;
        }
        ret = snprintf(pri_key_file, MAX_CONFIG_KEY_LENGTH,
                 "/run/secrets/etcd_%s_key", c_app_name);
        if (ret < 0) {
            LOG_ERROR_0("failed to create pri_key_file");
            goto err;
        }
        ret = strncpy_s(trust_file, MAX_CONFIG_KEY_LENGTH + 1,
                  "/run/secrets/ca_etcd", MAX_CONFIG_KEY_LENGTH);
        if (ret != 0) {
            LOG_ERROR_0("failed to create trust file");
            goto err;
        }

        char* confimgr_cert = getenv("CONFIGMGR_CERT");
        char* confimgr_key = getenv("CONFIGMGR_KEY");
        char* confimgr_cacert = getenv("CONFIGMGR_CACERT");
        if(confimgr_cert && confimgr_key && confimgr_cacert) {
            ret = strncpy_s(pub_cert_file, MAX_CONFIG_KEY_LENGTH + 1,
                            confimgr_cert, MAX_CONFIG_KEY_LENGTH);
            if (ret != 0) {
                LOG_ERROR_0("failed to add cert to trust file");
                goto err;
            }
            ret = strncpy_s(pri_key_file, MAX_CONFIG_KEY_LENGTH + 1,
                            confimgr_key, MAX_CONFIG_KEY_LENGTH);
            if (ret !=0) {
                LOG_ERROR_0("failed to add key to trust file");
                goto err;
            }
            ret = strncpy_s(trust_file, MAX_CONFIG_KEY_LENGTH + 1,
                            confimgr_cacert, MAX_CONFIG_KEY_LENGTH);
            if (ret != 0 ) {
                LOG_ERROR_0("failed to add cacert to trust file");
                goto err;
            }
        }
        cJSON_AddStringToObject(etcd_kv_store, "cert_file", pub_cert_file);
        cJSON_AddStringToObject(etcd_kv_store, "key_file", pri_key_file);
        cJSON_AddStringToObject(etcd_kv_store, "ca_file", trust_file);
    }

    // Constructing char* object from cJSON object
    char* config_value_cr = cJSON_Print(c_json);
    LOG_DEBUG("KV store config is : %s \n", config_value_cr);

    // Constructing config_t object from cJSON object
    config_t* config = config_new(
            (void*) c_json, free_json, get_config_value);
    if (config == NULL) {
        LOG_ERROR_0("Failed to initialize configuration object");
        goto err;
    }

    return config;
err:
    if (config_manager_type != NULL) {
        free(config_manager_type);
    }
    if (etcd_host != NULL) {
        free(etcd_host);
    }
    if (etcd_port != NULL) {
        free(etcd_port);
    }
    if (dev_mode_var != NULL) {
        free(dev_mode_var);
    }
    if (c_app_name != NULL) {
        free(c_app_name);
    }
    if (app_name_var != NULL) {
        free(app_name_var);
    }
    if (config_value_cr != NULL) {
        free(config_value_cr);
    }
    return NULL;
}

// function to get publisher by name
pub_cfg_t* get_publisher_by_name(app_cfg_t* app_cfg, const char* name) {

    LOG_INFO_0("get_publisher_by_name method");
    config_t* app_interface = app_cfg->base_cfg->m_app_interface;

    // Fetching list of Publisher interfaces
    config_value_t* publisher_interface = app_interface->get_config_value(app_interface->cfg, PUBLISHERS);

    // Iterating through available publisher configs
    for(int i=0; i<config_value_array_len(publisher_interface); i++) {
        // Fetch name of individual publisher config
        config_value_t* pub_config = config_value_array_get(publisher_interface, i);
        config_value_t* pub_config_name = config_value_object_get(pub_config, "Name");
        // Verifying publisher config with name exists
        if(strcmp(pub_config_name->body.string, name) == 0) {
            pub_cfg_t* pub_cfg = pub_cfg_new();
            app_cfg->base_cfg->pub_sub_config = pub_config;
            return pub_cfg;
        } else if(i == config_value_array_len(publisher_interface)) {
            LOG_ERROR("Publisher by name %s not found", name);
            return NULL;
        }
    }
}

// function to get publisher by index
pub_cfg_t* get_publisher_by_index(app_cfg_t* app_cfg, int index) {

    LOG_INFO_0("get_publisher_by_index method");
    config_t* app_interface = app_cfg->base_cfg->m_app_interface;

    // Fetching list of Publisher interfaces
    config_value_t* publisher_interface = app_interface->get_config_value(app_interface->cfg, PUBLISHERS);

    // Fetch publisher config associated with index
    config_value_t* pub_config_index = config_value_array_get(publisher_interface, index);
    pub_cfg_t* pub_cfg = pub_cfg_new();
    app_cfg->base_cfg->pub_sub_config = pub_config_index;
    return pub_cfg;
}

// function to get subscriber by name
sub_cfg_t* get_subscriber_by_name(app_cfg_t* app_cfg, const char* name) {

    LOG_INFO_0("get_subscriber_by_name method");
    config_t* app_interface = app_cfg->base_cfg->m_app_interface;

    // Fetching list of Subscribers interfaces
    config_value_t* subscriber_interface = app_interface->get_config_value(app_interface->cfg, SUBSCRIBERS);

    // Iterating through available subscriber configs
    for(int i=0; i<config_value_array_len(subscriber_interface); i++) {
        // Fetch name of individual subscriber config
        config_value_t* sub_config = config_value_array_get(subscriber_interface, i);
        config_value_t* sub_config_name = config_value_object_get(sub_config, "Name");
        // Verifying subscriber config with name exists
        if(strcmp(sub_config_name->body.string, name) == 0) {
            sub_cfg_t* sub_cfg = sub_cfg_new();
            app_cfg->base_cfg->pub_sub_config = sub_config;
            return sub_cfg;
        } else if(i == config_value_array_len(subscriber_interface)) {
            LOG_ERROR("Subscribers by name %s not found", name);
            return NULL;
        }
    }
}

// function to get subscriber by index
sub_cfg_t* get_subscriber_by_index(app_cfg_t* app_cfg, int index) {

    LOG_INFO_0("get_subscriber_by_index method");
    config_t* app_interface = app_cfg->base_cfg->m_app_interface;

    // Fetching list of Subscribers interfaces
    config_value_t* subscriber_interface = app_interface->get_config_value(app_interface->cfg, SUBSCRIBERS);

    // Fetch subscriber config associated with index
    config_value_t* sub_config = config_value_array_get(subscriber_interface, index);
    sub_cfg_t* sub_cfg = sub_cfg_new();
    app_cfg->base_cfg->pub_sub_config = sub_config;
    return sub_cfg;
}

// function to get server by name
server_cfg_t* get_server_by_name(app_cfg_t* app_cfg, const char* name) {

    LOG_INFO_0("get_server_by_name method");
    config_t* app_interface = app_cfg->base_cfg->m_app_interface;

    // Fetching list of server interfaces
    config_value_t* server_interface = app_interface->get_config_value(app_interface->cfg, SERVERS);

    // Iterating through available server configs
    for(int i=0; i<config_value_array_len(server_interface); i++) {
        // Fetch name of individual server config
        config_value_t* serv_config = config_value_array_get(server_interface, i);
        config_value_t* serv_config_name = config_value_object_get(serv_config, "Name");
        // Verifying server config with name exists
        if(strcmp(serv_config_name->body.string, name) == 0) {
            server_cfg_t* serv_cfg = server_cfg_new();
            app_cfg->base_cfg->pub_sub_config = serv_config;
            return serv_cfg;
        } else if(i == config_value_array_len(server_interface)) {
            LOG_ERROR("Servers by name %s not found", name);
            return NULL;
        }
    }
}

// function to get server by index
server_cfg_t* get_server_by_index(app_cfg_t* app_cfg, int index) {

    LOG_INFO_0("get_server_by_index method");
    config_t* app_interface = app_cfg->base_cfg->m_app_interface;

    // Fetching list of Servers interfaces
    config_value_t* server_interface = app_interface->get_config_value(app_interface->cfg, SERVERS);

    // Fetch Server config associated with index
    config_value_t* server_config = config_value_array_get(server_interface, index);
    server_cfg_t* serv_cfg = server_cfg_new();
    app_cfg->base_cfg->pub_sub_config = server_config;
    return serv_cfg;
}

// function to get client by name
client_cfg_t* get_client_by_name(app_cfg_t* app_cfg, const char* name) {

    LOG_INFO_0("get_client_by_name method");
    config_t* app_interface = app_cfg->base_cfg->m_app_interface;

    // Fetching list of client interfaces
    config_value_t* client_interface = app_interface->get_config_value(app_interface->cfg, CLIENTS);

    // Iterating through available client configs
    for(int i=0; i<config_value_array_len(client_interface); i++) {
        // Fetch name of individual client config
        config_value_t* cli_config = config_value_array_get(client_interface, i);
        config_value_t* cli_config_name = config_value_object_get(cli_config, "Name");
        // Verifying client config with name exists
        if(strcmp(cli_config_name->body.string, name) == 0) {
            client_cfg_t* cli_cfg = client_cfg_new();
            app_cfg->base_cfg->pub_sub_config = cli_config;
            return cli_cfg;
        } else if(i == config_value_array_len(client_interface)) {
            LOG_ERROR("Clients by name %s not found", name);
            return NULL;
        }
    }
}

// function to get client by index
client_cfg_t* get_client_by_index(app_cfg_t* app_cfg, int index) {

    LOG_INFO_0("get_client_by_index method");
    config_t* app_interface = app_cfg->base_cfg->m_app_interface;

    // Fetching list of Clients interfaces
    config_value_t* client_interface = app_interface->get_config_value(app_interface->cfg, CLIENTS);

    // Fetch client config associated with index
    config_value_t* cli_config = config_value_array_get(client_interface, index);
    client_cfg_t* cli_cfg = client_cfg_new();
    app_cfg->base_cfg->pub_sub_config = cli_config;
    return cli_cfg;
}

// function to destroy app_cfg_t
void app_cfg_config_destroy(app_cfg_t *app_cfg_config) {
    if(app_cfg_config != NULL) {
        free(app_cfg_config);
    }
}

// function to initialize app_cfg_t
app_cfg_t* app_cfg_new() {

    app_cfg_t *app_cfg = (app_cfg_t *)malloc(sizeof(app_cfg_t));
    if(app_cfg == NULL) {
        LOG_ERROR_0("Malloc failed for app_cfg_t");
        goto err;
    }

    // Fetching AppName
    char* app_name_var = getenv("AppName");
    if(app_name_var == NULL) {
        LOG_ERROR_0("AppName env not set");
        goto err;
    }
    size_t str_len = strlen(app_name_var) + 1;
    char* c_app_name = (char*)malloc(sizeof(char) * str_len);
    snprintf(c_app_name, str_len, "%s", app_name_var);
    LOG_DEBUG("AppName: %s", c_app_name);
    trim(c_app_name);

    // Fetching & intializing dev mode variable
    char* dev_mode_var = getenv("DEV_MODE");
    to_lower(dev_mode_var);
    int result = strcmp(dev_mode_var, "true");

    config_t* kv_store_config = create_kv_store_config();
    // Creating kv store client instance
    kv_store_client_t* kv_store_client = create_kv_client(kv_store_config);
    if(kv_store_client == NULL) {
        LOG_ERROR_0("kv_store_client is NULL");
        goto err;
    }

    // Initializing etcd client handle
    void *handle = kv_store_client->init(kv_store_client);

    // Fetching App interfaces
    size_t init_len = strlen("/") + strlen(c_app_name) + strlen("/interfaces") + 1;
    char* interface_char = concat_s(init_len, 3, "/", c_app_name, "/interfaces");

    // Fetching App config
    init_len = strlen("/") + strlen(c_app_name) + strlen("/config") + 1;
    char* config_char = concat_s(init_len, 3, "/", c_app_name, "/config");

    LOG_DEBUG("interface_char: %s", interface_char);
    LOG_DEBUG("config_char: %s", config_char);

    char* interface = kv_store_client->get(handle, interface_char);
    if(interface == NULL){
        LOG_ERROR("Value is not found for the key: %s", interface_char);
        goto err;
    }

    char* value = kv_store_client->get(handle, config_char);
    if(value == NULL){
        LOG_ERROR("Value is not found for the key: %s", config_char);
        goto err;
    }

    config_t* app_config = json_config_new_from_buffer(value);
    config_t* app_interface = json_config_new_from_buffer(interface);

    app_cfg->base_cfg = base_cfg_new();
    if(app_cfg->base_cfg != NULL) {
        if(c_app_name != NULL) {
        app_cfg->base_cfg->app_name = c_app_name;
        }
        if(kv_store_client != NULL) {
            app_cfg->base_cfg->m_kv_store_handle = kv_store_client;
        }
        if(app_config != NULL) {
            app_cfg->base_cfg->m_app_config = app_config;
        }
        if(app_interface != NULL) {
            app_cfg->base_cfg->m_app_interface = app_interface;
        }
        app_cfg->base_cfg->dev_mode = result;
    }
    return app_cfg;
err:
    if(app_name_var != NULL) {
        free(app_name_var);
    }
    if(c_app_name != NULL) {
        free(c_app_name);
    }
    if(dev_mode_var != NULL) {
        free(dev_mode_var);
    }
    if(interface != NULL) {
        free(interface);
    }
    if(interface_char != NULL) {
        free(interface_char);
    }
    if(config_char != NULL) {
        free(config_char);
    }
    if(value != NULL) {
        free(value);
    }
    return NULL;
}