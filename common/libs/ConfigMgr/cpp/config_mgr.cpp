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

ConfigMgr::ConfigMgr() {
    // Initializing C app_cfg object
    app_cfg_t* app_cfg = app_cfg_new();
    m_app_cfg_handler = new AppCfg(app_cfg->base_cfg->m_app_config, app_cfg->base_cfg->m_app_interface, NULL);
    m_app_cfg = app_cfg;
}

AppCfg* ConfigMgr::getAppConfig() {
    LOG_INFO_0("ConfigMgr getAppConfig  method");
    return m_app_cfg_handler;
}

PublisherCfg* ConfigMgr::getPublisherByIndex(int index) {
    LOG_INFO_0("AppCfg getPublisherByIndex method");
    // Calling the base C get_publisher_by_index API
    pub_cfg_t* pub_cfg = get_publisher_by_index(m_app_cfg, index);

    // Creating PublisherCfg object
    PublisherCfg* publisher = new PublisherCfg();
    publisher->setPubCfg(pub_cfg);
    publisher->setAppCfg(m_app_cfg);
    return publisher;
}

PublisherCfg* ConfigMgr::getPublisherByName(const char* name) {
    LOG_INFO_0("AppCfg getPublisherByName method");
    // Calling the base C get_publisher_by_name API
    pub_cfg_t* pub_cfg = get_publisher_by_name(m_app_cfg, name);

    // Creating PublisherCfg object
    PublisherCfg* publisher = new PublisherCfg();
    publisher->setPubCfg(pub_cfg);
    publisher->setAppCfg(m_app_cfg);
    return publisher;
}

SubscriberCfg* ConfigMgr::getSubscriberByIndex(int index) {
    LOG_INFO_0("AppCfg getSubscriberByIndex method");
    // Calling the base C get_subscriber_by_index API
    sub_cfg_t* sub_cfg = get_subscriber_by_index(m_app_cfg, index);

    // Creating SubscriberCfg object
    SubscriberCfg* subscriber = new SubscriberCfg();
    subscriber->setSubCfg(sub_cfg);
    subscriber->setAppCfg(m_app_cfg);
    return subscriber;
}

SubscriberCfg* ConfigMgr::getSubscriberByName(const char* name) {
    LOG_INFO_0("AppCfg getSubscriberByName method");
    // Calling the base C get_subscriber_by_name API
    sub_cfg_t* sub_cfg = get_subscriber_by_name(m_app_cfg, name);

    // Creating SubscriberCfg object
    SubscriberCfg* subscriber = new SubscriberCfg();
    subscriber->setSubCfg(sub_cfg);
    subscriber->setAppCfg(m_app_cfg);
    return subscriber;
}

ServerCfg* ConfigMgr::getServerByIndex(int index) {
    LOG_INFO_0("AppCfg getServerByIndex method");

    // Calling the base C get_server_by_index API
    server_cfg_t* serv_cfg = get_server_by_index(m_app_cfg, index);

    // Creating ServerCfg object
    ServerCfg* server = new ServerCfg();
    server->setServCfg(serv_cfg);
    server->setAppCfg(m_app_cfg);
    return server;
}

ServerCfg* ConfigMgr::getServerByName(const char* name) {
    LOG_INFO_0("AppCfg getServerByName method");

    // Calling the base C get_server_by_name API
    server_cfg_t* serv_cfg = get_server_by_name(m_app_cfg, name);

    // Creating ServerCfg object
    ServerCfg* server = new ServerCfg();
    server->setServCfg(serv_cfg);
    server->setAppCfg(m_app_cfg);
    return server;
}

ClientCfg* ConfigMgr::getClientByIndex(int index) {
    LOG_INFO_0("AppCfg getClientByName method");

    // Calling the base C get_client_by_name API
    client_cfg_t* cli_cfg = get_client_by_index(m_app_cfg, index);

    // Creating ClientCfg object
    ClientCfg* client = new ClientCfg();
    client->setCliCfg(cli_cfg);
    client->setAppCfg(m_app_cfg);
    return client;
}

ClientCfg* ConfigMgr::getClientByName(const char* name) {
    LOG_INFO_0("AppCfg getClientByName method");

    // Calling the base C get_client_by_name API
    client_cfg_t* cli_cfg = get_client_by_name(m_app_cfg, name);

    // Creating ClientCfg object
    ClientCfg* client = new ClientCfg();
    client->setCliCfg(cli_cfg);
    client->setAppCfg(m_app_cfg);
    return client;
}

ConfigMgr::~ConfigMgr() {
    if(m_app_cfg) {
        delete m_app_cfg;
    }
    if(m_app_cfg_handler) {
        delete m_app_cfg_handler;
    }
    LOG_INFO_0("ConfigMgr destructor");
}