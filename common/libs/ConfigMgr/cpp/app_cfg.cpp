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
 * @brief AppCfg Implementation
 * Holds the implementaion of APIs supported by AppCfg class
 */

#include "eis/config_manager/app_cfg.hpp"
#include "eis/config_manager/cfgmgr.h"

using namespace eis::config_manager;
using namespace std;


AppCfg::AppCfg(cfgmgr_ctx_t* cfgmgr) {
    m_app_config = NULL;
    m_app_interface = NULL;
    m_app_data_store = NULL;
    m_cfgmgr = cfgmgr;
}

config_t* AppCfg::getConfig() {
    m_app_config = cfgmgr_get_app_config(m_cfgmgr);
    if (m_app_config == NULL) {
        LOG_ERROR_0("App Config not set");
        return NULL;
    }
    return m_app_config;
}

config_t* AppCfg::getInterface() {
    m_app_interface = cfgmgr_get_app_interface(m_cfgmgr);
    if (m_app_interface == NULL) {
        LOG_ERROR_0("App Interface not set");
        return NULL;
    }
    return m_app_interface;
}

config_value_t* AppCfg::getConfigValue(const char* key) {
    config_value_t* value = cfgmgr_get_app_config_value(m_cfgmgr, key);
    if (value == NULL) {
        LOG_ERROR_0("Unable to fetch config value");
        return NULL;
    }
    return value;
}


bool AppCfg::watch(const char* key, cfgmgr_watch_callback_t watch_callback, void* user_data) {
    try {
        // Calling the base cfgmgr_watch C API
        cfgmgr_watch(m_cfgmgr, key, watch_callback, user_data);
    } catch(std::exception const & ex) {
        LOG_ERROR("Exception Occurred in cfgmgr_watch_prefix() API with the error: %s", ex.what());
        return false;
    }
    return true;
}

bool AppCfg::watchPrefix(char* prefix, cfgmgr_watch_callback_t watch_callback, void* user_data) {
    try {
        // Calling the base cfgmgr_watch_prefix C API
        cfgmgr_watch_prefix(m_cfgmgr, prefix, watch_callback, user_data);
    } catch(std::exception const & ex) {
        LOG_ERROR("Exception Occurred in cfgmgr_watch_prefix() API with the error: %s", ex.what());
        return false;
    }
    return true;
}

bool AppCfg::watchConfig(cfgmgr_watch_callback_t watch_callback, void* user_data) {
    // Creating /<AppName>/config key
    std::string config_key = "/" + std::string(m_cfgmgr->app_name) + "/config";
    try {
        // Calling the base cfgmgr_watch C API
        cfgmgr_watch(m_cfgmgr, (char*)config_key.c_str(), watch_callback, user_data);
    } catch(std::exception const & ex) {
        LOG_ERROR("Exception Occurred in cfgmgr_watch_prefix() API with the error: %s", ex.what());
        return false;
    }
    return true;
}

bool AppCfg::watchInterface(cfgmgr_watch_callback_t watch_callback, void* user_data) {
    // Creating /<AppName>/interfaces key
    std::string interface_key = "/" + std::string(m_cfgmgr->app_name) + "/interfaces";
    try {
        // Calling the base cfgmgr_watch C API
        cfgmgr_watch(m_cfgmgr, (char*)interface_key.c_str(), watch_callback, user_data);
    } catch(std::exception const & ex) {
        LOG_ERROR("Exception Occurred in cfgmgr_watch_prefix() API with the error: %s", ex.what());
        return false;
    }
    return true;
}

// This virtual method is implemented
// by sub class objects
config_t* AppCfg::getMsgBusConfig() {
    return NULL;
}

// This virtual method is implemented
// by sub class objects
std::string AppCfg::getEndpoint() {
    return NULL;
}

// This virtual method is implemented
// by sub class objects
config_value_t* AppCfg::getInterfaceValue(const char* key){
    return NULL;
}

// This virtual method is implemented
// by sub class objects
std::vector<std::string> AppCfg::getTopics() {
    std::vector <std::string> temp; 
    return temp; 

}

// This virtual method is implemented
// by sub class objects
std::vector<std::string> AppCfg::getAllowedClients() {
    std::vector <std::string> temp; 
    return temp; 
}

// This virtual method is implemented
// by sub class objects
bool AppCfg::setTopics(std::vector<std::string> topics_list) {
    return false;
}

// tokenizer function to split string based on delimiter
vector<string> AppCfg::tokenizer(const char* str, const char* delim) {

    std::string line(str);
    std::stringstream str1(line);

    // Vector of string to save tokens
    vector<std::string> tokens;

    std::string temp;

    // Tokenizing w.r.t. delimiter
    while (getline(str1, temp, ':')) {
        tokens.push_back(temp);
    }

    return tokens;
}


AppCfg::~AppCfg() {
    LOG_DEBUG_0("AppCfg destructor");
    if (m_cfgmgr) {
        cfgmgr_destroy(m_cfgmgr);
    }
    LOG_DEBUG_0("AppCfg destructor: Done");
}
