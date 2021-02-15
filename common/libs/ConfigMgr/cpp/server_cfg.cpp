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
 * @brief ServerCfg Implementation
 * Holds the implementaion of APIs supported by ServerCfg class
 */


#include "eii/config_manager/server_cfg.hpp"

using namespace eii::config_manager;

// Constructor
ServerCfg::ServerCfg(cfgmgr_interface_t* cfgmgr_interface):AppCfg(NULL) {
    m_cfgmgr_interface = cfgmgr_interface;
}

// m_cfgmgr_interface getter
cfgmgr_interface_t* ServerCfg::getServCfg() {
    return m_cfgmgr_interface;
}

// getMsgBusConfig of ServerCfg class
config_t* ServerCfg::getMsgBusConfig() {
    // Calling the base C get_msgbus_config_server() API
    config_t* server_config = cfgmgr_get_msgbus_config(m_cfgmgr_interface);
    if (server_config == NULL) {
        throw "Unable to fetch server msgbus config";
    }
    return server_config;
}

// Get the Interface Value of Server.
config_value_t* ServerCfg::getInterfaceValue(const char* key){
    config_value_t* interface_value = cfgmgr_get_interface_value(m_cfgmgr_interface, key);
    if (interface_value == NULL){
        throw "Getting interface value from base c layer failed";
    }
    return interface_value;
}

// To fetch endpoint from config
std::string ServerCfg::getEndpoint() {
    // Calling the base C get_endpoint_server() API
    config_value_t* ep = cfgmgr_get_endpoint(m_cfgmgr_interface);
    if (ep == NULL) {
        throw "Endpoint not found";
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

// To fetch list of allowed clients from config
std::vector<std::string> ServerCfg::getAllowedClients() {

    std::vector<std::string> client_list;
    // Calling the base C get_topics() API
    config_value_t* clients = cfgmgr_get_allowed_clients(m_cfgmgr_interface);
    if (clients == NULL) {
        throw "clients initialization failed";
    }
    config_value_t* client_value;

    size_t arr_len = config_value_array_len(clients);
    if (arr_len == 0) {
        throw "Empty array is not supported, atleast one value should be given.";
    }

    for (size_t i = 0; i < arr_len; i++) {
        client_value = config_value_array_get(clients, i);
        if (client_value == NULL) {
            throw "client_value initialization failed";
        }
        client_list.push_back(client_value->body.string);
        // Destroying client_value
        config_value_destroy(client_value);
    }
    // Destroying clients
    config_value_destroy(clients);
    return client_list;
}

// Destructor
ServerCfg::~ServerCfg() {
    if (m_cfgmgr_interface) {
        cfgmgr_interface_destroy(m_cfgmgr_interface);
    }
    LOG_INFO_0("ServerCfg destructor");
}