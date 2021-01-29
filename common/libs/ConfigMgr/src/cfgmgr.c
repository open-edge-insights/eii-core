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
#include "eis/config_manager/cfgmgr.h"

// function to generate kv_store_config from env
config_t* create_kv_store_config() {
    char* c_type_name = NULL;
    config_t* config = NULL;
    char* config_value_cr = NULL;
    char* c_app_name = NULL;
    char* config_manager_type = NULL;
    char* dev_mode_var = NULL;
    char* app_name_var = NULL;
    // Creating cJSON object
    cJSON* c_json = cJSON_CreateObject();
    if (c_json == NULL) {
        LOG_ERROR_0("c_json initialization failed");
        goto err;
    }

    // Fetching ConfigManager type from env
    config_manager_type = getenv("KVStore");
    if (config_manager_type == NULL) {
        LOG_DEBUG_0("KVStore env not set, defaulting to etcd");
        cJSON_AddStringToObject(c_json, "type", "etcd");
    } else {
        int ind_kv_store_type;
        strcmp_s(config_manager_type, strlen(config_manager_type),
                 "", &ind_kv_store_type);
        if (ind_kv_store_type == 0) {
            LOG_DEBUG_0("KVStore env is set to empty, defaulting to etcd");
            cJSON_AddStringToObject(c_json, "type", "etcd");
        } else {
            size_t str_len = strlen(config_manager_type) + 1;
            c_type_name = (char*)malloc(sizeof(char) * str_len);
            if (c_type_name == NULL){
                LOG_ERROR_0("Malloc failed for c_type_name");
                goto err;
            }
            int ret = snprintf(c_type_name, str_len, "%s", config_manager_type);
            if (ret < 0){
                LOG_ERROR_0("snprintf failed for c_type_name");
                goto err;
            }
            LOG_DEBUG("ConfigManager selected is %s", c_type_name);
            cJSON_AddStringToObject(c_json, "type", c_type_name);
        }
    }

    // Creating etcd_kv_store object
    cJSON* etcd_kv_store = cJSON_CreateObject();
    if (etcd_kv_store == NULL) {
        LOG_ERROR_0("c_json initialization failed");
        goto err;
    }
    cJSON_AddItemToObject(c_json, "etcd_kv_store", etcd_kv_store);

    // Fetching & intializing dev mode variable
    int result = 0;
    dev_mode_var = getenv("DEV_MODE");
    if (dev_mode_var == NULL) {
        LOG_DEBUG_0("DEV_MODE env not set, defaulting to true");
        result = 0;
    } else {
        int ind_dev_mode;
        strcmp_s(dev_mode_var, strlen(dev_mode_var), "", &ind_dev_mode);
        if (ind_dev_mode == 0) {
            LOG_DEBUG_0("DEV_MODE env is set to empty, defaulting to true");
            result = 0;
        } else {
            to_lower(dev_mode_var);
            strcmp_s(dev_mode_var, strlen(dev_mode_var), "true", &result);
        }
    }

    // Fetching & intializing AppName
    app_name_var = getenv("AppName");
    if (app_name_var == NULL) {
        LOG_ERROR_0("AppName env not set");
        goto err;
    }
    size_t str_len = strlen(app_name_var) + 1;
    c_app_name = (char*)malloc(sizeof(char) * str_len);
    if (c_app_name == NULL) {
        LOG_ERROR_0("Malloc failed for c_app_name");
        goto err;
    }
    int ret = snprintf(c_app_name, str_len, "%s", app_name_var);
    if (ret < 0){
        LOG_ERROR_0("snprintf failed for c_app_name");
        goto err;
    }
    LOG_DEBUG("AppName: %s", c_app_name);

    ret = 0;
    char pub_cert_file[MAX_CONFIG_KEY_LENGTH] = "";
    char pri_key_file[MAX_CONFIG_KEY_LENGTH] = "";
    char trust_file[MAX_CONFIG_KEY_LENGTH] = "";
    if (result == 0) {
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
        if (confimgr_cert && confimgr_key && confimgr_cacert) {
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
            if (ret != 0) {
                LOG_ERROR_0("failed to add cacert to trust file");
                goto err;
            }
        }
        cJSON_AddStringToObject(etcd_kv_store, "cert_file", pub_cert_file);
        cJSON_AddStringToObject(etcd_kv_store, "key_file", pri_key_file);
        cJSON_AddStringToObject(etcd_kv_store, "ca_file", trust_file);
    }

    // Constructing char* object from cJSON object
    config_value_cr = cJSON_Print(c_json);
    if (config_value_cr == NULL) {
        LOG_ERROR_0("KV Store config is NULL");
        goto err;
    }
    LOG_DEBUG("KV store config is : %s \n", config_value_cr);

    // Constructing config_t object from cJSON object
    config = config_new((void*) c_json, free_json, get_config_value);
    if (config == NULL) {
        LOG_ERROR_0("Failed to initialize configuration object");
        goto err;
    }

    if (config_value_cr != NULL) {
        free(config_value_cr);
    }
    if (c_app_name != NULL) {
        free(c_app_name);
    }
    if (c_type_name != NULL) {
        free(c_type_name);
    }
    return config;

err:
    if (c_app_name != NULL) {
        free(c_app_name);
    }
    if (c_type_name != NULL) {
        free(c_type_name);
    }
    if (config_value_cr != NULL) {
        free(config_value_cr);
    }
    if (c_json != NULL) {
        cJSON_Delete(c_json);
    }
    return NULL;
}

cfgmgr_interface_t* cfgmgr_interface_initialize() {
    cfgmgr_interface_t *cfgmgr_ctx = (cfgmgr_interface_t *)malloc(sizeof(cfgmgr_interface_t));
    if (cfgmgr_ctx == NULL) {
        LOG_ERROR_0("Malloc failed for cfgmgr_ctx_t");
    }
    return cfgmgr_ctx;
}

cfgmgr_interface_t* cfgmgr_get_publisher_by_name(cfgmgr_ctx_t* cfgmgr, const char* name) {
    int result = 0;
    LOG_INFO_0("cfgmgr_get_publisher_by_name method");
    cfgmgr_interface_t* ctx = NULL;
    config_value_t* pub_config_name = NULL;
    config_value_t* publisher_interface = NULL;

    config_t* app_interface = cfgmgr->app_interface;
    ctx = cfgmgr_interface_initialize();
    if (ctx == NULL) {
        LOG_ERROR_0("cfgmgr initialization failed");
        goto err;
    }

    // Fetching list of Publisher interfaces
    publisher_interface = app_interface->get_config_value(app_interface->cfg, PUBLISHERS);
    if (publisher_interface == NULL) {
        LOG_ERROR_0("publisher_interface initialization failed");
        goto err;
    }

    size_t arr_len = config_value_array_len(publisher_interface);
    if(arr_len == 0){
        LOG_ERROR_0("Empty array is not supported, atleast one value should be given.");
        goto err;
    }
    // Iterating through available publisher configs
    for(int i=0; i<arr_len; i++) {
        // Fetch name of individual publisher config
        config_value_t* pub_config = config_value_array_get(publisher_interface, i);
        if (pub_config == NULL) {
            LOG_ERROR_0("pub_config initialization failed");
            goto err;
        }
        pub_config_name = config_value_object_get(pub_config, "Name");
        if (pub_config_name == NULL || pub_config_name->body.string == NULL) {
            LOG_ERROR_0("pub_config_name initialization failed");
            goto err;
        }
        // Verifying publisher config with name exists
        strcmp_s(pub_config_name->body.string, strlen(pub_config_name->body.string), name, &result);
        if(result == 0) {
            ctx->cfg_mgr = cfgmgr;
            ctx->interface = pub_config;
            ctx->type = CFGMGR_PUBLISHER;
            goto err;
        } else if(i == config_value_array_len(publisher_interface)) {
            LOG_ERROR("Publisher by name %s not found", name);
            goto err;
        } else {
            if (pub_config_name != NULL) {
                config_value_destroy(pub_config_name);
            }
            if (pub_config != NULL) {
                config_value_destroy(pub_config);
            }
        }
    }

err:
    if(publisher_interface != NULL) {
        config_value_destroy(publisher_interface);
    }
    if (pub_config_name != NULL) {
        config_value_destroy(pub_config_name);
    }
    return ctx;
}

cfgmgr_interface_t* cfgmgr_get_publisher_by_index(cfgmgr_ctx_t* cfgmgr, int index) {
    LOG_DEBUG_0("cfgmgr_get_publisher_by_index method");
    config_value_t* pub_config_index = NULL;
    config_value_t* publisher_interface = NULL;
    config_t* app_interface = cfgmgr->app_interface;
    cfgmgr_interface_t* ctx = NULL;
    ctx = cfgmgr_interface_initialize();
    if (ctx == NULL) {
        LOG_ERROR_0("cfgmgr initialization failed");
        goto err;
    }

    // Fetching list of Publisher interfaces
    publisher_interface = app_interface->get_config_value(app_interface->cfg, PUBLISHERS);
    if (publisher_interface == NULL) {
        LOG_ERROR_0("publisher_interface initialization failed");
        goto err;
    }

    // Fetch publisher config associated with index
    pub_config_index = config_value_array_get(publisher_interface, index);
    if (pub_config_index == NULL) {
        LOG_ERROR_0("pub_config_index initialization failed");
        goto err;
    }

    ctx->cfg_mgr = cfgmgr;
    ctx->interface = pub_config_index;
    ctx->type = CFGMGR_PUBLISHER;

    if (publisher_interface != NULL) {
        config_value_destroy(publisher_interface);
    }
    return ctx;

err:
    if (publisher_interface != NULL) {
        config_value_destroy(publisher_interface);
    }
    if (pub_config_index != NULL) {
        config_value_destroy(pub_config_index);
    }
    if (ctx != NULL) {
        cfgmgr_interface_destroy(ctx);
    }
    return NULL;

}

cfgmgr_interface_t* cfgmgr_get_subscriber_by_name(cfgmgr_ctx_t* cfgmgr, const char* name) {
    LOG_INFO_0("cfgmgr_get_subscriber_by_name method");
    config_value_t* sub_config_name = NULL;
    config_value_t* sub_config_index = NULL;
    config_value_t* subscriber_interface = NULL;
    config_t* app_interface = cfgmgr->app_interface;
    cfgmgr_interface_t* ctx = NULL;
    ctx = cfgmgr_interface_initialize();
    if (ctx == NULL) {
        LOG_ERROR_0("cfgmgr initialization failed");
        goto err;
    }

    // Fetching list of Subscribers interfaces
    subscriber_interface = app_interface->get_config_value(app_interface->cfg, SUBSCRIBERS);
    if (subscriber_interface == NULL) {
        LOG_ERROR_0("subscriber_interface initialization failed");
        goto err;
    }

    size_t arr_len = config_value_array_len(subscriber_interface);
    if(arr_len == 0){
        LOG_ERROR_0("Empty array is not supported, atleast one value should be given.");
        goto err;
    }

    // Iterating through available subscriber configs
    for(int i=0; i<arr_len; i++) {
        // Fetch name of individual subscriber config
        sub_config_index = config_value_array_get(subscriber_interface, i);
        if (sub_config_index == NULL) {
            LOG_ERROR_0("sub_config_index initialization failed");
            goto err;
        }
        sub_config_name = config_value_object_get(sub_config_index, "Name");
        if (sub_config_name == NULL || sub_config_name->body.string == NULL) {
            LOG_ERROR_0("sub_config_name initialization failed");
            goto err;
        }
        // Verifying subscriber config with name exists
        int ret;
        strcmp_s(sub_config_name->body.string, strlen(sub_config_name->body.string), name, &ret);
        if(ret == 0) {
            ctx->cfg_mgr = cfgmgr;
            ctx->interface = sub_config_index;
            ctx->type = CFGMGR_SUBSCRIBER;
            goto err;
        } else if(i == config_value_array_len(subscriber_interface)) {
            LOG_ERROR("Subscribers by name %s not found", name);
            if (sub_config_index != NULL) {
                config_value_destroy(sub_config_index);
            }
            goto err;
        } else {
            if (sub_config_name != NULL) {
                config_value_destroy(sub_config_name);
            }
            if (sub_config_index != NULL) {
                config_value_destroy(sub_config_index);
            }
        }
    }

err:
    if (sub_config_name != NULL) {
        config_value_destroy(sub_config_name);
    }
    if (subscriber_interface != NULL) {
        config_value_destroy(subscriber_interface);
    }
    return ctx;
}

cfgmgr_interface_t* cfgmgr_get_subscriber_by_index(cfgmgr_ctx_t* cfgmgr, int index) {
    LOG_INFO_0("cfgmgr_get_subscriber_by_index method");

    config_t* app_interface = cfgmgr->app_interface;
    config_value_t* sub_config_index = NULL;
    config_value_t* subscriber_interface = NULL;
    cfgmgr_interface_t* ctx = NULL;
    ctx = cfgmgr_interface_initialize();
    if (ctx == NULL) {
        LOG_ERROR_0("cfgmgr initialization failed");
        goto err;
    }

    // Fetching list of Subscribers interfaces
    subscriber_interface = app_interface->get_config_value(app_interface->cfg, SUBSCRIBERS);
    if (subscriber_interface == NULL) {
        LOG_ERROR_0("subscriber_interface initialization failed");
        goto err;
    }

    // Fetch subscriber config associated with index
    sub_config_index = config_value_array_get(subscriber_interface, index);
    if (sub_config_index == NULL) {
        LOG_ERROR_0("sub_config_index initialization failed");
        goto err;
    }

    ctx->cfg_mgr = cfgmgr;
    ctx->interface = sub_config_index;
    ctx->type = CFGMGR_SUBSCRIBER;

    if (subscriber_interface != NULL) {
        config_value_destroy(subscriber_interface);
    }

    return ctx;
err:
    if (subscriber_interface != NULL) {
        config_value_destroy(subscriber_interface);
    }
    if (sub_config_index != NULL) {
        config_value_destroy(sub_config_index);
    }
    if (ctx != NULL) {
        cfgmgr_interface_destroy(ctx);
    }
    return NULL;
}

cfgmgr_interface_t* cfgmgr_get_server_by_name(cfgmgr_ctx_t* cfgmgr, const char* name) {
    LOG_INFO_0("cfgmgr_get_server_by_name method");
    int result = 0;
    config_value_t* server_interface = NULL;
    config_value_t* serv_config_name = NULL;
    config_t* app_interface = cfgmgr->app_interface;
    cfgmgr_interface_t* ctx = NULL;
    ctx = cfgmgr_interface_initialize();
    if (ctx == NULL) {
        LOG_ERROR_0("cfgmgr initialization failed");
        goto err;
    }

    // Fetching list of server interfaces
    server_interface = app_interface->get_config_value(app_interface->cfg, SERVERS);
    if (server_interface == NULL) {
        LOG_ERROR_0("server_interface initialization failed");
        goto err;
    }

    size_t arr_len = config_value_array_len(server_interface);
    if(arr_len == 0){
        LOG_ERROR_0("Empty array is not supported, atleast one value should be given.");
        goto err;
    }
    // Iterating through available server configs
    for(int i=0; i<arr_len; i++) {
        // Fetch name of individual server config
        config_value_t* serv_config = config_value_array_get(server_interface, i);
        if (serv_config == NULL) {
            LOG_ERROR_0("serv_config initialization failed");
            goto err;
        }
        serv_config_name = config_value_object_get(serv_config, "Name");
        if (serv_config_name == NULL) {
            LOG_ERROR_0("serv_config_name initialization failed");
            goto err;
        }
        // Verifying server config with name exists
        strcmp_s(serv_config_name->body.string, strlen(serv_config_name->body.string), name, &result);
        if(result == 0) {
            ctx->interface = serv_config;
            ctx->cfg_mgr = cfgmgr;
            ctx->type = CFGMGR_SERVER;
            goto err;
        } else if(i == config_value_array_len(server_interface)) {
            LOG_ERROR("Servers by name %s not found", name);
            goto err;
        } else {
            if (serv_config_name != NULL) {
                config_value_destroy(serv_config_name);
            }
            if (serv_config != NULL) {
                config_value_destroy(serv_config);
            }
        }
    }

err:
    if (server_interface != NULL) {
        config_value_destroy(server_interface);
    }
    if (serv_config_name != NULL) {
        config_value_destroy(serv_config_name);
    }
    return ctx;
}

cfgmgr_interface_t* cfgmgr_get_server_by_index(cfgmgr_ctx_t* cfgmgr, int index) {
    LOG_INFO_0("cfgmgr_get_server_by_index method");
    config_t* app_interface = cfgmgr->app_interface;
    config_value_t* server_interface = NULL;
    config_value_t* server_config = NULL;
    cfgmgr_interface_t* ctx = NULL;
    ctx = cfgmgr_interface_initialize();
    if (ctx == NULL) {
        LOG_ERROR_0("cfgmgr initialization failed");
        goto err;
    }

    // Fetching list of Servers interfaces
    server_interface = app_interface->get_config_value(app_interface->cfg, SERVERS);
    if (server_interface == NULL) {
        LOG_ERROR_0("server_interface initialization failed");
        goto err;
    }

    // Fetch Server config associated with index
    server_config = config_value_array_get(server_interface, index);
    if (server_config == NULL) {
        LOG_ERROR_0("server_config initialization failed");
        goto err;
    }
    ctx->interface = server_config;
    ctx->cfg_mgr = cfgmgr;
    ctx->type = CFGMGR_SERVER;
err:
    if (server_interface != NULL) {
        config_value_destroy(server_interface);
    }
    return ctx;
}

cfgmgr_interface_t* cfgmgr_get_client_by_name(cfgmgr_ctx_t* cfgmgr, const char* name) {
    LOG_INFO_0("cfgmgr_get_client_by_name method");
    int ret;
    config_t* app_interface = cfgmgr->app_interface;
    config_value_t* cli_config_name = NULL;
    cfgmgr_interface_t* ctx = NULL;
    config_value_t* client_interface = NULL;
    ctx = cfgmgr_interface_initialize();
    if (ctx == NULL) {
        LOG_ERROR_0("cfgmgr initialization failed");
        goto err;
    }

    // Fetching list of client interfaces
    client_interface = app_interface->get_config_value(app_interface->cfg, CLIENTS);
    if (client_interface == NULL) {
        LOG_ERROR_0("client_interface initialization failed");
        goto err;
    }

    size_t arr_len = config_value_array_len(client_interface);
    if(arr_len == 0){
        LOG_ERROR_0("Empty array is not supported, atleast one value should be given.");
        goto err;
    }
    // Iterating through available client configs
    for (int i = 0; i < arr_len; i++) {
        // Fetch name of individual client config
        config_value_t* cli_config = config_value_array_get(client_interface, i);
        if (cli_config == NULL) {
            LOG_ERROR_0("cli_config initialization failed");
            goto err;
        }
        cli_config_name = config_value_object_get(cli_config, "Name");
        if (cli_config_name == NULL) {
            LOG_ERROR_0("cli_config_name initialization failed");
            goto err;
        }
        // Verifying client config with name exists
        strcmp_s(cli_config_name->body.string, strlen(cli_config_name->body.string), name, &ret);
        if (ret == 0) {
            ctx->interface = cli_config;
            ctx->cfg_mgr = cfgmgr;
            ctx->type = CFGMGR_CLIENT;
            goto err;
        } else if (i == config_value_array_len(client_interface)) {
            LOG_ERROR("Clients by name %s not found", name);
            if (cli_config != NULL) {
                config_value_destroy(cli_config);
            }
            goto err;
        } else {
            if (cli_config_name != NULL) {
                config_value_destroy(cli_config_name);
            }
            if (cli_config != NULL) {
                config_value_destroy(cli_config);
            }
        }
    }
err:
   if (cli_config_name != NULL) {
        config_value_destroy(cli_config_name);
    }
    if (client_interface != NULL) {
        config_value_destroy(client_interface);
    }
    return ctx;
}

cfgmgr_interface_t* cfgmgr_get_client_by_index(cfgmgr_ctx_t* cfgmgr, int index) {
    LOG_INFO_0("cfgmgr_get_client_by_index method");
    config_t* app_interface = cfgmgr->app_interface;
    config_value_t* client_interface = NULL;
    config_value_t* cli_config_index = NULL;
    cfgmgr_interface_t* ctx = NULL;
    ctx = cfgmgr_interface_initialize();
    if (ctx == NULL) {
        LOG_ERROR_0("cfgmgr initialization failed");
        goto err;
    }

    // Fetching list of Clients interfaces
    client_interface = app_interface->get_config_value(app_interface->cfg, CLIENTS);
    if (client_interface == NULL) {
        LOG_ERROR_0("client_interface initialization failed");
        goto err;
    }

    // Fetch client config associated with index
    cli_config_index = config_value_array_get(client_interface, index);
    if (cli_config_index == NULL) {
        LOG_ERROR_0("cli_config_index initialization failed");
        goto err;
    }

    ctx->interface = cli_config_index;
    ctx->cfg_mgr = cfgmgr;
    ctx->type = CFGMGR_CLIENT;

    if (client_interface != NULL) {
        config_value_destroy(client_interface);
    }

    return ctx;

err:
    if (client_interface != NULL) {
        config_value_destroy(client_interface);
    }
    if (cli_config_index != NULL) {
        config_value_destroy(cli_config_index);
    }
    if (ctx != NULL) {
        cfgmgr_interface_destroy(ctx);
    }
    return NULL;
}

config_value_t* cfgmgr_get_endpoint(cfgmgr_interface_t* ctx) {
    config_value_t* pub_config = ctx->interface;
    if (pub_config == NULL) {
        LOG_ERROR_0("pub_config initialization failed");
        return NULL;
    }
    // Fetching EndPoint from config
    config_value_t* end_point = config_value_object_get(pub_config, "EndPoint");
    if (end_point == NULL) {
        LOG_ERROR_0("end_point initialization failed");
        return NULL;
    }
    return end_point;
}

config_value_t* cfgmgr_get_topics(cfgmgr_interface_t* ctx) {
    if (ctx->type == CFGMGR_SERVER || ctx->type == CFGMGR_CLIENT) {
        LOG_ERROR_0("cfgmgr_get_topics not applicable for CFGMGR_SERVER/CFGMGR_CLIENT");
        return NULL;
    }
    config_value_t* config = ctx->interface;
    cJSON *arr = NULL;

    if (config == NULL) {
        LOG_ERROR_0("config initialization failed");
        return NULL;
    }
    config_value_t* topics = config_value_object_get(ctx->interface, "Topics");
    if (topics == NULL) {
        LOG_ERROR_0("topics initialization failed");
        return NULL;
    }

    if (topics->type != CVT_ARRAY) {
        LOG_ERROR_0("Topics type mismatch, it should be array");
        return NULL;
    }

    arr = cJSON_CreateArray();
    if (arr == NULL) {
        LOG_ERROR_0("arr initialization failed");
        return NULL;
    }
    int ret;

    size_t arrlen = config_value_array_len(topics);
    if (arrlen == 0) {
        LOG_ERROR_0("Empty String is not supported in Topics. Atleast one topic is required");
        return NULL;
    }

    config_value_t* topic_value = NULL;
    topic_value = config_value_array_get(topics, 0);
    if (topic_value == NULL) {
        LOG_ERROR_0("Extracting first topic from topics array failed ");
        return NULL;
    }

    // If only one item in Topics and it is *,
    // then add empty string in order to allow all clients to subscribe
    strcmp_s(topic_value->body.string, strlen(topic_value->body.string), "*", &ret);

    if((arrlen == 1) && (ret == 0 )){
        cJSON_AddItemToArray(arr, cJSON_CreateString(""));
        config_value_destroy(topics);
        topics = config_value_new_array(
                (void*) arr , cJSON_GetArraySize(arr), get_array_item, NULL);
        if (topics == NULL){
            LOG_ERROR_0("config value new array for topic failed");
            return NULL;
        }
        return topics;
    }

    if (topic_value != NULL) {
        config_value_destroy(topic_value);
    }

    if (arr != NULL) {
        cJSON_free(arr);
    }

    return topics;
}

bool cfgmgr_set_topics(cfgmgr_interface_t* ctx, const char** topics_list, int len) {
    if (ctx->type == CFGMGR_SERVER || ctx->type == CFGMGR_CLIENT) {
        LOG_ERROR_0("cfgmgr_set_topics not applicable for CFGMGR_SERVER/CFGMGR_CLIENT");
        return NULL;
    }
    char* type = "";
    if (ctx->type == CFGMGR_PUBLISHER) {
        type = PUBLISHERS;
    } else if (ctx->type == CFGMGR_SUBSCRIBER) {
        type = SUBSCRIBERS;
    }

    int ret_val = 0;
    // Fetching the name of pub/sub interface
    config_value_t* config_name = config_value_object_get(ctx->interface, "Name");
    if (config_name == NULL) {
        LOG_ERROR_0("config_name initialization failed");
        goto err;
    }

    // Constructing cJSON object from obtained interface
    cJSON* temp = (cJSON*)ctx->cfg_mgr->app_interface->cfg;
    if (temp == NULL) {
        LOG_ERROR_0("cJSON temp object is NULL");
        ret_val = -1;
        goto err;
    }

    // Creating cJSON object of new topics
    cJSON* obj = cJSON_CreateArray();
    if (obj == NULL) {
        LOG_ERROR_0("cJSON obj initialization failed");
        ret_val = -1;
        goto err;
    }
    for (int i = 0; i < len; i++) {
        cJSON_AddItemToArray(obj, cJSON_CreateString(topics_list[i]));
    }

    // Fetching list of publishers/subscribers
    cJSON* config_list = cJSON_GetObjectItem(temp, type);
    if (config_list == NULL) {
        LOG_ERROR_0("cJSON config_list initialization failed");
        ret_val = -1;
        goto err;
    }

    // Iterating & validating name
    int config_size = cJSON_GetArraySize(config_list);
    for (int i = 0; i < config_size; i++) {
        cJSON* config_list_name = cJSON_GetArrayItem(config_list, i);
        if (config_list_name == NULL) {
            LOG_ERROR_0("config_list_name initialization failed");
            ret_val = -1;
            goto err;
        }
        char* name_to_check = cJSON_GetStringValue(cJSON_GetObjectItem(config_list_name, "Name"));
        if (name_to_check == NULL) {
            LOG_ERROR_0("name_to_check initialization failed");
            ret_val = -1;
            goto err;
        }
        // Replace topics if name matches
        if (strcmp(name_to_check, config_name->body.string) == 0) {
            cJSON_ReplaceItemInObject(config_list_name, "Topics", obj);
        }
    }

err:
    if (config_name != NULL) {
        config_value_destroy(config_name);
    }
    return ret_val;
}

config_value_t* cfgmgr_get_allowed_clients(cfgmgr_interface_t* ctx) {
    if (ctx->type == CFGMGR_SUBSCRIBER || ctx->type == CFGMGR_CLIENT) {
        LOG_ERROR_0("cfgmgr_get_allowed_clients not applicable for CFGMGR_SUBSCRIBER/CFGMGR_CLIENT");
        return NULL;
    }
    config_value_t* config = ctx->interface;
    if (config == NULL) {
        LOG_ERROR_0("config initialization failed");
        return NULL;
    }
    config_value_t* clients = config_value_object_get(config, "AllowedClients");
    if (clients == NULL) {
        LOG_ERROR_0("topics initialization failed");
        return NULL;
    }
    if (clients->type != CVT_ARRAY) {
        LOG_ERROR_0("Allowed Clients type mismatch, it should be array");
        return NULL;
    }
    return clients;
}

config_t* cfgmgr_get_msgbus_config_pub(cfgmgr_interface_t* ctx) {
    config_t* m_config = NULL;
    config_value_t* broker_app_name = NULL;
    char** host_port = NULL;
    char* host = NULL;
    char* port = NULL;
    // Initializing base_cfg variables
    config_value_t* pub_config = ctx->interface;
    char* app_name = ctx->cfg_mgr->app_name;
    int dev_mode = ctx->cfg_mgr->dev_mode;
    kv_store_client_t* kv_store_client = ctx->cfg_mgr->kv_store_client;
    void* kv_store_handle = ctx->cfg_mgr->kv_store_handle;
    config_value_t* publish_config_type = NULL;
    config_value_t* publish_config_endpoint = NULL;
    config_value_t* publish_config_name = NULL;
    char* config_value_cr = NULL;

    // Creating cJSON object
    cJSON* c_json = cJSON_CreateObject();
    if (c_json == NULL) {
        LOG_ERROR_0("c_json initialization failed");
        goto err;
    }

    // Fetching Type from config
    publish_config_type = config_value_object_get(pub_config, TYPE);
    if (publish_config_type == NULL) {
        LOG_ERROR_0("publish_config_type initialization failed");
        goto err;
    }

    if(publish_config_type->type != CVT_STRING || publish_config_type->body.string == NULL){
        LOG_ERROR_0("publish_config_type type mismatch or the string value is NULL");
        goto err;
    }

    char* type = publish_config_type->body.string;

    // Fetching EndPoint from config
    publish_config_endpoint = config_value_object_get(pub_config, ENDPOINT);
    if (publish_config_endpoint == NULL) {
        LOG_ERROR_0("publish_config_endpoint initialization failed");
        goto err;
    }

    const char* end_point;
    if(publish_config_endpoint->type == CVT_OBJECT){
        end_point = cvt_to_char(publish_config_endpoint);
    }else if(publish_config_endpoint->type == CVT_STRING && publish_config_endpoint->body.string != NULL){
        end_point = publish_config_endpoint->body.string;
    } else {
        end_point = NULL;
    }

    if(end_point == NULL) {
        LOG_ERROR_0("Failed to get endpoint, its NULL");
        goto err;
    }

    // Fetching Name from config
    publish_config_name = config_value_object_get(pub_config, NAME);
    if (publish_config_name == NULL) {
        LOG_ERROR_0("publish_config_name initialization failed");
        goto err;
    }

    if(publish_config_name->type != CVT_STRING || publish_config_name->body.string == NULL){
        LOG_ERROR_0("publish_config_name type mismatch or the string value is NULL");
        goto err;
    }

    // Overriding endpoint with PUBLISHER_<Name>_ENDPOINT if set
    size_t init_len = strlen("PUBLISHER_") + strlen(publish_config_name->body.string) + strlen("_ENDPOINT") + 2;
    char* ep_override_env = concat_s(init_len, 3, "PUBLISHER_", publish_config_name->body.string, "_ENDPOINT");
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
    free(ep_override_env);

    // Overriding endpoint with PUBLISHER_ENDPOINT if set
    // Note: This overrides all the publisher endpoints if set
    char* publisher_ep = getenv("PUBLISHER_ENDPOINT");
    if (publisher_ep != NULL) {
        LOG_DEBUG_0("Overriding endpoint with PUBLISHER_ENDPOINT");
        if (strlen(publisher_ep) != 0) {
            end_point = (const char*)publisher_ep;
        }
    } else {
        LOG_DEBUG_0("env not set for overridding PUBLISHER_ENDPOINT, and hence endpoint taking from interface ");
    }

    // Overriding endpoint with PUBLISHER_<Name>_TYPE if set
    init_len = strlen("PUBLISHER_") + strlen(publish_config_name->body.string) + strlen("_TYPE") + 2;
    char* type_override_env = concat_s(init_len, 3, "PUBLISHER_", publish_config_name->body.string, "_TYPE");
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
        LOG_DEBUG("env not set for overridding %s, and hence type taking from interface ", type_override_env);
    }

    free(type_override_env);

    // Overriding endpoint with PUBLISHER_TYPE if set
    // Note: This overrides all the publisher type if set
    char* publisher_type = getenv("PUBLISHER_TYPE");
    if (publisher_type != NULL) {
        LOG_DEBUG_0("Overriding endpoint with PUBLISHER_TYPE");
        if (strlen(publisher_type) != 0) {
            type = publisher_type;
        }
    } else {
        LOG_DEBUG("env not set for overridding PUBLISHER_TYPE, and hence type taking from interface ");
    }

    cJSON_AddStringToObject(c_json, "type", type);
    // TODO: type has to be freed in destructor
    // free(type);

    // Adding zmq_recv_hwm value if available
    config_value_t* zmq_recv_hwm_value = config_value_object_get(pub_config, ZMQ_RECV_HWM);
    if (zmq_recv_hwm_value != NULL) {
        if (zmq_recv_hwm_value->type != CVT_INTEGER) {
            LOG_ERROR_0("zmq_recv_hwm type is not integer");
            goto err;
        }
        cJSON_AddNumberToObject(c_json, ZMQ_RECV_HWM, zmq_recv_hwm_value->body.integer);
    }

    // Adding brokered value if available
    config_value_t* brokered_value = config_value_object_get(pub_config, BROKERED);
    if (brokered_value != NULL) {
        if (brokered_value->type != CVT_BOOLEAN) {
            LOG_ERROR_0("brokered_value type is not boolean");
            goto err;
        }
    }

    if (!strcmp(type, "zmq_ipc")) {
        bool ret = get_ipc_config(c_json, pub_config, end_point, CFGMGR_PUBLISHER);
        if (ret == false){
            LOG_ERROR_0("IPC configuration for publisher failed");
            goto err;
        }
    } else if (!strcmp(type, "zmq_tcp")) {
        // Add host & port to zmq_tcp_publish cJSON object
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
        cJSON* zmq_tcp_publish = cJSON_CreateObject();
        if (zmq_tcp_publish == NULL) {
            LOG_ERROR_0("zmq_tcp_publish initialization failed");
            goto err;
        }
        cJSON_AddStringToObject(zmq_tcp_publish, "host", host);
        cJSON_AddNumberToObject(zmq_tcp_publish, "port", i_port);
        if (brokered_value != NULL) {
            if (brokered_value->body.boolean) {
                cJSON_AddBoolToObject(zmq_tcp_publish, BROKERED, true);
            } else {
                cJSON_AddBoolToObject(zmq_tcp_publish, BROKERED, false);
            }
        }
        cJSON_AddItemToObject(c_json, "zmq_tcp_publish", zmq_tcp_publish);
        if (dev_mode != 0) {
            bool ret_val;

            // Checking if Publisher is using Broker to publish or not and constructing
            // the publisher messagebus config based on the check. 
            broker_app_name = config_value_object_get(pub_config, BROKER_APPNAME);
            if(broker_app_name != NULL){

                if(broker_app_name->type != CVT_STRING){
                    LOG_ERROR_0("[Type Missmatch]: BrokerAppName should be of type String");
                    goto err;
                }
                // If a publisher is using broker to communicate with its respective subscriber
                // then publisher will act as a subscriber to X-SUB, hence publishers 
                // message bus config looks like a subscriber one.  
                ret_val = add_keys_to_config(zmq_tcp_publish, app_name, kv_store_client, kv_store_handle, broker_app_name, pub_config);
                if(!ret_val) {
                    LOG_ERROR_0("Failed to add respective cert keys");
                    goto err;
                }
            } else{
                ret_val = construct_tcp_publisher_prod(app_name, c_json, zmq_tcp_publish, kv_store_handle, pub_config, kv_store_client);
                if(!ret_val) {
                    LOG_ERROR_0("Failed to construct tcp config struct");
                    goto err;
                }
            }
        }
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
    LOG_DEBUG("Env publisher Config is : %s \n", config_value_cr);

    // Constructing config_t object from cJSON object
    m_config = config_new(
            (void*) c_json, free_json, get_config_value);
    if (m_config == NULL) {
        LOG_ERROR_0("Failed to initialize configuration object");
        goto err;
    }

err:
    if (config_value_cr != NULL) {
        free(config_value_cr);
    }
    if (publish_config_type != NULL) {
        config_value_destroy(publish_config_type);
    }
    if (publish_config_name != NULL) {
        config_value_destroy(publish_config_name);
    }
    if (publish_config_endpoint != NULL) {
        config_value_destroy(publish_config_endpoint);
    }
    if (broker_app_name != NULL) {
        config_value_destroy(broker_app_name);
    }
    if (host_port != NULL) {
        free_mem(host_port);
    }
    return m_config;
}

config_t* cfgmgr_get_msgbus_config_sub(cfgmgr_interface_t* ctx) {
    char** host_port = NULL;
    char* host = NULL;
    char* port = NULL;
    // Initializing base_cfg variables
    config_value_t* sub_config = ctx->interface;
    char* app_name = ctx->cfg_mgr->app_name;
    bool dev_mode = false;
    config_t* m_config = NULL;
    config_value_t* topic_array = NULL;
    config_value_t* publisher_appname = NULL;
    config_value_t* topic = NULL;
    char* config_value_cr = NULL;
    config_value_t* subscribe_config_name = NULL;
    config_value_t* subscribe_config_endpoint = NULL;
    char* type_override_env = NULL;
    char* type_override = NULL;
    config_value_t* subscribe_config_type = NULL;
    char* ep_override_env = NULL;
    char* ep_override = NULL;
    config_value_t* zmq_recv_hwm_value = NULL;

    int devmode = ctx->cfg_mgr->dev_mode;
    if(devmode == 0) {
        dev_mode = true;
    }
 
    kv_store_client_t* kv_store_client = ctx->cfg_mgr->kv_store_client;
    void* kv_store_handle = ctx->cfg_mgr->kv_store_handle;
    // Creating cJSON object
    cJSON* c_json = cJSON_CreateObject();
    if (c_json == NULL) {
        LOG_ERROR_0("c_json initialization failed");
        goto err;
    }

    // Fetching Type from config
    subscribe_config_type = config_value_object_get(sub_config, TYPE);
    if (subscribe_config_type == NULL || subscribe_config_type->body.string == NULL) {
        LOG_ERROR_0("subscribe_config_type initialization failed");
        goto err;
    }

    if(subscribe_config_type->type != CVT_STRING || subscribe_config_type->body.string == NULL){
        LOG_ERROR_0("subscribe_config_type type mismatch or the string value is NULL");
        goto err;
    }
    char* type = subscribe_config_type->body.string;

    // Fetching EndPoint from config
    subscribe_config_endpoint = config_value_object_get(sub_config, ENDPOINT);
    if (subscribe_config_endpoint == NULL) {
        LOG_ERROR_0("subscribe_config_endpoint initialization failed");
        goto err;
    }

    const char* end_point = NULL;
    if(subscribe_config_endpoint->type == CVT_OBJECT){
        end_point = cvt_to_char(subscribe_config_endpoint);
    }else{
        end_point = subscribe_config_endpoint->body.string;
    }

    if(end_point == NULL){
        LOG_ERROR_0("Failed to get endpoint, its NULL");
        goto err;
    }

    // Fetching Name from config
    subscribe_config_name = config_value_object_get(sub_config, NAME);
    if (subscribe_config_name == NULL) {
        LOG_ERROR_0("subscribe_config_name initialization failed");
        goto err;
    }

    // Overriding endpoint with SUBSCRIBER_<Name>_ENDPOINT if set
    size_t init_len = strlen("SUBSCRIBER_") + strlen(subscribe_config_name->body.string) + strlen("_ENDPOINT") + 2;
    ep_override_env = concat_s(init_len, 3, "SUBSCRIBER_", subscribe_config_name->body.string, "_ENDPOINT");
    if (ep_override_env == NULL) {
        LOG_ERROR_0("concatenation for ep_override_env failed");
        goto err;
    }
    ep_override = getenv(ep_override_env);   
    if (ep_override != NULL) {
        if (strlen(ep_override) != 0) {
            LOG_DEBUG("Overriding endpoint with %s", ep_override_env);
            end_point = (const char*)ep_override;
        }
    } else {
        LOG_DEBUG("env not set for overridding %s, and hence endpoint taking from interface ", ep_override_env);
    }

    // Overriding endpoint with SUBSCRIBER_ENDPOINT if set
    // Note: This overrides all the subscriber type if set
    char* subscriber_ep = getenv("SUBSCRIBER_ENDPOINT");
    if (subscriber_ep != NULL) {
        LOG_DEBUG_0("Overriding endpoint with SUBSCRIBER_ENDPOINT");
        if (strlen(subscriber_ep) != 0) {
            end_point = (const char*)subscriber_ep;
        }
    } else {
        LOG_DEBUG_0("env not set for overridding SUBSCRIBER_ENDPOINT, and hence endpoint taking from interface ");
    }

    // Overriding endpoint with SUBSCRIBER_<Name>_TYPE if set
    init_len = strlen("SUBSCRIBER_") + strlen(subscribe_config_name->body.string) + strlen("_TYPE") + 2;
    type_override_env = concat_s(init_len, 3, "SUBSCRIBER_", subscribe_config_name->body.string, "_TYPE");
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

    // Overriding endpoint with SUBSCRIBER_TYPE if set
    // Note: This overrides all the subscriber endpoints if set
    char* subscriber_type = getenv("SUBSCRIBER_TYPE");
    if (subscriber_type != NULL) {
        LOG_DEBUG_0("Overriding endpoint with SUBSCRIBER_TYPE");
        if (strlen(subscriber_type) != 0) {
            type = subscriber_type;
        }
    } else {
        LOG_DEBUG("env not set for overridding SUBSCRIBER_TYPE, and hence type taking from interface ");
    }
    cJSON_AddStringToObject(c_json, "type", type);

    // Adding zmq_recv_hwm value if available
    zmq_recv_hwm_value = config_value_object_get(sub_config, ZMQ_RECV_HWM);
    if (zmq_recv_hwm_value != NULL) {
        if (zmq_recv_hwm_value->type != CVT_INTEGER) {
            LOG_ERROR_0("zmq_recv_hwm type is not integer");
            goto err;
        }
        cJSON_AddNumberToObject(c_json, ZMQ_RECV_HWM, zmq_recv_hwm_value->body.integer);
    }

    if(!strcmp(type, "zmq_ipc")) {
        bool ret = get_ipc_config(c_json, sub_config, end_point, CFGMGR_SUBSCRIBER);
        if (ret == false) {
            LOG_ERROR_0("IPC configuration for subscriber failed");
            goto err;
        }
    } else if(!strcmp(type, "zmq_tcp")) {

        // Fetching Topics from config
        topic_array = config_value_object_get(sub_config, TOPICS);
        if (topic_array == NULL) {
            LOG_ERROR_0("topic_array initialization failed");
            goto err;
        }

        size_t arr_len = config_value_array_len(topic_array);
        if(arr_len == 0){
            LOG_ERROR_0("Empty array is not supported, atleast one value should be given.");
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
        int64_t i_port = atoi(port);
        
        int ret;
        int topicret;
        // comparing the first topic in the array of subscribers topic with "*"
        topic = config_value_array_get(topic_array, 0);
        if (topic == NULL || topic->body.string == NULL){
            LOG_ERROR_0("topic initialization failed");
            goto err;
        }
        strcmp_s(topic->body.string, strlen(topic->body.string), "*", &topicret);
        if (topic != NULL) {
            config_value_destroy(topic);
        }

        publisher_appname = config_value_object_get(sub_config, PUBLISHER_APPNAME);
        if (publisher_appname == NULL) {
            LOG_ERROR("%s initialization failed", PUBLISHER_APPNAME);
            goto err;
        }

        if(publisher_appname->type != CVT_STRING || publisher_appname->body.string == NULL){
            LOG_ERROR("PublisherAppName type mismatch or the string is NULL");
            goto err;
        }

        for (int i = 0; i < arr_len; i++) {
            // Create cJSON object for every topic
            cJSON* topics = cJSON_CreateObject();
            if (topics == NULL) {
                LOG_ERROR_0("c_json initialization failed");
                goto err;
            }
            topic = config_value_array_get(topic_array, i);
            if (topic == NULL){
                LOG_ERROR_0("topic initialization failed");
                goto err;
            }
            // Add host & port to cJSON object
            cJSON_AddStringToObject(topics, "host", host);
            cJSON_AddNumberToObject(topics, "port", i_port);

            // if topics lenght is not 1 or the topic is not equal to "*",
            //then we are adding that topic for subscription.
            if(!dev_mode) {
                bool ret_val;
                // This is EISZmqBroker usecase, where in "PublisherAppname" will be specified as "*"
                // hence comparing for "PublisherAppname" and "*"
                strcmp_s(publisher_appname->body.string, strlen(publisher_appname->body.string), "*", &ret);
                if(ret == 0) {
                    // In case of EISZmqBroker, it is "X-SUB" which needs "publishers" way of
                    // messagebus config, hence calling "construct_tcp_publisher_prod()" function
                    ret_val = construct_tcp_publisher_prod(app_name, c_json, topics, kv_store_handle, sub_config, kv_store_client);
                     if(!ret_val) {
                        LOG_ERROR_0("Failed in construct_tcp_publisher_prod()");
                        goto err;
                    }
                }else {
                    ret_val = add_keys_to_config(topics, app_name, kv_store_client, kv_store_handle, publisher_appname, sub_config);
                    if(!ret_val) {
                        LOG_ERROR_0("Failed in add_keys_to_config()");
                        goto err;
                    }
                }
            }
            if((arr_len == 1) && (topicret == 0)) {
                cJSON_AddItemToObject(c_json, "", topics);
            } else {
                cJSON_AddItemToObject(c_json, topic->body.string, topics);
            }
            if (topic != NULL) {
                config_value_destroy(topic);
            }
        }
    } else {
        LOG_ERROR_0("Type should be either \"zmq_ipc\" or \"zmq_tcp\"");
        goto err;
    }

    // Constructing char* object from cJSON object
    config_value_cr = cJSON_Print(c_json);
    if (config_value_cr == NULL) {
        LOG_ERROR_0("c_json initialization failed");
        goto err;
    }
    LOG_DEBUG("Env subscriber Config is : %s \n", config_value_cr);

    // Constructing config_t object from cJSON object
    m_config = config_new(
            (void*) c_json, free_json, get_config_value);
    if (m_config == NULL) {
        LOG_ERROR_0("Failed to initialize configuration object");
        goto err;
    }
err:
    if (host_port != NULL) {
        free_mem(host_port);
    }
    if (subscribe_config_type != NULL) {
        config_value_destroy(subscribe_config_type);
    }
    if (subscribe_config_endpoint != NULL) {
        config_value_destroy(subscribe_config_endpoint);
    }
    if (subscribe_config_name != NULL) {
        config_value_destroy(subscribe_config_name);
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
    if (topic_array != NULL) {
        config_value_destroy(topic_array);
    }
    if (publisher_appname != NULL) {
        config_value_destroy(publisher_appname);
    }
    if (config_value_cr != NULL) {
        free(config_value_cr);
    }
    return m_config;
}

config_t* cfgmgr_get_msgbus_config_server(cfgmgr_interface_t* ctx) {
    // Initializing base_cfg variables
    config_value_t* serv_config = ctx->interface;
    char* app_name = ctx->cfg_mgr->app_name;
    int dev_mode = ctx->cfg_mgr->dev_mode;
    kv_store_client_t* kv_store_client = ctx->cfg_mgr->kv_store_client;
    void* kv_store_handle = ctx->cfg_mgr->kv_store_handle;
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
        bool ret = get_ipc_config(c_json, serv_config, end_point, CFGMGR_SERVER);
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
                pub_key_values = kv_store_client->get_prefix(kv_store_handle, "/Publickeys/");
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
                    const char* client_public_key = kv_store_client->get(kv_store_handle, grab_public_key);
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
            server_secret_key = kv_store_client->get(kv_store_handle, pub_pri_key);
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

config_t* cfgmgr_get_msgbus_config_client(cfgmgr_interface_t* ctx) {
    // Initializing base_cfg variables
    config_value_t* cli_config = ctx->interface;
    char* app_name = ctx->cfg_mgr->app_name;
    int dev_mode = ctx->cfg_mgr->dev_mode;
    kv_store_client_t* kv_store_client = ctx->cfg_mgr->kv_store_client;
    void* kv_store_handle = ctx->cfg_mgr->kv_store_handle;
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
        bool ret = get_ipc_config(c_json, cli_config, end_point, CFGMGR_CLIENT);
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
            char* server_public_key = kv_store_client->get(kv_store_handle, retreive_server_pub_key);
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
            sub_public_key = kv_store_client->get(kv_store_handle, s_client_public_key);
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
            sub_pri_key = kv_store_client->get(kv_store_handle, s_client_pri_key);
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

config_t* cfgmgr_get_msgbus_config(cfgmgr_interface_t* ctx) {
    config_t* config;
    if (ctx->type == CFGMGR_PUBLISHER) {
        config = cfgmgr_get_msgbus_config_pub(ctx);
    } else if (ctx->type == CFGMGR_SUBSCRIBER) {
        config = cfgmgr_get_msgbus_config_sub(ctx);
    } else if (ctx->type == CFGMGR_SERVER) {
        config = cfgmgr_get_msgbus_config_server(ctx);
    } else if (ctx->type == CFGMGR_CLIENT) {
        config = cfgmgr_get_msgbus_config_client(ctx);
    }
    return config;
}

bool cfgmgr_is_dev_mode(cfgmgr_ctx_t* cfgmgr) {
    // Fetching dev mode from cfgmgr
    int result = cfgmgr->dev_mode;
    if (result == 0) {
        return true;
    }
    return false;
}

config_value_t* cfgmgr_get_appname(cfgmgr_ctx_t* cfgmgr) {
    // Fetching app name from cfgmgr
    config_value_t* app_name = config_value_new_string(cfgmgr->app_name);
    if (app_name == NULL) {
        LOG_ERROR_0("Fetching appname failed");
        return NULL;
    }
    return app_name;
}

int cfgmgr_get_num_elements(config_t* config, const char* key) {
    // Fetching list of interface elements
    config_value_t* interfaces = config_get(config, key);
    if (interfaces == NULL) {
        LOG_ERROR_0("Failed to fetch number of elements in interface");
        return -1;
    }
    if (interfaces->type != CVT_ARRAY) {
        LOG_ERROR("%s interface is not an array type", key);
        return -1;
    }
    int result = config_value_array_len(interfaces);
    config_value_destroy(interfaces);
    return result;
}

int cfgmgr_get_num_publishers(cfgmgr_ctx_t* cfgmgr) {
    int result = cfgmgr_get_num_elements(cfgmgr->app_interface, PUBLISHERS);
    return result;
}

int cfgmgr_get_num_subscribers(cfgmgr_ctx_t* cfgmgr) {
    int result = cfgmgr_get_num_elements(cfgmgr->app_interface, SUBSCRIBERS);
    return result;
}

int cfgmgr_get_num_servers(cfgmgr_ctx_t* cfgmgr) {
    int result = cfgmgr_get_num_elements(cfgmgr->app_interface, SERVERS);
    return result;
}

int cfgmgr_get_num_clients(cfgmgr_ctx_t* cfgmgr) {
    int result = cfgmgr_get_num_elements(cfgmgr->app_interface, CLIENTS);
    return result;
}

config_t* cfgmgr_get_app_config(cfgmgr_ctx_t* cfgmgr) {
    return cfgmgr->app_config;
}

config_t* cfgmgr_get_app_interface(cfgmgr_ctx_t* cfgmgr) {
    return cfgmgr->app_interface;
}

config_value_t* cfgmgr_get_app_config_value(cfgmgr_ctx_t* cfgmgr, const char* key) {
    return cfgmgr->app_config->get_config_value(cfgmgr->app_config->cfg, key);
}

config_value_t* cfgmgr_get_app_interface_value(cfgmgr_ctx_t* cfgmgr, const char* key) {
    return cfgmgr->app_interface->get_config_value(cfgmgr->app_interface->cfg, key);
}

config_value_t* cfgmgr_get_interface_value(cfgmgr_interface_t* cfgmgr_interface, const char* key) {
    config_value_t* interface_value = config_value_object_get(cfgmgr_interface->interface, key);
    return interface_value;
}

void cfgmgr_watch(cfgmgr_ctx_t* cfgmgr, const char* key, cfgmgr_watch_callback_t watch_callback, void* user_data) {
    // Calling the base watch API
    cfgmgr->kv_store_client->watch(cfgmgr->kv_store_handle, key, watch_callback, user_data);
    return;
}

void cfgmgr_watch_prefix(cfgmgr_ctx_t* cfgmgr, char* prefix, cfgmgr_watch_callback_t watch_callback, void* user_data) {
    // Calling the base watch_prefix API
    cfgmgr->kv_store_client->watch_prefix(cfgmgr->kv_store_handle, prefix, watch_callback, user_data);
    return;
}

cfgmgr_ctx_t* cfgmgr_initialize() {
    int result = 0;
    config_t* app_config = NULL;
    char* interface = NULL;
    config_t* app_interface = NULL;
    char* value = NULL;
    char* c_app_name = NULL;
    char* interface_char = NULL;
    char* config_char = NULL;
    char* env_var = NULL;
    kv_store_client_t* kv_store_client = NULL;
    config_t* kv_store_config = NULL;
    char* dev_mode_var = NULL;
    char* app_name_var = NULL;

    cfgmgr_ctx_t *cfg_mgr = (cfgmgr_ctx_t *)malloc(sizeof(cfgmgr_ctx_t));
    if (cfg_mgr == NULL) {
        LOG_ERROR_0("Malloc failed for cfgmgr_ctx_t");
        goto err;
    }
    // Setting app_cfg->env_var to NULL initially
    cfg_mgr->env_var = NULL;

    // Fetching & intializing dev mode variable
    dev_mode_var = getenv("DEV_MODE");
    if (dev_mode_var != NULL) {
        to_lower(dev_mode_var);
        strcmp_s(dev_mode_var, strlen(dev_mode_var), "true", &result);
    } else {
        LOG_ERROR_0("DEV_MODE variable not set");
        goto err;
    }

    kv_store_config = create_kv_store_config();
    if (kv_store_config == NULL) {
        LOG_ERROR_0("kv_store_config initialization failed");
        goto err;
    }
    // Creating kv store client instance
    kv_store_client = create_kv_client(kv_store_config);
    if (kv_store_client == NULL) {
        LOG_ERROR_0("kv_store_client is NULL");
        goto err;
    }

    // Initializing etcd client handle
    void *handle = kv_store_client->init(kv_store_client);
    if (handle == NULL) {
        LOG_ERROR_0("ConfigMgr handle initialization failed");
        goto err;
    }

    // Fetching GlobalEnv
    env_var = kv_store_client->get(handle, "/GlobalEnv/");
    if (env_var == NULL) {
        LOG_WARN_0("Value is not found for the key /GlobalEnv/,"
                   " continuing without setting GlobalEnv vars");
    } else {
        // Creating cJSON of /GlobalEnv/ to iterate over a loop
        cJSON* env_json = cJSON_Parse(env_var);
        if (env_json == NULL) {
            LOG_ERROR("Error when parsing JSON: %s", cJSON_GetErrorPtr());
            goto err;
        }
        int env_vars_count = cJSON_GetArraySize(env_json);
        // Looping over env vars and setting them in env
        for (int i = 0; i < env_vars_count; i++) {
            cJSON *temp = cJSON_GetArrayItem(env_json, i);
            int env_set = setenv(temp->string, temp->valuestring, 1);
            if (env_set != 0) {
                LOG_ERROR("Failed to set env %s", temp->string);
                goto err;
            }
        }
        //TODO : Go and python making use of ths variable
        // We need to allocate and find a suitable place to
        // free. As it is used across C++ & C APIs.
        //free(env_var);
        cJSON_Delete(env_json);
    }

    // Fetching AppName
    app_name_var = getenv("AppName");
    if (app_name_var == NULL) {
        LOG_ERROR_0("AppName env not set");
        goto err;
    }
    size_t str_len = strlen(app_name_var) + 1;
    c_app_name = (char*)malloc(sizeof(char) * str_len);
    if (c_app_name == NULL) {
        LOG_ERROR_0("c_app_name is NULL");
        goto err;
    }
    int ret = snprintf(c_app_name, str_len, "%s", app_name_var);
    if (ret < 0){
        LOG_ERROR_0("snprintf failed to c_app_name");
        goto err;
    }
    LOG_DEBUG("AppName: %s", c_app_name);
    trim(c_app_name);

    // Fetching App interfaces
    size_t init_len = strlen("/") + strlen(c_app_name) + strlen("/interfaces") + 1;
    interface_char = concat_s(init_len, 3, "/", c_app_name, "/interfaces");
    if (interface_char == NULL){
        LOG_ERROR_0("Concatenation of /appname and /interfaces failed");
        goto err;
    }

    // Fetching App config
    init_len = strlen("/") + strlen(c_app_name) + strlen("/config") + 1;
    config_char = concat_s(init_len, 3, "/", c_app_name, "/config");
    if (config_char == NULL){
        LOG_ERROR_0("Concatenation of /appname and /config failed");
        goto err;
    }

    LOG_DEBUG("interface_char: %s", interface_char);
    LOG_DEBUG("config_char: %s", config_char);

    interface = kv_store_client->get(handle, interface_char);
    if (interface == NULL) {
        LOG_ERROR("Value is not found for the key: %s", interface_char);
        goto err;
    }

    value = kv_store_client->get(handle, config_char);
    if (value == NULL) {
        LOG_ERROR("Value is not found for the key: %s", config_char);
        goto err;
    }

    app_config = json_config_new_from_buffer(value);
    if (app_config == NULL) {
        LOG_ERROR_0("app_config initialization failed");
        goto err;
    }

    app_interface = json_config_new_from_buffer(interface);
    if (app_interface == NULL) {
        LOG_ERROR_0("app_interface initialization failed");
        goto err;
    }

    if (c_app_name != NULL) {
        cfg_mgr->app_name = c_app_name;
    }
    if (kv_store_client != NULL) {
        cfg_mgr->kv_store_client = kv_store_client;
    }
    if (app_config != NULL) {
        cfg_mgr->app_config = app_config;
    }
    if (app_interface != NULL) {
        cfg_mgr->app_interface = app_interface;
    }
    if (handle != NULL) {
        cfg_mgr->kv_store_handle = handle;
    }
    if (env_var != NULL) {
        cfg_mgr->env_var = env_var;
    }
    cfg_mgr->dev_mode = result;
    // Assigining this to NULL as its currently not being used
    cfg_mgr->data_store = NULL;

    if (config_char != NULL) {
        free(config_char);
    }
    if (interface_char != NULL) {
        free(interface_char);
    }
    if (interface != NULL) {
        free(interface);
    }
    if (kv_store_config != NULL) {
        config_destroy(kv_store_config);
    }
    if (value != NULL) {
        free(value);
    }

    return cfg_mgr;

err:
    if (c_app_name != NULL) {
        free(c_app_name);
    }
    if (interface != NULL) {
        free(interface);
    }
    if (interface_char != NULL) {
        free(interface_char);
    }
    if (config_char != NULL) {
        free(config_char);
    }
    if (value != NULL) {
        free(value);
    }
    if (env_var != NULL) {
        free(env_var);
    }
    if (kv_store_client != NULL) {
        kv_client_free(kv_store_client);
    }
    if (kv_store_config != NULL) {
        config_destroy(kv_store_config);
    }
    if (cfg_mgr != NULL) {
        free(cfg_mgr);
    }
    return NULL;
}

void cfgmgr_destroy(cfgmgr_ctx_t *cfg_mgr) {
    LOG_DEBUG_0("cfgmgr_ctx_t destroy");
    if (cfg_mgr != NULL) {
        if (cfg_mgr->app_config) {
            config_destroy(cfg_mgr->app_config);
        }
        if (cfg_mgr->app_interface) {
            config_destroy(cfg_mgr->app_interface);
        }
        if (cfg_mgr->data_store) {
            config_destroy(cfg_mgr->data_store);
        }
        if (cfg_mgr->kv_store_handle) {
            free(cfg_mgr->kv_store_handle);
        }
        if (cfg_mgr->app_name) {
            free(cfg_mgr->app_name);
        }
        if (cfg_mgr->env_var) {
            free(cfg_mgr->env_var);
        }
        if (cfg_mgr->kv_store_client) {
            kv_client_free(cfg_mgr->kv_store_client);
        }
        free(cfg_mgr);
    }
    LOG_DEBUG_0("cfgmgr_ctx_t destroy: Done");
}

void cfgmgr_interface_destroy(cfgmgr_interface_t *cfg_mgr_interface) {
    LOG_DEBUG_0("cfgmgr_interface_t destroy");
    if (cfg_mgr_interface != NULL) {
        if (cfg_mgr_interface->cfg_mgr) {
            cfgmgr_destroy(cfg_mgr_interface->cfg_mgr);
        }
        if (cfg_mgr_interface->interface) {
            config_value_destroy(cfg_mgr_interface->interface);
        }
        free(cfg_mgr_interface);
    }
    LOG_DEBUG_0("cfgmgr_interface_t destroy: Done");
}