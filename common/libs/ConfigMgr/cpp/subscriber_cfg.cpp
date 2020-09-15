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
 * @brief SubscriberCfg Implementation
 * Holds the implementaion of APIs supported by SubscriberCfg class
 */


#include "eis/config_manager/subscriber_cfg.h"

using namespace eis::config_manager;

// Constructor
SubscriberCfg::SubscriberCfg():AppCfg(NULL) {
}

// m_cli_cfg getter
sub_cfg_t* SubscriberCfg::getSubCfg() {
    return m_sub_cfg;
}

// m_cli_cfg setter
void SubscriberCfg::setSubCfg(sub_cfg_t* serv_cfg) {
    m_sub_cfg = serv_cfg;
}

// m_app_cfg getter
app_cfg_t* SubscriberCfg::getAppCfg() {
    return m_app_cfg;
}

// m_app_cfg setter
void SubscriberCfg::setAppCfg(app_cfg_t* app_cfg) {
    m_app_cfg = app_cfg;
}

// getMsgBusConfig of Subscriber class
config_t* SubscriberCfg::getMsgBusConfig() {
    // Calling the base C get_msgbus_config_sub() API
    config_t* sub_config = m_sub_cfg->get_msgbus_config_sub(m_app_cfg->base_cfg);
    return sub_config;
}

// To fetch endpoint from config
std::string SubscriberCfg::getEndpoint() {
    // Calling the base C get_endpoint_sub() API
    char* ep = m_sub_cfg->get_endpoint_sub(m_app_cfg->base_cfg);
    std::string s(ep);
    return s;
}

// To fetch topics from config
std::vector<std::string> SubscriberCfg::getTopics() {
    std::vector<std::string> topic_list;
    // Calling the base C get_topics_sub() API
    char** abcd = m_sub_cfg->get_topics_sub(m_app_cfg->base_cfg);
    for (char* c = *abcd; c; c=*++abcd) {
        std::string temp(c);
        topic_list.push_back(temp);
    }
    return topic_list;
}

// To set topics in config
bool SubscriberCfg::setTopics(std::vector<std::string> topics_list) {
    char **topics_to_be_set = (char**)calloc(topics_list.size(), sizeof(char*));
    for (int i =0; i < topics_list.size(); i++) {
        topics_to_be_set[i] = strdup(topics_list[i].c_str());
    }
    // Calling the base C set_topics_sub() API
    int topics_set = m_sub_cfg->set_topics_sub(topics_to_be_set, m_app_cfg->base_cfg);
    if(topics_set == 0) {
        LOG_INFO_0("Topics successfully set");
        return true;
    }
    return false;
}

// Destructor
SubscriberCfg::~SubscriberCfg() {
    if(m_sub_cfg) {
        delete m_sub_cfg;
    }
    if(m_app_cfg) {
        delete m_app_cfg;
    }
    LOG_INFO_0("SubscriberCfg destructor");
}