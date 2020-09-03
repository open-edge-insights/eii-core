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
 * @brief PublisherCfg Implementation
 * Holds the implementaion of APIs supported by PublisherCfg class
 */


#include "eis/config_manager/publisher_cfg.h"

using namespace eis::config_manager;

// Constructor
PublisherCfg::PublisherCfg():AppCfg(NULL, NULL, NULL) {
}

// m_pub_cfg getter
pub_cfg_t* PublisherCfg::getPubCfg() {
    return m_pub_cfg;
}

// m_pub_cfg setter
void PublisherCfg::setPubCfg(pub_cfg_t* pub_cfg) {
    m_pub_cfg = pub_cfg;
}

// m_app_cfg getter
app_cfg_t* PublisherCfg::getAppCfg() {
    return m_app_cfg;
}

// m_app_cfg setter
void PublisherCfg::setAppCfg(app_cfg_t* app_cfg) {
    m_app_cfg = app_cfg;
}

// getMsgBusConfig of Publisher class
config_t* PublisherCfg::getMsgBusConfig() {
    // Calling the base C get_msgbus_config() API
    config_t* pub_config = m_pub_cfg->get_msgbus_config(m_app_cfg->base_cfg);
    return pub_config;
}

// To fetch endpoint from config
std::string PublisherCfg::getEndpoint() {
    // Calling the base C get_endpoint() API
    char* ep = m_pub_cfg->get_endpoint(m_app_cfg->base_cfg);
    std::string s(ep);
    return s;
}

// To fetch topics from config
std::vector<std::string> PublisherCfg::getTopics() {

    std::vector<std::string> topic_list;
    // Calling the base C get_topics() API
    char** topics = m_pub_cfg->get_topics(m_app_cfg->base_cfg);
    for (char* c = *topics; c; c=*++topics) {
        std::string temp(c);
        topic_list.push_back(temp);
    }
    return topic_list;
}

// To set topics in config
bool PublisherCfg::setTopics(std::vector<std::string> topics_list) {

    char **topics_to_be_set = (char**)calloc(topics_list.size(), sizeof(char*));
    for (int i =0; i < topics_list.size(); i++) {
        topics_to_be_set[i] = strdup(topics_list[i].c_str());
    }
    // Calling the base C set_topics() API
    int topics_set = m_pub_cfg->set_topics(topics_to_be_set, m_app_cfg->base_cfg);
    if(topics_set == 0) {
        LOG_INFO_0("Topics successfully set");
        return true;
    }
    return false;
}

// To fetch list of allowed clients from config
std::vector<std::string> PublisherCfg::getAllowedClients() {

    std::vector<std::string> client_list;
    // Calling the base C get_allowed_clients() API
    char** clients = m_pub_cfg->get_allowed_clients(m_app_cfg->base_cfg);
    for (char* c = *clients; c; c=*++clients) {
        std::string temp(c);
        client_list.push_back(temp);
    }
    return client_list;
}

// Destructor
PublisherCfg::~PublisherCfg() {
    if(m_pub_cfg) {
        delete m_pub_cfg;
    }
    if(m_app_cfg) {
        delete m_app_cfg;
    }
    LOG_INFO_0("PublisherCfg destructor");
}