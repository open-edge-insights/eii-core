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
    bool dev_mode = true;
    char* c_dev_mode = (char*)malloc(sizeof(char) * 40);
    snprintf(c_dev_mode, 40, "%s", getenv("DEV_MODE"));
    printf("DEV mode is set to %s \n", c_dev_mode);
    std::string str_dev_mode(c_dev_mode);

    if(str_dev_mode == "True" || str_dev_mode == "TRUE" || str_dev_mode == "true") {
        dev_mode = true;
    }

    // Creating etcd client instance
    db_client_t* db_client = NULL;
    if(!dev_mode) {
        // TODO: Hard-coded certs will be removed soon
        db_client = create_etcd_client("localhost", "2379",
        "/home/vishwas/mult/IEdgeInsights/build/provision/Certificates/root/root_client_certificate.pem",
        "/home/vishwas/mult/IEdgeInsights/build/provision/Certificates/root/root_client_key.pem",
        "/home/vishwas/mult/IEdgeInsights/build/provision/Certificates/ca/ca_certificate.pem");
    } else {
        db_client = create_etcd_client("localhost", "2379", "", "", "");
    }

    if(str_type_name == "ETCD") {

        // Initializing etcd client handle
        void *handle = db_client->init(db_client);

        // Fetching AppName from env
        char* c_app_name = (char*)malloc(sizeof(char) * 40);
        snprintf(c_app_name, 40, "%s", getenv("AppName"));
        printf("AppName is %s \n", c_app_name);
        std::string str_app_name(c_app_name);

        std::string str_app_interface = "/" + str_app_name + "/interface";
        char* interface_char = &str_app_interface[0];

        std::string str_app_config = "/" + str_app_name + "/config";
        char* config_char = &str_app_config[0];

        // Fetching App interfaces
        char* interface = db_client->get(handle, interface_char);
        // Fetching App config
        char* value = db_client->get(handle, config_char);

        printf("Interface value is %s\n", interface);

        m_app_config = json_config_new_from_buffer(value);
        m_app_interface = json_config_new_from_buffer(interface);

        m_etcd_handler = new ConfigHandler(m_app_config, m_app_interface, dev_mode);
    } else if (str_type_name == "VAULT") {
        printf("ConfigManager type not supported yet");
    } else {
        printf("Invalid ConfigManager type");
    }
}

ConfigHandler* ConfigMgr::getAppConfig() {
    LOG_INFO_0("ConfigMgr getAppConfig  method");
    return m_etcd_handler;
}

PublisherCfg* ConfigMgr::getPublisherByIndex(int index) {
    LOG_INFO_0("ConfigHandler getPublisherByIndex method");

    config_value_t* publisher_interface = m_app_interface->get_config_value(m_app_interface->cfg, "Publish");
    m_etcd_handler->interface_cfg = config_value_array_get(publisher_interface, index);

    PublisherCfg* publisher = NULL;
    publisher = new PublisherCfg(m_etcd_handler->interface_cfg);
    return publisher;
}

SubscriberCfg* ConfigMgr::getSubscriberByIndex(int index) {
    LOG_INFO_0("ConfigHandler getSubscriberByIndex method");

    config_value_t* subscriber_interface = m_app_interface->get_config_value(m_app_interface->cfg, "Subscribe");
    m_etcd_handler->interface_cfg = config_value_array_get(subscriber_interface, index);

    SubscriberCfg* subscriber = NULL;
    subscriber = new SubscriberCfg(m_etcd_handler->interface_cfg);
    return subscriber;
}

ServerCfg* ConfigMgr::getServerByIndex(int index) {
    LOG_INFO_0("ConfigMgr getServerByIndex  method");

    config_value_t* server_interface = m_app_interface->get_config_value(m_app_interface->cfg, "Server");
    m_etcd_handler->interface_cfg = config_value_array_get(server_interface, index);

    ServerCfg* server = NULL;
    server = new ServerCfg(m_etcd_handler->interface_cfg);
    return server;
}

ClientCfg* ConfigMgr::getClientByIndex(int index) {
    LOG_INFO_0("ConfigMgr getClientByIndex  method");

    config_value_t* client_interface = m_app_interface->get_config_value(m_app_interface->cfg, "Client");
    m_etcd_handler->interface_cfg = config_value_array_get(client_interface, index);

    ClientCfg* client = NULL;
    client = new ClientCfg(m_etcd_handler->interface_cfg);
    return client;
}

ConfigMgr::~ConfigMgr() {
    // if(m_app_name) {
    //     delete m_app_name;
    // }
    // Stop the thread (if it is running)
    LOG_INFO_0("ConfigMgr destructor");
}