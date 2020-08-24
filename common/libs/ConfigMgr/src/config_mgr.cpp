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
 * @brief ConfigMgr Implementation
 * Holds the implementaion of APIs supported by ConfigMgr class
 */


#include "eis/config_manager/config_mgr.h"

#define MAX_CONFIG_KEY_LENGTH 250

using namespace eis::config_manager;

// local function to generate KV store config from env
config_t* createKVStoreConfig(bool dev_mode, std::string app_name) {
    // Creating cJSON object
    cJSON* c_json = cJSON_CreateObject();

    // Fetching ConfigManager type from env
    char* config_manager_type = getenv("KVStore");
    if(config_manager_type == NULL) {
        LOG_ERROR_0("KVStore env not set");
        return NULL;
    } else {
        size_t str_len = strlen(config_manager_type) + 1;
        char* c_type_name = (char*)malloc(sizeof(char) * str_len);
        snprintf(c_type_name, str_len, "%s", config_manager_type);
        LOG_INFO("ConfigManager selected is %s", c_type_name);
        std::string str_type_name(c_type_name);
        if(str_type_name != "etcd") {
            LOG_ERROR_0("KV store other than etcd not supported");
            return NULL;
        }
        cJSON_AddStringToObject(c_json, "type", "etcd");
    }

    // Creating etcd_kv_store object
    cJSON* etcd_kv_store = cJSON_CreateObject();
    cJSON_AddItemToObject(c_json, "etcd_kv_store", etcd_kv_store);

    // Fetching ETCD_HOST type from env
    char* etcd_host = getenv("ETCD_HOST");
    if(etcd_host == NULL) {
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

    int ret = 0;
    char pub_cert_file[MAX_CONFIG_KEY_LENGTH] = "";
    char pri_key_file[MAX_CONFIG_KEY_LENGTH] = "";
    char trust_file[MAX_CONFIG_KEY_LENGTH] = "";
    if(!dev_mode) {
        ret = snprintf(pub_cert_file, MAX_CONFIG_KEY_LENGTH,
                 "/run/secrets/etcd_%s_cert", app_name);
        if (ret < 0) {
            throw "failed to create pub_cert_file";
        }
        ret = snprintf(pri_key_file, MAX_CONFIG_KEY_LENGTH,
                 "/run/secrets/etcd_%s_key", app_name);
        if (ret < 0) {
            throw "failed to create pri_key_file";
        }
        ret = strncpy_s(trust_file, MAX_CONFIG_KEY_LENGTH + 1,
                  "/run/secrets/ca_etcd", MAX_CONFIG_KEY_LENGTH);
        if (ret != 0) {
            throw "failed to create trust file";
        }

        char* confimgr_cert = getenv("CONFIGMGR_CERT");
        char* confimgr_key = getenv("CONFIGMGR_KEY");
        char* confimgr_cacert = getenv("CONFIGMGR_CACERT");
        if(confimgr_cert && confimgr_key && confimgr_cacert) {
            ret = strncpy_s(pub_cert_file, MAX_CONFIG_KEY_LENGTH + 1,
                            confimgr_cert, MAX_CONFIG_KEY_LENGTH);
            if (ret != 0) {
                throw "failed to add cert to trust file";
            }
            ret = strncpy_s(pri_key_file, MAX_CONFIG_KEY_LENGTH + 1,
                            confimgr_key, MAX_CONFIG_KEY_LENGTH);
            if (ret !=0) {
                throw "failed to add key to trust file";
            }
            ret = strncpy_s(trust_file, MAX_CONFIG_KEY_LENGTH + 1,
                            confimgr_cacert, MAX_CONFIG_KEY_LENGTH);
            if (ret != 0 ){
                 throw "failed to add cacert to trust file";
            }
        }
        cJSON_AddStringToObject(etcd_kv_store, "cert_file", pub_cert_file);
        cJSON_AddStringToObject(etcd_kv_store, "key_file", pri_key_file);
        cJSON_AddStringToObject(etcd_kv_store, "ca_file", trust_file);
    } else {
        cJSON_AddStringToObject(etcd_kv_store, "cert_file", "");
        cJSON_AddStringToObject(etcd_kv_store, "key_file", "");
        cJSON_AddStringToObject(etcd_kv_store, "ca_file", "");
    }

    // Constructing char* object from cJSON object
    char* config_value_cr = cJSON_Print(c_json);
    LOG_DEBUG("KV store config is : %s \n", config_value_cr);

    // Constructing config_t object from cJSON object
    config_t* config = config_new(
            (void*) c_json, free_json, get_config_value);
    if (config == NULL) {
        LOG_ERROR_0("Failed to initialize configuration object");
        return NULL;
    }
    return config;
}

ConfigMgr::ConfigMgr() {

    // Fetching & intializing dev mode variable
    char* dev_mode_var = getenv("DEV_MODE");
    if(dev_mode_var == NULL) {
        LOG_ERROR_0("DEV_MODE env not set");
        return;
    }
    size_t str_len = strlen(dev_mode_var) + 1;
    char* c_dev_mode = (char*)malloc(sizeof(char) * str_len);
    snprintf(c_dev_mode, str_len, "%s", dev_mode_var);
    LOG_DEBUG("DEV mode is set to %s", c_dev_mode);
    std::string str_dev_mode(c_dev_mode);

    bool dev_mode = false;
    transform(str_dev_mode.begin(), str_dev_mode.end(), str_dev_mode.begin(), toupper);
    if(str_dev_mode == "TRUE") {
        dev_mode = true;
    }

    // Fetching & intializing AppName
    char* app_name_var = getenv("AppName");
    if(app_name_var == NULL) {
        LOG_ERROR_0("AppName env not set");
        return;
    }
    str_len = strlen(app_name_var) + 1;
    char* c_app_name = (char*)malloc(sizeof(char) * str_len);
    snprintf(c_app_name, str_len, "%s", app_name_var);
    LOG_DEBUG("AppName: %s", c_app_name);
    std::string str_app_name(c_app_name);

    // Creating kv_store config
    config_t* config = createKVStoreConfig(dev_mode, str_app_name);
    // Creating kv store client instance
    kv_store_client_t* kv_store_client = create_kv_client(config);

    // Initializing etcd client handle
    void *handle = kv_store_client->init(kv_store_client);

    // Fetching App interfaces
    std::string str_app_interface = "/" + str_app_name + "/interfaces";
    char* interface_char = &str_app_interface[0];
    char* interface = kv_store_client->get(handle, interface_char);

    // Fetching App config
    std::string str_app_config = "/" + str_app_name + "/config";
    char* config_char = &str_app_config[0];
    char* value = kv_store_client->get(handle, config_char);

    m_app_config = json_config_new_from_buffer(value);
    m_app_interface = json_config_new_from_buffer(interface);

    m_etcd_handler = new AppCfg(m_app_config, m_app_interface, dev_mode);
    m_etcd_handler->m_app_name = str_app_name;
    m_etcd_handler->m_kv_store_client_handle = kv_store_client;
}

AppCfg* ConfigMgr::getAppConfig() {
    LOG_INFO_0("ConfigMgr getAppConfig  method");
    return m_etcd_handler;
}

PublisherCfg* ConfigMgr::getPublisherByIndex(int index) {
    LOG_INFO_0("AppCfg getPublisherByIndex method");

    // Fetching list of Publisher interfaces
    config_value_t* publisher_interface = m_app_interface->get_config_value(m_app_interface->cfg, "Publishers");

    // Fetch publisher config associated with index
    m_etcd_handler->m_interface_cfg = config_value_array_get(publisher_interface, index);

    PublisherCfg* publisher = NULL;
    publisher = new PublisherCfg(m_etcd_handler->m_interface_cfg);
    // Initializing appname & kv_store_client handle
    publisher->m_app_name = m_etcd_handler->m_app_name;
    publisher->m_kv_store_client_handle = m_etcd_handler->m_kv_store_client_handle;
    // Return publisher config object based on index
    return publisher;
}

PublisherCfg* ConfigMgr::getPublisherByName(const char* name) {
    LOG_INFO_0("AppCfg getPublisherByName method");

    // Fetching list of Publisher interfaces
    config_value_t* publisher_interface = m_app_interface->get_config_value(m_app_interface->cfg, "Publishers");

    // Iterating through available publisher configs
    for(int i=0; i<config_value_array_len(publisher_interface); i++) {
        // Fetch name of individual publisher config
        config_value_t* pub_config = config_value_array_get(publisher_interface, i);
        config_value_t* pub_config_name = config_value_object_get(pub_config, "Name");
        std::string s_config_name(pub_config_name->body.string);
        std::string s_name(name);
        // Verifying publisher config with name exists
        if(s_config_name == s_name) {
            m_etcd_handler->m_interface_cfg = pub_config;
            PublisherCfg* publisher = NULL;
            publisher = new PublisherCfg(m_etcd_handler->m_interface_cfg);
            // Initializing appname & kv_store_client handle
            publisher->m_app_name = m_etcd_handler->m_app_name;
            publisher->m_kv_store_client_handle = m_etcd_handler->m_kv_store_client_handle;
            // Return publisher config object based on index
            return publisher;
        } else if(i == config_value_array_len(publisher_interface)) {
            LOG_ERROR("Publisher by name %s not found", name);
            return NULL;
        }
    }
}

SubscriberCfg* ConfigMgr::getSubscriberByIndex(int index) {
    LOG_INFO_0("AppCfg getSubscriberByIndex method");

    // Fetching list of Subscriber interfaces
    config_value_t* subscriber_interface = m_app_interface->get_config_value(m_app_interface->cfg, "Subscribers");

    // Fetch subscriber config associated with index
    m_etcd_handler->m_interface_cfg = config_value_array_get(subscriber_interface, index);

    SubscriberCfg* subscriber = NULL;
    subscriber = new SubscriberCfg(m_etcd_handler->m_interface_cfg);
    // Initializing appname & kv_store_client handle
    subscriber->m_app_name = m_etcd_handler->m_app_name;
    subscriber->m_kv_store_client_handle = m_etcd_handler->m_kv_store_client_handle;
    // Return subscriber config object based on index
    return subscriber;
}

SubscriberCfg* ConfigMgr::getSubscriberByName(const char* name) {
    LOG_INFO_0("AppCfg getSubscriberByName method");

    // Fetching list of Subscriber interfaces
    config_value_t* subscriber_interface = m_app_interface->get_config_value(m_app_interface->cfg, "Subscribers");

    // Iterating through available subscriber configs
    for(int i=0; i<config_value_array_len(subscriber_interface); i++) {
        // Fetch name of individual subscriber config
        config_value_t* sub_config = config_value_array_get(subscriber_interface, i);
        config_value_t* sub_config_name = config_value_object_get(sub_config, "Name");
        std::string s_config_name(sub_config_name->body.string);
        std::string s_name(name);
        // Verifying subscriber config with name exists
        if(s_config_name == s_name) {
            m_etcd_handler->m_interface_cfg = sub_config;
            SubscriberCfg* subscriber = NULL;
            subscriber = new SubscriberCfg(m_etcd_handler->m_interface_cfg);
            // Initializing appname & kv_store_client handle
            subscriber->m_app_name = m_etcd_handler->m_app_name;
            subscriber->m_kv_store_client_handle = m_etcd_handler->m_kv_store_client_handle;
            // Return subscriber config object based on name
            return subscriber;
        } else if(i == config_value_array_len(subscriber_interface)) {
            LOG_ERROR("Subscriber by name %s not found", name);
            return NULL;
        }
    }
}

ServerCfg* ConfigMgr::getServerByIndex(int index) {
    LOG_INFO_0("ConfigMgr getServerByIndex  method");

    // Fetching list of Server interfaces
    config_value_t* server_interface = m_app_interface->get_config_value(m_app_interface->cfg, "Servers");

    // Fetch server config associated with index
    m_etcd_handler->m_interface_cfg = config_value_array_get(server_interface, index);
    ServerCfg* server = NULL;
    server = new ServerCfg(m_etcd_handler->m_interface_cfg);
    // Initializing appname & kv_store_client handle
    server->m_app_name = m_etcd_handler->m_app_name;
    server->m_kv_store_client_handle = m_etcd_handler->m_kv_store_client_handle;
    // Return server config object based on index
    return server;
}

ServerCfg* ConfigMgr::getServerByName(const char* name) {
    LOG_INFO_0("AppCfg getServerByName method");

    // Fetching list of Server interfaces
    config_value_t* server_interface = m_app_interface->get_config_value(m_app_interface->cfg, "Servers");

    // Iterating through available server configs
    for(int i=0; i<config_value_array_len(server_interface); i++) {
        // Fetch name of individual server config
        config_value_t* server_config = config_value_array_get(server_interface, i);
        config_value_t* server_config_name = config_value_object_get(server_config, "Name");
        std::string s_config_name(server_config_name->body.string);
        std::string s_name(name);
        // Verifying server config with name exists
        if(s_config_name == s_name) {
            m_etcd_handler->m_interface_cfg = server_config;
            ServerCfg* server = NULL;
            server = new ServerCfg(m_etcd_handler->m_interface_cfg);
            // Initializing appname & kv_store_client handle
            server->m_app_name = m_etcd_handler->m_app_name;
            server->m_kv_store_client_handle = m_etcd_handler->m_kv_store_client_handle;
            // Return server config object based on name
            return server;
        } else if(i == config_value_array_len(server_interface)) {
            LOG_ERROR("Server by name %s not found", name);
            return NULL;
        }
    }
}

ClientCfg* ConfigMgr::getClientByIndex(int index) {
    LOG_INFO_0("ConfigMgr getClientByIndex  method");

    // Fetching list of Client interfaces
    config_value_t* client_interface = m_app_interface->get_config_value(m_app_interface->cfg, "Clients");

    // Fetch client config associated with index
    m_etcd_handler->m_interface_cfg = config_value_array_get(client_interface, index);

    ClientCfg* client = NULL;
    client = new ClientCfg(m_etcd_handler->m_interface_cfg);
    // Initializing appname & kv_store_client handle
    client->m_app_name = m_etcd_handler->m_app_name;
    client->m_kv_store_client_handle = m_etcd_handler->m_kv_store_client_handle;
    // Return client config object based on index
    return client;
}

ClientCfg* ConfigMgr::getClientByName(const char* name) {
    LOG_INFO_0("AppCfg getClientByName method");
    // Fetching list of Client interfaces
    config_value_t* client_interface = m_app_interface->get_config_value(m_app_interface->cfg, "Clients");

    // Iterating through available client configs
    for(int i=0; i<config_value_array_len(client_interface); i++) {
        config_value_t* client_config = config_value_array_get(client_interface, i);
        config_value_t* cli_config_name = config_value_object_get(client_config, "Name");
        std::string s_config_name(cli_config_name->body.string);
        std::string s_name(name);
        // Verifying client config with name exists
        if(s_config_name == s_name) {
            m_etcd_handler->m_interface_cfg = client_config;
            ClientCfg* client = NULL;
            client = new ClientCfg(m_etcd_handler->m_interface_cfg);
            // Initializing appname & kv_store_client handle
            client->m_app_name = m_etcd_handler->m_app_name;
            client->m_kv_store_client_handle = m_etcd_handler->m_kv_store_client_handle;
            // Return client config object based on index
            return client;
        } else if(i == config_value_array_len(client_interface)) {
            LOG_ERROR("Client by name %s not found", name);
            return NULL;
        }
    }
}

ConfigMgr::~ConfigMgr() {
    if(m_app_config) {
        delete m_app_config;
    }
    if(m_app_interface) {
        delete m_app_interface;
    }
    if(m_app_datastore) {
        delete m_app_datastore;
    }
    if(m_etcd_handler) {
        delete m_etcd_handler;
    }
    LOG_INFO_0("ConfigMgr destructor");
}