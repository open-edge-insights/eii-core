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


#include "eii/config_manager/config_mgr.hpp"

#define MAX_CONFIG_KEY_LENGTH 250

using namespace eii::config_manager;

ConfigMgr::ConfigMgr() {
    // Initializing C app_cfg object
    cfgmgr_ctx_t* cfgmgr = NULL;
    cfgmgr = cfgmgr_initialize();
    if (cfgmgr != NULL) {
        m_app_cfg_handler = new AppCfg(cfgmgr);
        m_cfgmgr = cfgmgr;
    } else {
        LOG_ERROR_0("cfgmgr initialization failed");
        throw("cfgmgr initialization failed");
    }
}

ConfigMgr::ConfigMgr(const ConfigMgr& src) {
    throw "This object should not be copied";
}

ConfigMgr& ConfigMgr::operator=(const ConfigMgr& src) {
    return *this;
}

AppCfg* ConfigMgr::getAppConfig() {
    LOG_DEBUG_0("ConfigMgr getAppConfig  method");
    return m_app_cfg_handler;
}

int ConfigMgr::getNumPublishers() {
    // Calling the base C cfgmgr_get_num_elements_base API
    int result = cfgmgr_get_num_publishers(m_cfgmgr);
    if (result == -1) {
        LOG_ERROR_0("Failed to fetch number of pulishers");
    }
    return result;
}

int ConfigMgr::getNumSubscribers() {
    // Calling the base C cfgmgr_get_num_elements_base API
    int result = cfgmgr_get_num_subscribers(m_cfgmgr);
    if (result == -1) {
        LOG_ERROR_0("Failed to fetch number of subscribers");
    }
    return result;
}

int ConfigMgr::getNumServers() {
    // Calling the base C cfgmgr_get_num_elements_base API
    int result = cfgmgr_get_num_servers(m_cfgmgr);
    if (result == -1) {
        LOG_ERROR_0("Failed to fetch number of servers");
    }
    return result;
}

int ConfigMgr::getNumClients() {
    // Calling the base C cfgmgr_get_num_elements_base API
    int result = cfgmgr_get_num_clients(m_cfgmgr);
    if (result == -1) {
        LOG_ERROR_0("Failed to fetch number of clients");
    }
    return result;
}

bool ConfigMgr::isDevMode() {
    // Calling the base C cfgmgr_is_dev_mode_base API
    return cfgmgr_is_dev_mode(m_cfgmgr);
}

std::string ConfigMgr::getAppName() {
    // Calling the base C cfgmgr_get_appname_base API
    config_value_t* appname = cfgmgr_get_appname(m_cfgmgr);
    if (appname == NULL) {
        throw "AppName is NULL";
    }

    if (appname->type != CVT_STRING) {
        throw "appname type is not string";
    } else if (appname->body.string == NULL) {
        throw "AppName is NULL";
    }
    std::string app_name(appname->body.string);
    config_value_destroy(appname);
    return app_name;
}

PublisherCfg* ConfigMgr::getPublisherByIndex(int index) {
    LOG_DEBUG("In %s method", __func__);
    // Calling the base C get_publisher_by_index API
    cfgmgr_interface_t* cfgmgr_interface = cfgmgr_get_publisher_by_index(m_cfgmgr, index);
    if (cfgmgr_interface == NULL) {
        throw "cfgmgr_interface initialization failed";
    }

    // Creating PublisherCfg object
    PublisherCfg* publisher = new PublisherCfg(cfgmgr_interface);
    return publisher;
}

PublisherCfg* ConfigMgr::getPublisherByName(const char* name) {
    LOG_DEBUG("In %s method", __func__);
    // Calling the base C get_publisher_by_name API
    cfgmgr_interface_t* cfgmgr_interface = cfgmgr_get_publisher_by_name(m_cfgmgr, name);
    if (cfgmgr_interface == NULL) {
        throw "cfgmgr_interface initialization failed";
    }

    // Creating PublisherCfg object
    PublisherCfg* publisher = new PublisherCfg(cfgmgr_interface);
    return publisher;
}

SubscriberCfg* ConfigMgr::getSubscriberByIndex(int index) {
    LOG_DEBUG("In %s method", __func__);
    // Calling the base C get_subscriber_by_index API
    cfgmgr_interface_t* cfgmgr_interface = cfgmgr_get_subscriber_by_index(m_cfgmgr, index);
    if (cfgmgr_interface == NULL) {
        throw "cfgmgr_interface initialization failed";
    }

    // Creating SubscriberCfg object
    SubscriberCfg* subscriber = new SubscriberCfg(cfgmgr_interface);
    return subscriber;
}

SubscriberCfg* ConfigMgr::getSubscriberByName(const char* name) {
    LOG_DEBUG("In %s method", __func__);
    // Calling the base C get_subscriber_by_name API
    cfgmgr_interface_t* cfgmgr_interface = cfgmgr_get_subscriber_by_name(m_cfgmgr, name);
    if (cfgmgr_interface == NULL) {
        throw "cfgmgr_interface initialization failed";
    }

    // Creating SubscriberCfg object
    SubscriberCfg* subscriber = new SubscriberCfg(cfgmgr_interface);
    return subscriber;
}

ServerCfg* ConfigMgr::getServerByIndex(int index) {
    LOG_DEBUG("In %s method", __func__);
    // Calling the base C get_server_by_index API
    cfgmgr_interface_t* cfgmgr_interface = cfgmgr_get_server_by_index(m_cfgmgr, index);
    if (cfgmgr_interface == NULL) {
        throw "cfgmgr_interface initialization failed";
    }

    // Creating ServerCfg object
    ServerCfg* server = new ServerCfg(cfgmgr_interface);
    return server;
}

ServerCfg* ConfigMgr::getServerByName(const char* name) {
    LOG_DEBUG("In %s method", __func__);
    // Calling the base C get_server_by_name API
    cfgmgr_interface_t* cfgmgr_interface = cfgmgr_get_server_by_name(m_cfgmgr, name);
    if (cfgmgr_interface == NULL) {
        throw "cfgmgr_interface initialization failed";
    }

    // Creating ServerCfg object
    ServerCfg* server = new ServerCfg(cfgmgr_interface);
    return server;
}

ClientCfg* ConfigMgr::getClientByIndex(int index) {
    LOG_DEBUG("In %s method", __func__);
    // Calling the base C get_client_by_name API
    cfgmgr_interface_t* cfgmgr_interface = cfgmgr_get_client_by_index(m_cfgmgr, index);
    if (cfgmgr_interface == NULL) {
        throw "cfgmgr_interface initialization failed";
    }

    // Creating ClientCfg object
    ClientCfg* client = new ClientCfg(cfgmgr_interface);
    return client;
}

ClientCfg* ConfigMgr::getClientByName(const char* name) {
    LOG_DEBUG("In %s method", __func__);
    // Calling the base C get_client_by_name API
    cfgmgr_interface_t* cfgmgr_interface = cfgmgr_get_client_by_name(m_cfgmgr, name);
    if (cfgmgr_interface == NULL) {
        throw "cfgmgr_interface initialization failed";
    }

    // Creating ClientCfg object
    ClientCfg* client = new ClientCfg(cfgmgr_interface);
    return client;
}

ConfigMgr::~ConfigMgr() {
    LOG_DEBUG("In %s method", __func__);
    if (m_app_cfg_handler) {
        LOG_DEBUG_0("ConfigMgr Destructor: Deleting m_app_cfg_handler class...");
        delete m_app_cfg_handler;
    }
    if (m_cfgmgr) {
        cfgmgr_destroy(m_cfgmgr);
    }
}