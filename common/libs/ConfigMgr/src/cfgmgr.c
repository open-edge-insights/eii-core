// Copyright (c) 2021 Intel Corporation.
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
#include "eii/config_manager/cfgmgr.h"

// function to generate kv_store_config from env
config_t* create_kv_store_config() {
    LOG_DEBUG("In %s function", __func__);
    char* c_type_name = NULL;
    char* c_app_name = NULL;
    char* config_manager_type = NULL;
    char* dev_mode_var = NULL;
    char* app_name_var = NULL;
    bool config_set_result = false;
    config_t* config = NULL;
    config_value_t* etcd_type = NULL;
    config_t* etcd_kv_store = NULL;
    config_value_t* cert_file = NULL;
    config_value_t* key_file = NULL;
    config_value_t* ca_file = NULL;
    config_value_t* etcd_kv_store_cvt = NULL;

    // Creating final config object
    config = json_config_new_from_buffer("{}");
    if (config == NULL) {
        LOG_ERROR_0("Error creating config_t config object");
        goto err;
    }

    // Fetching ConfigManager type from env
    config_manager_type = getenv("KVStore");
    if (config_manager_type == NULL) {
        LOG_DEBUG_0("KVStore env not set, defaulting to etcd");
        etcd_type = config_value_new_string("etcd");
        if (etcd_type == NULL) {
            LOG_ERROR_0("Error creating etcd_type config_value_t object");
            goto err;
        }
        config_set_result = config_set(config, "type", etcd_type);
        if (!config_set_result) {
            LOG_ERROR_0("Unable to set config value");
            goto err;
        }

    } else {
        int ind_kv_store_type;
        strcmp_s(config_manager_type, strlen(config_manager_type),
                 "", &ind_kv_store_type);
        if (ind_kv_store_type == 0) {
            LOG_DEBUG_0("KVStore env is set to empty, defaulting to etcd");
            etcd_type = config_value_new_string("etcd");
            if (etcd_type == NULL) {
                LOG_ERROR_0("Error creating etcd_type config_value_t object");
                goto err;
            }
            config_set_result = config_set(config, "type", etcd_type);
            if (!config_set_result) {
                LOG_ERROR_0("Unable to set config value");
                goto err;
            }
        } else {
            size_t str_len = strlen(config_manager_type) + 1;
            c_type_name = (char*)malloc(sizeof(char) * str_len);
            if (c_type_name == NULL) {
                LOG_ERROR_0("Malloc failed for c_type_name");
                goto err;
            }
            int ret = snprintf(c_type_name, str_len, "%s", config_manager_type);
            if (ret < 0) {
                LOG_ERROR_0("snprintf failed for c_type_name");
                goto err;
            }
            LOG_DEBUG("ConfigManager selected is %s", c_type_name);
            etcd_type = config_value_new_string(c_type_name);
            if (etcd_type == NULL) {
                LOG_ERROR_0("Error creating etcd_type config_value_t object");
                goto err;
            }
            config_set_result = config_set(config, "type", etcd_type);
            if (!config_set_result) {
                LOG_ERROR_0("Unable to set config value");
                goto err;
            }
        }
    }

    // Creating etcd_kv_store object
    etcd_kv_store = json_config_new_from_buffer("{}");
    if (etcd_kv_store == NULL) {
        LOG_ERROR_0("Failed to create etcd_kv_store config_value_t object");
        goto err;
    }

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
    if (ret < 0) {
        LOG_ERROR_0("snprintf failed for c_app_name");
        goto err;
    }
    LOG_DEBUG("AppName: %s", c_app_name);

    ret = 0;
    char pub_cert_file[MAX_CONFIG_KEY_LENGTH] = "";
    char pri_key_file[MAX_CONFIG_KEY_LENGTH] = "";
    char trust_file[MAX_CONFIG_KEY_LENGTH] = "";
    if (result == 0) {
        cert_file = config_value_new_string("");
        if (cert_file == NULL) {
            LOG_ERROR_0("Error creating config_value_t object");
            goto err;
        }
        config_set_result = config_set(etcd_kv_store, "cert_file", cert_file);
        if (!config_set_result) {
            LOG_ERROR("Unable to set config value");
            goto err;
        }

        key_file = config_value_new_string("");
        if (key_file == NULL) {
            LOG_ERROR_0("Error creating config_value_t object");
            goto err;
        }
        config_set_result = config_set(etcd_kv_store, "key_file", key_file);
        if (!config_set_result) {
            LOG_ERROR("Unable to set config value");
            goto err;
        }

        ca_file = config_value_new_string("");
        if (ca_file == NULL) {
            LOG_ERROR_0("Error creating config_value_t object");
            goto err;
        }
        config_set_result = config_set(etcd_kv_store, "ca_file", ca_file);
        if (!config_set_result) {
            LOG_ERROR("Unable to set config value");
            goto err;
        }
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

        cert_file = config_value_new_string(pub_cert_file);
        if (cert_file == NULL) {
            LOG_ERROR_0("Error creating config_value_t object");
            goto err;
        }
        config_set_result = config_set(etcd_kv_store, "cert_file", cert_file);
        if (!config_set_result) {
            LOG_ERROR("Unable to set config value");
            goto err;
        }

        key_file = config_value_new_string(pri_key_file);
        if (key_file == NULL) {
            LOG_ERROR_0("Error creating config_value_t object");
            goto err;
        }
        config_set_result = config_set(etcd_kv_store, "key_file", key_file);
        if (!config_set_result) {
            LOG_ERROR("Unable to set config value");
            goto err;
        }

        ca_file = config_value_new_string(trust_file);
        if (ca_file == NULL) {
            LOG_ERROR_0("Error creating config_value_t object");
            goto err;
        }
        config_set_result = config_set(etcd_kv_store, "ca_file", ca_file);
        if (!config_set_result) {
            LOG_ERROR("Unable to set config value");
            goto err;
        }
    }

    etcd_kv_store_cvt = config_value_new_object(etcd_kv_store->cfg, get_config_value, NULL);
    if (etcd_kv_store_cvt == NULL) {
        LOG_ERROR_0("Error creating config_value_t object");
        goto err;
    }
    config_set_result = config_set(config, "etcd_kv_store", etcd_kv_store_cvt);
    if (!config_set_result) {
        LOG_ERROR("Unable to set config value");
        goto err;
    }

    if (c_app_name != NULL) {
        free(c_app_name);
    }
    if (c_type_name != NULL) {
        free(c_type_name);
    }
    if (etcd_type != NULL) {
        config_value_destroy(etcd_type);
    }
    if (etcd_kv_store != NULL) {
        config_value_destroy(etcd_kv_store);
    }
    if (cert_file != NULL) {
        config_value_destroy(cert_file);
    }
    if (key_file != NULL) {
        config_value_destroy(key_file);
    }
    if (ca_file != NULL) {
        config_value_destroy(ca_file);
    }
    if (etcd_kv_store_cvt != NULL) {
        config_value_destroy(etcd_kv_store_cvt);
    }
    return config;
err:
    if (c_app_name != NULL) {
        free(c_app_name);
    }
    if (c_type_name != NULL) {
        free(c_type_name);
    }
    if (config_manager_type != NULL) {
        free(config_manager_type);
    }
    if (dev_mode_var != NULL) {
        free(dev_mode_var);
    }
    if (app_name_var != NULL) {
        free(app_name_var);
    }
    if (config != NULL) {
        config_destroy(config);
    }
    if (etcd_type != NULL) {
        config_value_destroy(etcd_type);
    }
    if (etcd_kv_store != NULL) {
        config_value_destroy(etcd_kv_store);
    }
    if (cert_file != NULL) {
        config_value_destroy(cert_file);
    }
    if (key_file != NULL) {
        config_value_destroy(key_file);
    }
    if (ca_file != NULL) {
        config_value_destroy(ca_file);
    }
    if (etcd_kv_store_cvt != NULL) {
        config_value_destroy(etcd_kv_store_cvt);
    }
    return NULL;
}

cfgmgr_interface_t* cfgmgr_interface_initialize() {
    LOG_DEBUG("In %s function", __func__);
    cfgmgr_interface_t *cfgmgr_ctx = (cfgmgr_interface_t *)malloc(sizeof(cfgmgr_interface_t));
    if (cfgmgr_ctx == NULL) {
        LOG_ERROR_0("Malloc failed for cfgmgr_ctx_t");
        return NULL;
    }
    return cfgmgr_ctx;
}

cfgmgr_interface_t* cfgmgr_get_interface_by_name(cfgmgr_ctx_t* cfgmgr, const char* name, cfgmgr_iface_type_t type) {
    LOG_DEBUG("In %s function", __func__);
    char* interface_type = NULL;
    if (type == CFGMGR_PUBLISHER) {
        interface_type = PUBLISHERS;
    } else if (type == CFGMGR_SUBSCRIBER) {
        interface_type = SUBSCRIBERS;
    } else if (type == CFGMGR_SERVER) {
        interface_type = SERVERS;
    } else if (type == CFGMGR_CLIENT) {
        interface_type = CLIENTS;
    } else {
        LOG_ERROR_0("Interface type not supported");
        return NULL;
    }
    int result = 0;
    cfgmgr_interface_t* ctx = NULL;
    config_value_t* config_name = NULL;
    config_value_t* interface = NULL;

    config_t* app_interface = cfgmgr->app_interface;
    ctx = cfgmgr_interface_initialize();
    if (ctx == NULL) {
        LOG_ERROR_0("cfgmgr initialization failed");
        goto err;
    }

    // Fetching list of Publisher interfaces
    interface = app_interface->get_config_value(app_interface->cfg, interface_type);
    if (interface == NULL) {
        LOG_ERROR_0("interface initialization failed");
        goto err;
    }

    size_t arr_len = config_value_array_len(interface);
    if (arr_len == 0) {
        LOG_ERROR_0("Empty array is not supported, atleast one value should be given.");
        goto err;
    }
    // Iterating through available publisher configs
    for (size_t i = 0; i < arr_len; i++) {
        // Fetch name of individual publisher config
        config_value_t* config = config_value_array_get(interface, i);
        if (config == NULL) {
            LOG_ERROR_0("config initialization failed");
            goto err;
        }
        config_name = config_value_object_get(config, "Name");
        if (config_name == NULL || config_name->body.string == NULL) {
            LOG_ERROR_0("config_name initialization failed");
            goto err;
        }
        // Verifying publisher config with name exists
        strcmp_s(config_name->body.string, strlen(config_name->body.string), name, &result);
        if(result == 0) {
            ctx->cfg_mgr = cfgmgr;
            ctx->interface = config;
            ctx->type = type;
            goto err;
        } else if(i == config_value_array_len(interface)) {
            LOG_ERROR("Publisher by name %s not found", name);
            goto err;
        } else {
            if (config_name != NULL) {
                config_value_destroy(config_name);
            }
            if (config != NULL) {
                config_value_destroy(config);
            }
        }
    }

err:
    if (interface != NULL) {
        config_value_destroy(interface);
    }
    if (config_name != NULL) {
        config_value_destroy(config_name);
    }
    return ctx;
}

cfgmgr_interface_t* cfgmgr_get_interface_by_index(cfgmgr_ctx_t* cfgmgr, int index, cfgmgr_iface_type_t type) {
    LOG_DEBUG("In %s function", __func__);
    char* interface_type = NULL;
    if (type == CFGMGR_PUBLISHER) {
        interface_type = PUBLISHERS;
    } else if (type == CFGMGR_SUBSCRIBER) {
        interface_type = SUBSCRIBERS;
    } else if (type == CFGMGR_SERVER) {
        interface_type = SERVERS;
    } else if (type == CFGMGR_CLIENT) {
        interface_type = CLIENTS;
    } else {
        LOG_ERROR_0("Interface type not supported");
        return NULL;
    }
    config_value_t* config = NULL;
    config_value_t* interface = NULL;
    config_t* app_interface = cfgmgr->app_interface;
    cfgmgr_interface_t* ctx = NULL;
    ctx = cfgmgr_interface_initialize();
    if (ctx == NULL) {
        LOG_ERROR_0("cfgmgr initialization failed");
        goto err;
    }

    // Fetching list of Publisher interfaces
    interface = app_interface->get_config_value(app_interface->cfg, interface_type);
    if (interface == NULL) {
        LOG_ERROR_0("interface initialization failed");
        goto err;
    }

    // Fetch publisher config associated with index
    config = config_value_array_get(interface, index);
    if (config == NULL) {
        LOG_ERROR_0("config initialization failed");
        goto err;
    }

    ctx->cfg_mgr = cfgmgr;
    ctx->interface = config;
    ctx->type = type;

    if (interface != NULL) {
        config_value_destroy(interface);
    }
    return ctx;

err:
    if (interface != NULL) {
        config_value_destroy(interface);
    }
    if (config != NULL) {
        config_value_destroy(config);
    }
    if (ctx != NULL) {
        cfgmgr_interface_destroy(ctx);
    }
    return NULL;
}

cfgmgr_interface_t* cfgmgr_get_publisher_by_name(cfgmgr_ctx_t* cfgmgr, const char* name) {
    LOG_DEBUG("In %s function", __func__);
    cfgmgr_interface_t* publisher_interface = cfgmgr_get_interface_by_name(cfgmgr, name, CFGMGR_PUBLISHER);
    if (publisher_interface == NULL) {
        LOG_ERROR_0("Failed to fetch publisher_interface");
        return NULL;
    }
    return publisher_interface;
}

cfgmgr_interface_t* cfgmgr_get_publisher_by_index(cfgmgr_ctx_t* cfgmgr, int index) {
    LOG_DEBUG("In %s function", __func__);
    cfgmgr_interface_t* publisher_interface = cfgmgr_get_interface_by_index(cfgmgr, index, CFGMGR_PUBLISHER);
    if (publisher_interface == NULL) {
        LOG_ERROR_0("Failed to fetch publisher_interface");
        return NULL;
    }
    return publisher_interface;
}

cfgmgr_interface_t* cfgmgr_get_subscriber_by_name(cfgmgr_ctx_t* cfgmgr, const char* name) {
    LOG_DEBUG("In %s function", __func__);
    cfgmgr_interface_t* subscriber_interface = cfgmgr_get_interface_by_name(cfgmgr, name, CFGMGR_SUBSCRIBER);
    if (subscriber_interface == NULL) {
        LOG_ERROR_0("Failed to fetch subscriber_interface");
        return NULL;
    }
    return subscriber_interface;
}

cfgmgr_interface_t* cfgmgr_get_subscriber_by_index(cfgmgr_ctx_t* cfgmgr, int index) {
    LOG_DEBUG("In %s function", __func__);
    cfgmgr_interface_t* subscriber_interface = cfgmgr_get_interface_by_index(cfgmgr, index, CFGMGR_SUBSCRIBER);
    if (subscriber_interface == NULL) {
        LOG_ERROR_0("Failed to fetch subscriber_interface");
        return NULL;
    }
    return subscriber_interface;
}

cfgmgr_interface_t* cfgmgr_get_server_by_name(cfgmgr_ctx_t* cfgmgr, const char* name) {
    LOG_DEBUG("In %s function", __func__);
    cfgmgr_interface_t* server_interface = cfgmgr_get_interface_by_name(cfgmgr, name, CFGMGR_SERVER);
    if (server_interface == NULL) {
        LOG_ERROR_0("Failed to fetch server_interface");
        return NULL;
    }
    return server_interface;
}

cfgmgr_interface_t* cfgmgr_get_server_by_index(cfgmgr_ctx_t* cfgmgr, int index) {
    LOG_DEBUG("In %s function", __func__);
    cfgmgr_interface_t* server_interface = cfgmgr_get_interface_by_index(cfgmgr, index, CFGMGR_SERVER);
    if (server_interface == NULL) {
        LOG_ERROR_0("Failed to fetch server_interface");
        return NULL;
    }
    return server_interface;
}

cfgmgr_interface_t* cfgmgr_get_client_by_name(cfgmgr_ctx_t* cfgmgr, const char* name) {
    LOG_DEBUG("In %s function", __func__);
    cfgmgr_interface_t* client_interface = cfgmgr_get_interface_by_name(cfgmgr, name, CFGMGR_CLIENT);
    if (client_interface == NULL) {
        LOG_ERROR_0("Failed to fetch client_interface");
        return NULL;
    }
    return client_interface;
}

cfgmgr_interface_t* cfgmgr_get_client_by_index(cfgmgr_ctx_t* cfgmgr, int index) {
    LOG_DEBUG("In %s function", __func__);
    cfgmgr_interface_t* client_interface = cfgmgr_get_interface_by_index(cfgmgr, index, CFGMGR_CLIENT);
    if (client_interface == NULL) {
        LOG_ERROR_0("Failed to fetch client_interface");
        return NULL;
    }
    return client_interface;
}

config_value_t* cfgmgr_get_endpoint(cfgmgr_interface_t* ctx) {
    LOG_DEBUG("In %s function", __func__);
    config_value_t* config = ctx->interface;
    if (config == NULL) {
        LOG_ERROR_0("config initialization failed");
        return NULL;
    }
    // Fetching EndPoint from config
    config_value_t* end_point = config_value_object_get(config, "EndPoint");
    if (end_point == NULL) {
        LOG_ERROR_0("end_point initialization failed");
        return NULL;
    }
    return end_point;
}

config_value_t* cfgmgr_get_topics(cfgmgr_interface_t* ctx) {
    LOG_DEBUG("In %s function", __func__);
    if (ctx->type == CFGMGR_SERVER || ctx->type == CFGMGR_CLIENT) {
        LOG_ERROR_0("cfgmgr_get_topics not applicable for CFGMGR_SERVER/CFGMGR_CLIENT");
        return NULL;
    }
    // Setting default to 1 since strcmp_s
    // changes it to 0
    int ret = 1;
    config_t* topics_cvt = NULL;
    // Fetching interface
    config_value_t* config = ctx->interface;
    if (config == NULL) {
        LOG_ERROR_0("config initialization failed");
        return NULL;
    }
    // Fetching topics from interface
    config_value_t* topics = config_value_object_get(ctx->interface, "Topics");
    if (topics == NULL) {
        LOG_ERROR_0("topics initialization failed");
        return NULL;
    }
    if (topics->type != CVT_ARRAY) {
        LOG_ERROR_0("Topics type mismatch, it should be array");
        return NULL;
    }

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
    if ((arrlen == 1) && (ret == 0 )) {
        // Creating empty config array for * single topic
        config_t* topics_cvt = json_config_new_from_buffer("[\"\"]");
        if (topics_cvt == NULL) {
            LOG_ERROR_0("Failed to create topics_cvt config_value_t object");
            return NULL;
        }
        topics = config_value_new_array(
                (void*) topics_cvt->cfg , arrlen, get_array_item, NULL);
        if (topics == NULL) {
            LOG_ERROR_0("config value new array for topic failed");
            return NULL;
        }
        return topics;
    }
    if (topic_value != NULL) {
        config_value_destroy(topic_value);
    }
    return topics;
}

bool cfgmgr_set_topics(cfgmgr_interface_t* ctx, char const* const* topics_list, int len) {
    LOG_DEBUG("In %s function", __func__);
    if (ctx->type == CFGMGR_SERVER || ctx->type == CFGMGR_CLIENT) {
        LOG_ERROR_0("cfgmgr_set_topics not applicable for CFGMGR_SERVER/CFGMGR_CLIENT");
        return NULL;
    }
    char* type = NULL;
    if (ctx->type == CFGMGR_PUBLISHER) {
        type = PUBLISHERS;
    } else if (ctx->type == CFGMGR_SUBSCRIBER) {
        type = SUBSCRIBERS;
    }
    config_t* config_arr = NULL;
    config_value_t* config_arr_cvt = NULL;
    config_t* config_input = NULL;

    bool ret_val = true;
    // Creating config array from char**
    config_arr = json_config_new_array(topics_list, len);
    if (config_arr == NULL) {
        LOG_ERROR_0("Failed to create config_arr config_t object");
        goto err;
    }
    // Creating config_value_t object from config array
    config_arr_cvt = config_value_new_array(
                (void*) config_arr->cfg , len, get_array_item, NULL);
    if (config_arr_cvt == NULL) {
        LOG_ERROR_0("Failed to create config_arr_cvt config_value_t object");
        goto err;
    }
    // Creating config_t object to set new topics value
    config_input = config_new(
            (void*) ctx->interface->body.object->object, free_json, get_config_value, set_config_value);
    if (config_input == NULL) {
        LOG_ERROR_0("Failed to create config_input config_t object");
        goto err;
    }
    // Setting new topics
    ret_val = config_set(config_input, TOPICS, config_arr_cvt);
    if (!ret_val) {
        LOG_ERROR_0("Unable to set config value");
        goto err;
    }
    if (config_arr_cvt != NULL) {
        config_value_destroy(config_arr_cvt);
    }
    // Freeing this instead of destroying since we do
    // not want to free the inner config_input->cfg
    if (config_input != NULL) {
        free(config_input);
    }
    // Freeing this instead of destroying since we do
    // not want to free the inner config_arr->cfg
    if (config_arr != NULL) {
        free(config_arr);
    }

    return ret_val;
err:
    if (config_arr != NULL) {
        config_destroy(config_arr);
    }
    if (config_input != NULL) {
        config_destroy(config_input);
    }
    if (config_arr_cvt != NULL) {
        config_value_destroy(config_arr_cvt);
    }
    return ret_val;
}

config_value_t* cfgmgr_get_allowed_clients(cfgmgr_interface_t* ctx) {
    LOG_DEBUG("In %s function", __func__);
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
    LOG_DEBUG("In %s function", __func__);
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
    config_value_t* cvt_type = NULL;
    config_value_t* zmq_recv_hwm_value = NULL;
    config_value_t* brokered_value = NULL;
    config_t* zmq_tcp_publish_cvt = NULL;
    config_value_t* zmq_tcp_host = NULL;
    config_value_t* zmq_tcp_port = NULL;
    config_value_t* zmq_tcp_publish = NULL;
    char* config_value_cr = NULL;
    bool config_set_result = false;
    // Creating final config object
    m_config = json_config_new_from_buffer("{}");
    if (m_config == NULL) {
        LOG_ERROR_0("Error creating m_config object");
        goto err;
    }

    // Fetching Type from config
    publish_config_type = config_value_object_get(pub_config, TYPE);
    if (publish_config_type == NULL) {
        LOG_ERROR_0("publish_config_type initialization failed");
        goto err;
    }

    if (publish_config_type->type != CVT_STRING || publish_config_type->body.string == NULL) {
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

    char* end_point = NULL;
    if (publish_config_endpoint->type == CVT_OBJECT) {
        end_point = cvt_to_char(publish_config_endpoint);
    } else if(publish_config_endpoint->type == CVT_STRING && publish_config_endpoint->body.string != NULL) {
        end_point = publish_config_endpoint->body.string;
    }

    if (end_point == NULL) {
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
            end_point = ep_override;
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
            end_point = publisher_ep;
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

    // Creating type config_value_t to set in final config
    cvt_type = config_value_new_string(type);
    if (cvt_type == NULL) {
        LOG_ERROR_0("Error creating config_value_t object");
        goto err;
    }
    config_set_result = config_set(m_config, "type", cvt_type);
    if (!config_set_result) {
        LOG_ERROR("Unable to set config value");
        goto err;
    }

    // Adding zmq_recv_hwm value if available
    zmq_recv_hwm_value = config_value_object_get(pub_config, ZMQ_RECV_HWM);
    if (zmq_recv_hwm_value != NULL) {
        if (zmq_recv_hwm_value->type != CVT_INTEGER) {
            LOG_ERROR_0("zmq_recv_hwm type is not integer");
            goto err;
        }
        config_set_result = config_set(m_config, ZMQ_RECV_HWM, zmq_recv_hwm_value);
        if (!config_set_result) {
            LOG_ERROR("Unable to set config value");
            goto err;
        }
    }

    // Adding brokered value if available
    brokered_value = config_value_object_get(pub_config, BROKERED);
    if (brokered_value != NULL) {
        if (brokered_value->type != CVT_BOOLEAN) {
            LOG_ERROR_0("brokered_value type is not boolean");
            goto err;
        }
    }

    if (!strcmp(type, "zmq_ipc")) {
        bool ret = get_ipc_config(m_config, pub_config, end_point, CFGMGR_PUBLISHER);
        if (ret == false){
            LOG_ERROR_0("IPC configuration for publisher failed");
            goto err;
        }
    } else if (!strcmp(type, "zmq_tcp")) {
        // Add host & port to zmq_tcp_publish object
        host_port = get_host_port(end_point);
        if (host_port == NULL) {
            LOG_ERROR_0("Get host and port failed");
            goto err;
        }
        host = host_port[0];
        trim(host);
        port = host_port[1];
        trim(port);
        __int64_t i_port = atoi(port);
        // Creating empty config object
        zmq_tcp_publish_cvt = json_config_new_from_buffer("{}");
        if (zmq_tcp_publish_cvt == NULL) {
            LOG_ERROR_0("Get zmq_tcp_publish_cvt failed");
            goto err;
        }
        zmq_tcp_host = config_value_new_string(host);
        if (zmq_tcp_host == NULL) {
            LOG_ERROR_0("Get zmq_tcp_host failed");
            goto err;
        }
        zmq_tcp_port = config_value_new_integer(i_port);
        if (zmq_tcp_port == NULL) {
            LOG_ERROR_0("Get zmq_tcp_port failed");
            goto err;
        }
        config_set_result = config_set(zmq_tcp_publish_cvt, "host", zmq_tcp_host);
        if (!config_set_result) {
            LOG_ERROR("Unable to set config value");
            goto err;
        }
        config_set_result = config_set(zmq_tcp_publish_cvt, "port", zmq_tcp_port);
        if (!config_set_result) {
            LOG_ERROR("Unable to set config value");
            goto err;
        }
        if (brokered_value != NULL) {
            config_set_result = config_set(zmq_tcp_publish_cvt, BROKERED, brokered_value);
            if (!config_set_result) {
                LOG_ERROR("Unable to set config value");
                goto err;
            }
        }
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
                ret_val = add_keys_to_config(zmq_tcp_publish_cvt, app_name, kv_store_client, kv_store_handle, broker_app_name, pub_config);
                if(!ret_val) {
                    LOG_ERROR_0("Failed to add respective cert keys");
                    goto err;
                }
            } else{
                ret_val = construct_tcp_publisher_prod(app_name, m_config, zmq_tcp_publish_cvt, kv_store_handle, pub_config, kv_store_client);
                if(!ret_val) {
                    LOG_ERROR_0("Failed to construct tcp config struct");
                    goto err;
                }
            }
        }
        zmq_tcp_publish = config_value_new_object(zmq_tcp_publish_cvt->cfg, get_config_value, NULL);
        config_set_result = config_set(m_config, "zmq_tcp_publish", zmq_tcp_publish);
        if (!config_set_result) {
            LOG_ERROR("Unable to set config value");
            goto err;
        }
    } else {
        LOG_ERROR_0("Type should be either \"zmq_ipc\" or \"zmq_tcp\"");
        goto err;
    }
    // Constructing char* object from config_t object
    config_value_cr = configt_to_char(m_config);
    if (config_value_cr == NULL) {
        LOG_ERROR_0("config_value_cr initialization failed");
        goto err;
    }
    LOG_DEBUG("Env publisher Config is : %s \n", config_value_cr);

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
    if (cvt_type != NULL) {
        config_value_destroy(cvt_type);
    }
    if (host_port != NULL) {
        free_mem(host_port);
    }
    // Freeing this instead of destroying since we don't
    // want to free the inner zmq_tcp_publish_cvt->cfg
    if (zmq_tcp_publish_cvt != NULL) {
        free(zmq_tcp_publish_cvt);
    }
    if (zmq_recv_hwm_value != NULL) {
        config_value_destroy(zmq_recv_hwm_value);
    }
    if (brokered_value != NULL) {
        config_value_destroy(brokered_value);
    }
    if (zmq_tcp_host != NULL) {
        config_value_destroy(zmq_tcp_host);
    }
    if (zmq_tcp_port != NULL) {
        config_value_destroy(zmq_tcp_port);
    }
    return m_config;
}

config_t* cfgmgr_get_msgbus_config_sub(cfgmgr_interface_t* ctx) {
    LOG_DEBUG("In %s function", __func__);
    char** host_port = NULL;
    char* host = NULL;
    char* port = NULL;
    config_value_t* sub_config = ctx->interface;
    char* app_name = ctx->cfg_mgr->app_name;
    bool dev_mode = false;
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
    config_t* topics = NULL;
    kv_store_client_t* kv_store_client = NULL;
    config_t* c_json = NULL;
    config_value_t* type_cvt = NULL;
    config_value_t* zmq_tcp_host = NULL;
    config_value_t* zmq_tcp_port = NULL;

    int devmode = ctx->cfg_mgr->dev_mode;
    if (devmode == 0) {
        dev_mode = true;
    }

    kv_store_client = ctx->cfg_mgr->kv_store_client;
    void* kv_store_handle = ctx->cfg_mgr->kv_store_handle;

    // Creating final config object
    c_json = json_config_new_from_buffer("{}");
    if (c_json == NULL) {
        LOG_ERROR_0("Error creating c_json object");
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

    char* end_point = NULL;
    if (subscribe_config_endpoint->type == CVT_OBJECT) {
        end_point = cvt_to_char(subscribe_config_endpoint);
    } else {
        end_point = subscribe_config_endpoint->body.string;
    }

    if (end_point == NULL) {
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
            end_point = ep_override;
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
            end_point = subscriber_ep;
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

    type_cvt = config_value_new_string(type);
    if (type_cvt == NULL) {
        LOG_ERROR_0("Get type_cvt failed");
        goto err;
    }
    bool config_set_result = config_set(c_json, "type", type_cvt);
    if (!config_set_result) {
        LOG_ERROR_0("Unable to set config value");
        goto err;
    }

    // Adding zmq_recv_hwm value if available
    zmq_recv_hwm_value = config_value_object_get(sub_config, ZMQ_RECV_HWM);
    if (zmq_recv_hwm_value != NULL) {
        if (zmq_recv_hwm_value->type != CVT_INTEGER) {
            LOG_ERROR_0("zmq_recv_hwm type is not integer");
            goto err;
        }
        bool config_set_result = config_set(c_json, ZMQ_RECV_HWM, zmq_recv_hwm_value);
        if (!config_set_result) {
            LOG_ERROR("Unable to set config value");
        }
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

        for (size_t i = 0; i < arr_len; i++) {
            // Creating empty config object
            topics = json_config_new_from_buffer("{}");
            zmq_tcp_host = config_value_new_string(host);
            if (zmq_tcp_host == NULL) {
                LOG_ERROR_0("Get zmq_tcp_host failed");
                goto err;
            }
            zmq_tcp_port = config_value_new_integer(i_port);
            if (zmq_tcp_port == NULL) {
                LOG_ERROR_0("Get zmq_tcp_port failed");
                goto err;
            }
            config_set_result = config_set(topics, "host", zmq_tcp_host);
            if (!config_set_result) {
                LOG_ERROR("Unable to set config value");
            }
            config_set_result = config_set(topics, "port", zmq_tcp_port);
            if (!config_set_result) {
                LOG_ERROR("Unable to set config value");
            }

            topic = config_value_array_get(topic_array, i);
            if (topic == NULL) {
                LOG_ERROR_0("topic initialization failed");
                goto err;
            }

            // if topics lenght is not 1 or the topic is not equal to "*",
            //then we are adding that topic for subscription.
            if (!dev_mode) {
                bool ret_val;
                // This is ZmqBroker usecase, where in "PublisherAppname" will be specified as "*"
                // hence comparing for "PublisherAppname" and "*"
                strcmp_s(publisher_appname->body.string, strlen(publisher_appname->body.string), "*", &ret);
                if(ret == 0) {
                    // In case of ZmqBroker, it is "X-SUB" which needs "publishers" way of
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
            config_value_t* topics_cvt = config_value_new_object(topics->cfg, get_config_value, NULL);
            if (topics_cvt == NULL) {
                LOG_ERROR_0("Unable to create topics_cvt config_value_t object");
                goto err;
            }
            if((arr_len == 1) && (topicret == 0)) {
                config_set_result = config_set(c_json, "", topics_cvt);
                if (!config_set_result) {
                    LOG_ERROR_0("Unable to set config value");
                    goto err;
                }
            } else {
                config_set_result = config_set(c_json, topic->body.string, topics_cvt);
                if (!config_set_result) {
                    LOG_ERROR_0("Unable to set config value");
                    goto err;
                }
            }
            if (topic != NULL) {
                config_value_destroy(topic);
            }
        }
    } else {
        LOG_ERROR_0("Type should be either \"zmq_ipc\" or \"zmq_tcp\"");
        goto err;
    }

    // Constructing char* object from config_t object
    config_value_cr = configt_to_char(c_json);
    if (config_value_cr == NULL) {
        LOG_ERROR_0("config_value_cr initialization failed");
        goto err;
    }
    LOG_DEBUG("Env subscriber Config is : %s \n", config_value_cr);

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
    if (type_cvt != NULL) {
        config_value_destroy(type_cvt);
    }
    if (zmq_tcp_host != NULL) {
        config_value_destroy(zmq_tcp_host);
    }
    if (zmq_tcp_port != NULL) {
        config_value_destroy(zmq_tcp_port);
    }
return c_json;
}

config_t* cfgmgr_get_msgbus_config_server(cfgmgr_interface_t* ctx) {
    LOG_DEBUG("In %s function", __func__);
    config_value_t* serv_config = ctx->interface;
    char* app_name = ctx->cfg_mgr->app_name;
    int dev_mode = ctx->cfg_mgr->dev_mode;
    kv_store_client_t* kv_store_client = ctx->cfg_mgr->kv_store_client;
    void* kv_store_handle = ctx->cfg_mgr->kv_store_handle;
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
    char **all_clients = NULL;
    config_value_t* type_cvt = NULL;
    config_value_t* zmq_recv_hwm_value = NULL;
    config_t* server_topic = NULL;
    config_value_t* zmq_tcp_host = NULL;
    config_value_t* zmq_tcp_port = NULL;
    config_t* all_clients_arr_config = NULL;
    config_value_t* all_clients_cvt = NULL;
    config_value_t* server_secret_key_cvt = NULL;
    config_value_t* server_topic_cvt = NULL;

    config_t* c_json = NULL;
    // Creating final config object
    c_json = json_config_new_from_buffer("{}");
    if (c_json == NULL) {
        LOG_ERROR_0("Error creating c_json object");
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
    char* end_point = server_endpoint->body.string;

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
            end_point = ep_override;
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
            end_point = server_ep;
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

    type_cvt = config_value_new_string(type);
    if (type_cvt == NULL) {
        LOG_ERROR_0("Get type_cvt failed");
        goto err;
    }
    bool config_set_result = config_set(c_json, "type", type_cvt);
    if (!config_set_result) {
        LOG_ERROR("Unable to set config value");
        goto err;
    }

    // Adding zmq_recv_hwm value if available
    zmq_recv_hwm_value = config_value_object_get(serv_config, ZMQ_RECV_HWM);
    if (zmq_recv_hwm_value != NULL) {
        if (zmq_recv_hwm_value->type != CVT_INTEGER) {
            LOG_ERROR_0("zmq_recv_hwm type is not integer");
            goto err;
        }

        bool config_set_result = config_set(c_json, ZMQ_RECV_HWM, zmq_recv_hwm_value);
        if (!config_set_result) {
            LOG_ERROR("Unable to set config value");
            goto err;
        }
    }

    if (!strcmp(type, "zmq_ipc")) {
        bool ret = get_ipc_config(c_json, serv_config, end_point, CFGMGR_SERVER);
        if (ret == false) {
            LOG_ERROR_0("IPC configuration for server failed");
            goto err;
        }
    } else if (!strcmp(type, "zmq_tcp")) {
        // Add host & port to zmq_tcp_publish object
        server_topic = json_config_new_from_buffer("{}");
        if (server_topic == NULL) {
            LOG_ERROR_0("server_topic initialization failed");
            goto err;
        }

        host_port = get_host_port(end_point);
        if (host_port == NULL) {
            LOG_ERROR_0("Get host and port failed");
            goto err;
        }
        char* host = host_port[0];
        trim(host);
        char* port = host_port[1];
        trim(port);
        __int64_t i_port = atoi(port);

        zmq_tcp_host = config_value_new_string(host);
        if (zmq_tcp_host == NULL) {
            LOG_ERROR_0("Get zmq_tcp_host failed");
            goto err;
        }
        zmq_tcp_port = config_value_new_integer(i_port);
        if (zmq_tcp_port == NULL) {
            LOG_ERROR_0("Get zmq_tcp_port failed");
            goto err;
        }

        config_set_result = config_set(server_topic, "host", zmq_tcp_host);
        if (!config_set_result) {
            LOG_ERROR("Unable to set config value");
            goto err;
        }
        config_set_result = config_set(server_topic, "port", zmq_tcp_port);
        if (!config_set_result) {
            LOG_ERROR("Unable to set config value");
            goto err;
        }

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
                pub_key_values = kv_store_client->get_prefix(kv_store_handle, "/Publickeys/");
                if (pub_key_values == NULL) {
                    LOG_ERROR_0("pub_key_values initialization failed");
                    goto err;
                }

                config_set_result = config_set(c_json, "allowed_clients", pub_key_values);
                if (!config_set_result) {
                    LOG_ERROR_0("Unable to set config value");
                    goto err;
                }
            } else {
                config_value_t* array_value;
                size_t arr_len = config_value_array_len(server_json_clients);
                if(arr_len == 0){
                    LOG_ERROR_0("Empty array is not supported, atleast one value should be given.");
                    goto err;
                }
                all_clients = (char**)calloc(arr_len, sizeof(char*));
                if (all_clients == NULL) {
                    LOG_ERROR_0("all_clients initialization failed");
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
                        all_clients[i] = NULL;
                    } else {
                        all_clients[i] = client_public_key;
                    }
                    free(grab_public_key);

                    config_value_destroy(array_value);
                }
                // Adding all public keys of clients to allowed_clients of config
                all_clients_arr_config = json_config_new_array(all_clients, arr_len);
                if (all_clients_arr_config == NULL) {
                    LOG_ERROR_0("Failed to create all_clients_arr_config config_t object");
                    goto err;
                }

                all_clients_cvt = config_value_new_object(all_clients_arr_config->cfg, get_config_value, NULL);
                if (all_clients_cvt == NULL) {
                    LOG_ERROR_0("Unable to create config_value_t all_clients_cvt object");
                    goto err;
                }

                config_set_result = config_set(c_json, "allowed_clients", all_clients_cvt);
                if (!config_set_result) {
                    LOG_ERROR_0("Unable to set config value");
                    goto err;
                }
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

            server_secret_key_cvt = config_value_new_string(server_secret_key);
            if (server_secret_key_cvt == NULL) {
                LOG_ERROR_0("Get server_secret_key_cvt failed");
                goto err;
            }
            config_set_result = config_set(server_topic, "server_secret_key", server_secret_key_cvt);
            if (!config_set_result) {
                LOG_ERROR("Unable to set config value");
                goto err;
            }

        }
        // Creating the server_topic config_value_t object
        server_topic_cvt = config_value_new_object(server_topic->cfg, get_config_value, set_config_value);
        if (server_topic_cvt == NULL) {
            LOG_ERROR_0("server_topic_cvt config_value_t initialization failed");
            goto err;
        }
        config_set_result = config_set(c_json, server_name->body.string, server_topic_cvt);
        if (!config_set_result) {
            LOG_ERROR_0("Unable to set config value");
            goto err;
        }
    } else {
        LOG_ERROR_0("Type should be either \"zmq_ipc\" or \"zmq_tcp\"");
        goto err;
    }

    // Constructing char* object from object
    config_value_cr = configt_to_char(c_json);
    if (config_value_cr == NULL) {
        LOG_ERROR_0("config_value_cr initialization failed");
        goto err;
    }
    LOG_DEBUG("Env server Config is : %s \n", config_value_cr);

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
    if (type_cvt != NULL) {
        config_value_destroy(type_cvt);
    }
    if (zmq_recv_hwm_value != NULL) {
        config_value_destroy(zmq_recv_hwm_value);
    }
    if (server_topic != NULL) {
        free(server_topic);
    }
    if (zmq_tcp_host != NULL) {
        config_value_destroy(zmq_tcp_host);
    }
    if (zmq_tcp_port != NULL) {
        config_value_destroy(zmq_tcp_port);
    }
    if (all_clients_arr_config != NULL) {
        free(all_clients_arr_config);
    }
    if (all_clients_cvt != NULL) {
        free(all_clients_cvt);
    }
    if (all_clients != NULL) {
        free_mem(all_clients);
    }
    if (server_secret_key_cvt != NULL) {
        config_value_destroy(server_secret_key_cvt);
    }
    if (server_topic_cvt != NULL) {
        free(server_topic_cvt);
    }
    return c_json;
}

config_t* cfgmgr_get_msgbus_config_client(cfgmgr_interface_t* ctx) {
    LOG_DEBUG("In %s function", __func__);
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
    config_t* c_json = NULL;
    config_value_t* client_name = NULL;
    config_value_t* type_cvt = NULL;
    config_t* client_topic = NULL;
    config_value_t* zmq_tcp_host = NULL;
    config_value_t* zmq_tcp_port = NULL;
    config_value_t* server_public_key_cvt = NULL;
    config_value_t* client_public_key_cvt = NULL;
    config_value_t* sub_pri_key_cvt = NULL;
    config_value_t* client_topic_cvt = NULL;

    // Creating the final config object
    c_json = json_config_new_from_buffer("{}");
    if (c_json == NULL) {
        LOG_ERROR_0("Error creating c_json object");
        goto err;
    }

    // Fetching name from config
    client_name = config_value_object_get(cli_config, NAME);
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
    char* end_point = client_endpoint->body.string;

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
            end_point = ep_override;
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
            end_point = client_ep;
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

    type_cvt = config_value_new_string(type);
    if (type_cvt == NULL) {
        LOG_ERROR_0("Get type_cvt failed");
        goto err;
    }
    bool config_set_result = config_set(c_json, "type", type_cvt);
    if (!config_set_result) {
        LOG_ERROR_0("Unable to set config value");
        goto err;
    }

    // Adding zmq_recv_hwm value if available
    zmq_recv_hwm_value = config_value_object_get(cli_config, ZMQ_RECV_HWM);
    if (zmq_recv_hwm_value != NULL) {
        if (zmq_recv_hwm_value->type != CVT_INTEGER) {
            LOG_ERROR_0("zmq_recv_hwm type is not integer");
            goto err;
        }

        config_set_result = config_set(c_json, ZMQ_RECV_HWM, zmq_recv_hwm_value);
        if (!config_set_result) {
            LOG_ERROR_0("Unable to set config value");
            goto err;
        }
    }

    if (!strcmp(type, "zmq_ipc")) {
        bool ret = get_ipc_config(c_json, cli_config, end_point, CFGMGR_CLIENT);
        if (ret == false) {
            LOG_ERROR_0("IPC configuration for client failed");
            return NULL;
        }
    } else if (!strcmp(type, "zmq_tcp")) {

        // Add host & port to client_topic object
        client_topic = json_config_new_from_buffer("{}");
        if (client_topic == NULL) {
            LOG_ERROR_0("client_topic object initialization failed");
            goto err;
        }
        host_port = get_host_port(end_point);
        if (host_port == NULL) {
            LOG_ERROR_0("Get host and port failed");
            goto err;
        }
        host = host_port[0];
        trim(host);
        port = host_port[1];
        trim(port);
        __int64_t i_port = atoi(port);
        zmq_tcp_host = config_value_new_string(host);
        if (zmq_tcp_host == NULL) {
            LOG_ERROR_0("Get zmq_tcp_host failed");
            goto err;
        }
        zmq_tcp_port = config_value_new_integer(i_port);
        if (zmq_tcp_port == NULL) {
            LOG_ERROR_0("Get zmq_tcp_port failed");
            goto err;
        }
        config_set_result = config_set(client_topic, "host", zmq_tcp_host);
        if (!config_set_result) {
            LOG_ERROR_0("Unable to set config value");
            goto err;
        }
        config_set_result = config_set(client_topic, "port", zmq_tcp_port);
        if (!config_set_result) {
            LOG_ERROR_0("Unable to set config value");
            goto err;
        }

        if (dev_mode != 0) {

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
            server_public_key_cvt = config_value_new_string(server_public_key);
            if (server_public_key_cvt == NULL) {
                LOG_ERROR_0("Get server_secret_key_cvt failed");
                goto err;
            }
            config_set_result = config_set(client_topic, "server_public_key", server_public_key_cvt);
            if (!config_set_result) {
                LOG_ERROR("Unable to set config value");
            }

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

            client_public_key_cvt = config_value_new_string(sub_public_key);
            if (client_public_key_cvt == NULL) {
                LOG_ERROR_0("Get client_public_key_cvt failed");
                goto err;
            }
            config_set_result = config_set(client_topic, "client_public_key", client_public_key_cvt);
            if (!config_set_result) {
                LOG_ERROR_0("Unable to set config value");
                goto err;
            }

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

            sub_pri_key_cvt = config_value_new_string(sub_pri_key);
            if (sub_pri_key_cvt == NULL) {
                LOG_ERROR_0("Get sub_pri_key_cvt failed");
                goto err;
            }
            config_set_result = config_set(client_topic, "client_secret_key", sub_pri_key_cvt);
            if (!config_set_result) {
                LOG_ERROR_0("Unable to set config value");
                goto err;
            }
        }
        client_topic_cvt = config_value_new_object(client_topic->cfg, get_config_value, NULL);
        if (client_topic_cvt == NULL) {
            LOG_ERROR_0("Unable to create config_value_t client_topic_cvt object");
            goto err;
        }
        // Creating the final config object
        config_set_result = config_set(c_json, client_name->body.string, client_topic_cvt);
        if (!config_set_result) {
            LOG_ERROR_0("Unable to set config value");
            goto err;
        }
    } else {
        LOG_ERROR_0("Type should be either \"zmq_ipc\" or \"zmq_tcp\"");
        goto err;
    }

    config_value = configt_to_char(c_json);
    if (config_value == NULL) {
        LOG_ERROR_0("config_value object initialization failed");
        goto err;
    }
    LOG_DEBUG("Env client Config is : %s \n", config_value);

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
    if (type_cvt != NULL) {
        config_value_destroy(type_cvt);
    }
    if (client_topic != NULL) {
        free(client_topic);
    }
    if (zmq_tcp_host != NULL) {
        config_value_destroy(zmq_tcp_host);
    }
    if (zmq_tcp_port != NULL) {
        config_value_destroy(zmq_tcp_port);
    }
    if (server_public_key_cvt != NULL) {
        config_value_destroy(server_public_key_cvt);
    }
    if (client_public_key_cvt != NULL) {
        config_value_destroy(client_public_key_cvt);
    }
    if (sub_pri_key_cvt != NULL) {
        config_value_destroy(sub_pri_key_cvt);
    }
    if (client_topic_cvt != NULL) {
        config_value_destroy(client_topic_cvt);
    }
    return c_json;
}

config_t* cfgmgr_get_msgbus_config(cfgmgr_interface_t* ctx) {
    LOG_DEBUG("In %s function", __func__);
    config_t* config;
    if (ctx->type == CFGMGR_PUBLISHER) {
        config = cfgmgr_get_msgbus_config_pub(ctx);
    } else if (ctx->type == CFGMGR_SUBSCRIBER) {
        config = cfgmgr_get_msgbus_config_sub(ctx);
    } else if (ctx->type == CFGMGR_SERVER) {
        config = cfgmgr_get_msgbus_config_server(ctx);
    } else if (ctx->type == CFGMGR_CLIENT) {
        config = cfgmgr_get_msgbus_config_client(ctx);
    } else {
        LOG_ERROR_0("Interface type not supported");
        return NULL;
    }
    return config;
}

bool cfgmgr_is_dev_mode(cfgmgr_ctx_t* cfgmgr) {
    LOG_DEBUG("In %s function", __func__);
    // Fetching dev mode from cfgmgr
    int result = cfgmgr->dev_mode;
    if (result == 0) {
        return true;
    }
    return false;
}

config_value_t* cfgmgr_get_appname(cfgmgr_ctx_t* cfgmgr) {
    LOG_DEBUG("In %s function", __func__);
    // Fetching app name from cfgmgr
    config_value_t* app_name = config_value_new_string(cfgmgr->app_name);
    if (app_name == NULL) {
        LOG_ERROR_0("Fetching appname failed");
        return NULL;
    }
    return app_name;
}

int cfgmgr_get_num_elements(config_t* config, const char* key) {
    LOG_DEBUG("In %s function", __func__);
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
    LOG_DEBUG("In %s function", __func__);
    int result = cfgmgr_get_num_elements(cfgmgr->app_interface, PUBLISHERS);
    return result;
}

int cfgmgr_get_num_subscribers(cfgmgr_ctx_t* cfgmgr) {
    LOG_DEBUG("In %s function", __func__);
    int result = cfgmgr_get_num_elements(cfgmgr->app_interface, SUBSCRIBERS);
    return result;
}

int cfgmgr_get_num_servers(cfgmgr_ctx_t* cfgmgr) {
    LOG_DEBUG("In %s function", __func__);
    int result = cfgmgr_get_num_elements(cfgmgr->app_interface, SERVERS);
    return result;
}

int cfgmgr_get_num_clients(cfgmgr_ctx_t* cfgmgr) {
    LOG_DEBUG("In %s function", __func__);
    int result = cfgmgr_get_num_elements(cfgmgr->app_interface, CLIENTS);
    return result;
}

config_t* cfgmgr_get_app_config(cfgmgr_ctx_t* cfgmgr) {
    LOG_DEBUG("In %s function", __func__);
    return cfgmgr->app_config;
}

config_t* cfgmgr_get_app_interface(cfgmgr_ctx_t* cfgmgr) {
    LOG_DEBUG("In %s function", __func__);
    return cfgmgr->app_interface;
}

config_value_t* cfgmgr_get_app_config_value(cfgmgr_ctx_t* cfgmgr, const char* key) {
    LOG_DEBUG("In %s function", __func__);
    return cfgmgr->app_config->get_config_value(cfgmgr->app_config->cfg, key);
}

config_value_t* cfgmgr_get_app_interface_value(cfgmgr_ctx_t* cfgmgr, const char* key) {
    LOG_DEBUG("In %s function", __func__);
    return cfgmgr->app_interface->get_config_value(cfgmgr->app_interface->cfg, key);
}

config_value_t* cfgmgr_get_interface_value(cfgmgr_interface_t* cfgmgr_interface, const char* key) {
    LOG_DEBUG("In %s function", __func__);
    config_value_t* interface_value = config_value_object_get(cfgmgr_interface->interface, key);
    return interface_value;
}

void cfgmgr_watch(cfgmgr_ctx_t* cfgmgr, const char* key, cfgmgr_watch_callback_t watch_callback, void* user_data) {
    LOG_DEBUG("In %s function", __func__);
    // Calling the base watch API
    cfgmgr->kv_store_client->watch(cfgmgr->kv_store_handle, key, watch_callback, user_data);
    return;
}

void cfgmgr_watch_prefix(cfgmgr_ctx_t* cfgmgr, char* prefix, cfgmgr_watch_callback_t watch_callback, void* user_data) {
    LOG_DEBUG("In %s function", __func__);
    // Calling the base watch_prefix API
    cfgmgr->kv_store_client->watch_prefix(cfgmgr->kv_store_handle, prefix, watch_callback, user_data);
    return;
}

cfgmgr_ctx_t* cfgmgr_initialize() {
    LOG_DEBUG("In %s function", __func__);
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
        // TODO: Find a way to parse a char* to iterate and fetch the
        // key-value pairs using just config_t, not depending on cJSON
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
        cJSON_Delete(env_json);
    }

    // Setting log level
    char* str_log_level = NULL;
    log_lvl_t log_level = LOG_LVL_ERROR; // default log level is `ERROR`

    str_log_level = getenv("C_LOG_LEVEL");
    if(str_log_level == NULL) {
        LOG_ERROR_0("C_LOG_LEVEL env not set");
    } else {
        if(strncmp(str_log_level, "DEBUG", 5) == 0) {
            log_level = LOG_LVL_DEBUG;
        } else if(strncmp(str_log_level, "INFO", 5) == 0) {
            log_level = LOG_LVL_INFO;
        } else if(strncmp(str_log_level, "WARN", 5) == 0) {
            log_level = LOG_LVL_WARN;
        } else if(strncmp(str_log_level, "ERROR", 5) == 0) {
            log_level = LOG_LVL_ERROR;
    }
    set_log_level(log_level);
    }

    set_log_level(log_level);

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
    if (ret < 0) {
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
    if (config_char == NULL) {
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
    LOG_DEBUG("In %s function", __func__);
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
    LOG_DEBUG("In %s function", __func__);
    if (cfg_mgr_interface != NULL) {
        // This needs to be freed manually by user
        // by calling cfgmgr_destroy() separately
        // if (cfg_mgr_interface->cfg_mgr) {
        //     cfgmgr_destroy(cfg_mgr_interface->cfg_mgr);
        // }
        if (cfg_mgr_interface->interface) {
            config_value_destroy(cfg_mgr_interface->interface);
        }
        free(cfg_mgr_interface);
    }
    LOG_DEBUG_0("cfgmgr_interface_t destroy: Done");
}