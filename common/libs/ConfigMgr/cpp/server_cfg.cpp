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


#include "eis/config_manager/server_cfg.h"

using namespace eis::config_manager;

// Constructor
ServerCfg::ServerCfg():AppCfg(NULL, NULL, NULL) {
}

// m_cli_cfg getter
server_cfg_t* ServerCfg::getServCfg() {
    return m_serv_cfg;
}

// m_cli_cfg setter
void ServerCfg::setServCfg(server_cfg_t* serv_cfg) {
    m_serv_cfg = serv_cfg;
}

// m_app_cfg getter
app_cfg_t* ServerCfg::getAppCfg() {
    return m_app_cfg;
}

// m_app_cfg setter
void ServerCfg::setAppCfg(app_cfg_t* app_cfg) {
    m_app_cfg = app_cfg;
}

// getMsgBusConfig of ServerCfg class
config_t* ServerCfg::getMsgBusConfig() {
    // Calling the base C get_msgbus_config_server() API
    config_t* server_config = m_serv_cfg->get_msgbus_config_server(m_app_cfg->base_cfg);
    return server_config;
}

// To fetch endpoint from config
std::string ServerCfg::getEndpoint() {
    // Calling the base C get_endpoint_server() API
    char* ep = m_serv_cfg->get_endpoint_server(m_app_cfg->base_cfg);
    std::string s(ep);
    return s;
}

// To fetch list of allowed clients from config
std::vector<std::string> ServerCfg::getAllowedClients() {

    std::vector<std::string> client_list;
    // Calling the base C get_allowed_clients_server() API
    char** clients = m_serv_cfg->get_allowed_clients_server(m_app_cfg->base_cfg);
    for (char* c = *clients; c; c=*++clients) {
        std::string temp(c);
        client_list.push_back(temp);
    }
    return client_list;
}

// Destructor
ServerCfg::~ServerCfg() {
    if(m_serv_cfg) {
        delete m_serv_cfg;
    }
    if(m_app_cfg) {
        delete m_app_cfg;
    }
    LOG_INFO_0("ServerCfg destructor");
}