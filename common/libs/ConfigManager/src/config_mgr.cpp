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
 * @brief ConfigMgr Implementation
 * Holds the implementaion of APIs supported by ConfigMgr class
 */


#include "eis/config_manager/config_mgr.h"

using namespace eis::config_manager;

ConfigMgr::ConfigMgr() {

    // TODO: This will come from env
    setenv("ConfigManagerType", "ETCD", 1);

    // Fetching ConfigManager type from env
    char* c_type_name = (char*)malloc(sizeof(char) * 40);
    snprintf(c_type_name, 40, "%s", getenv("ConfigManagerType"));
    printf("ConfigManager selected is %s \n", c_type_name);
    std::string str_type_name(c_type_name);

    // Fetching & intializing dev mode variable
    bool dev_mode = false;
    char* c_dev_mode = (char*)malloc(sizeof(char) * 40);
    snprintf(c_dev_mode, 40, "%s", getenv("DEV_MODE"));
    printf("DEV mode is set to %s \n", c_dev_mode);
    std::string str_dev_mode(c_dev_mode);

    if(str_dev_mode == "True" || str_dev_mode == "TRUE" || str_dev_mode == "true") {
        dev_mode = true;
    }

    if(str_type_name == "ETCD") {

        // Creating etcd client instance
        db_client_t* db_client = (db_client_t*)malloc(sizeof(db_client_t));
        if(!dev_mode) {
            // TODO: Hard-coded certs will be removed soon
            db_client = create_etcd_client("localhost", "2379",
            "/home/vishwas/mult/IEdgeInsights/build/provision/Certificates/root/root_client_certificate.pem",
            "/home/vishwas/mult/IEdgeInsights/build/provision/Certificates/root/root_client_key.pem",
            "/home/vishwas/mult/IEdgeInsights/build/provision/Certificates/ca/ca_certificate.pem");
        } else {
            db_client = create_etcd_client("localhost", "2379", "", "", "");
        }

        // Initializing etcd client handle
        void *handle = db_client->init(db_client);

        // Fetching AppName from env
        char* c_app_name = (char*)malloc(sizeof(char) * 40);
        snprintf(c_app_name, 40, "%s", getenv("AppName"));
        std::string str_app_name(c_app_name);

        std::string str_app_interface = "/" + str_app_name + "/interfaces";
        char* interface_char = &str_app_interface[0];

        std::string str_app_config = "/" + str_app_name + "/config";
        char* config_char = &str_app_config[0];

        // Fetching App interfaces
        char* interface = db_client->get(handle, interface_char);
        // Fetching App config
        char* value = db_client->get(handle, config_char);

        m_app_config = json_config_new_from_buffer(value);
        m_app_interface = json_config_new_from_buffer(interface);

        m_etcd_handler = new AppCfg(m_app_config, m_app_interface, dev_mode);
        m_etcd_handler->m_app_name = str_app_name;
        m_etcd_handler->m_db_client_handle = db_client;
    } else if (str_type_name == "VAULT") {
        printf("ConfigManager type not supported yet");
    } else {
        printf("Invalid ConfigManager type");
    }
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
    // Initializing appname & db_client handle
    publisher->m_app_name = m_etcd_handler->m_app_name;
    publisher->m_db_client_handle = m_etcd_handler->m_db_client_handle;
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
            // Initializing appname & db_client handle
            publisher->m_app_name = m_etcd_handler->m_app_name;
            publisher->m_db_client_handle = m_etcd_handler->m_db_client_handle;
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
    // Initializing appname & db_client handle
    subscriber->m_app_name = m_etcd_handler->m_app_name;
    subscriber->m_db_client_handle = m_etcd_handler->m_db_client_handle;
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
        std::cout << s_config_name << std::endl;
        std::cout << s_name << std::endl;
        // Verifying subscriber config with name exists
        if(s_config_name == s_name) {
            m_etcd_handler->m_interface_cfg = sub_config;
            SubscriberCfg* subscriber = NULL;
            subscriber = new SubscriberCfg(m_etcd_handler->m_interface_cfg);
            // Initializing appname & db_client handle
            subscriber->m_app_name = m_etcd_handler->m_app_name;
            subscriber->m_db_client_handle = m_etcd_handler->m_db_client_handle;
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
    // Initializing appname & db_client handle
    server->m_app_name = m_etcd_handler->m_app_name;
    server->m_db_client_handle = m_etcd_handler->m_db_client_handle;
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
            // Initializing appname & db_client handle
            server->m_app_name = m_etcd_handler->m_app_name;
            server->m_db_client_handle = m_etcd_handler->m_db_client_handle;
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
    // Initializing appname & db_client handle
    client->m_app_name = m_etcd_handler->m_app_name;
    client->m_db_client_handle = m_etcd_handler->m_db_client_handle;
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
            // Initializing appname & db_client handle
            client->m_app_name = m_etcd_handler->m_app_name;
            client->m_db_client_handle = m_etcd_handler->m_db_client_handle;
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