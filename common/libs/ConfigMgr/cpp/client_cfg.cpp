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
 * @brief ClientCfg Implementation
 * Holds the implementaion of APIs supported by ClientCfg class
 */

#include "eis/config_manager/client_cfg.hpp"

using namespace eis::config_manager;

// Constructor
ClientCfg::ClientCfg(client_cfg_t* cli_cfg, app_cfg_t* app_cfg):AppCfg(NULL) {
    m_cli_cfg = cli_cfg;
    m_app_cfg = app_cfg;
}

// m_cli_cfg getter
client_cfg_t* ClientCfg::getCfg() {
    return m_cli_cfg;
}

// m_app_cfg getter
app_cfg_t* ClientCfg::getAppCfg() {
    return m_app_cfg;
}

// getMsgBusConfig of ClientCfg class
config_t* ClientCfg::getMsgBusConfig() {
    // Calling the base C get_msgbus_config_client() API
    config_t* cpp_client_config = m_cli_cfg->cfgmgr_get_msgbus_config_client(m_app_cfg->base_cfg);
    if (cpp_client_config == NULL) {
        LOG_ERROR_0("Unable to fetch client msgbus config");
        return NULL;
    }
    return cpp_client_config;
}

// Get the Interface Value of Client.
config_value_t* ClientCfg::getInterfaceValue(const char* key){
    return m_cli_cfg->cfgmgr_get_interface_value_client(m_app_cfg->base_cfg, key);
}

// To fetch endpoint from config
std::string ClientCfg::getEndpoint() {
    // Fetching EndPoint from config
    config_value_t* ep = m_cli_cfg->cfgmgr_get_endpoint_client(m_app_cfg->base_cfg);
    if (ep == NULL) {
        LOG_ERROR_0("Endpoint is not set");
        return NULL;
    }
    std::string s(ep->body.string);
    // Destroying ep
    config_value_destroy(ep);
    return s;
}

// Destructor
ClientCfg::~ClientCfg() {
    if(m_cli_cfg) {
        delete m_cli_cfg;
    }
    if(m_app_cfg) {
        delete m_app_cfg;
    }
    LOG_INFO_0("ClientCfg destructor");
}