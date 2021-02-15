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

#include "eii/config_manager/client_cfg.hpp"

using namespace eii::config_manager;

// Constructor
ClientCfg::ClientCfg(cfgmgr_interface_t* cfgmgr_interface):AppCfg(NULL) {
    m_cfgmgr_interface = cfgmgr_interface;
}

// m_cfgmgr_interface getter
cfgmgr_interface_t* ClientCfg::getCfg() {
    return m_cfgmgr_interface;
}

// getMsgBusConfig of ClientCfg class
config_t* ClientCfg::getMsgBusConfig() {
    // Calling the base C get_msgbus_config_client() API
    config_t* cpp_client_config = cfgmgr_get_msgbus_config(m_cfgmgr_interface);
    if (cpp_client_config == NULL) {
        throw "Unable to fetch client msgbus config";
    }
    return cpp_client_config;
}

// Get the Interface Value of Client.
config_value_t* ClientCfg::getInterfaceValue(const char* key){
    config_value_t* interface_value = cfgmgr_get_interface_value(m_cfgmgr_interface, key);
    if (interface_value == NULL) {
        throw "Getting interface value from base c layer failed";
    }
    return interface_value;
}

// To fetch endpoint from config
std::string ClientCfg::getEndpoint() {
    // Fetching EndPoint from config
    config_value_t* ep = cfgmgr_get_endpoint(m_cfgmgr_interface);
    if (ep == NULL) {
        throw "Endpoint is not set";
    }

    char* value;
    value = cvt_obj_str_to_char(ep);
    if (value == NULL) {
        throw "Endpoint object to string conversion failed";
    }

    std::string s(value);
    // Destroying ep
    config_value_destroy(ep);
    return s;
}

// Destructor
ClientCfg::~ClientCfg() {
    if (m_cfgmgr_interface) {
        cfgmgr_interface_destroy(m_cfgmgr_interface);
    }
    LOG_INFO_0("ClientCfg destructor");
}