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


#include "eii/config_manager/subscriber_cfg.hpp"

using namespace eii::config_manager;

// Constructor
SubscriberCfg::SubscriberCfg(sub_cfg_t* sub_cfg, app_cfg_t* app_cfg):AppCfg(NULL) {
    m_sub_cfg = sub_cfg;
    m_app_cfg = app_cfg;
}

// m_cli_cfg getter
sub_cfg_t* SubscriberCfg::getSubCfg() {
    return m_sub_cfg;
}

// m_app_cfg getter
app_cfg_t* SubscriberCfg::getAppCfg() {
    return m_app_cfg;
}

// getMsgBusConfig of Subscriber class
config_t* SubscriberCfg::getMsgBusConfig() {
    // Calling the base C get_msgbus_config_sub() API
    config_t* sub_config = m_sub_cfg->cfgmgr_get_msgbus_config_sub(m_app_cfg->base_cfg, m_sub_cfg);
    if (sub_config == NULL) {
        LOG_ERROR_0("Unable to fetch subscriber msgbus config");
        return NULL;
    }
    return sub_config;
}

// Get the Interface Value of Subscriber.
config_value_t* SubscriberCfg::getInterfaceValue(const char* key){
    config_value_t* interface_value = m_sub_cfg->cfgmgr_get_interface_value_sub(m_sub_cfg, key);
    if(interface_value == NULL){
        LOG_DEBUG_0("[Subscriber]:Getting interface value from base c layer failed");
        return NULL;
    }
    return interface_value;
}

// To fetch endpoint from config
std::string SubscriberCfg::getEndpoint() {
    // Calling the base C get_endpoint_sub() API
    config_value_t* ep = m_sub_cfg->cfgmgr_get_endpoint_sub(m_sub_cfg);
    if (ep == NULL) {
        LOG_ERROR_0("Endpoint not found");
        return "";
    }
    
    char* value;
    value = cvt_obj_str_to_char(ep);
    if(value == NULL){
        LOG_ERROR_0("Endpoint object to string conversion failed");
        config_value_destroy(ep);
        return "";
    }

    std::string s(value);
    // Destroying ep
    config_value_destroy(ep);
    return s;
}

// To fetch topics from config
std::vector<std::string> SubscriberCfg::getTopics() {

    std::vector<std::string> topic_list;
    // Calling the base C get_topics() API
    config_value_t* topics = m_sub_cfg->cfgmgr_get_topics_sub(m_sub_cfg);
    if (topics == NULL) {
        LOG_ERROR_0("topics initialization failed");
        return {};
    }
    config_value_t* topic_value;
    for (size_t i = 0; i < config_value_array_len(topics); i++) {
        topic_value = config_value_array_get(topics, i);
        if (topic_value == NULL) {
            LOG_ERROR_0("topics initialization failed");
            config_value_destroy(topics);
            return {};
        }
        topic_list.push_back(topic_value->body.string);
        // Destroying topics
        config_value_destroy(topic_value);
    }
    // Destroying topics
    config_value_destroy(topics);
    return topic_list;
}

// To set topics in config
bool SubscriberCfg::setTopics(std::vector<std::string> topics_list) {

    int topics_length = topics_list.size();
    char **topics_to_be_set = (char**)calloc(topics_length, sizeof(char*));
    if (topics_to_be_set == NULL) {
        LOG_ERROR_0("calloc failed for topics_to_be_set");
        return false;
    }
    for (int i =0; i < topics_length; i++) {
        topics_to_be_set[i] = strdup(topics_list[i].c_str());
        if (topics_to_be_set[i] == NULL) {
            free_mem(topics_to_be_set);
            return false;
        }
    }
    // Calling the base C set_topics_sub() API
    int topics_set = m_sub_cfg->cfgmgr_set_topics_sub(topics_to_be_set, topics_length, m_app_cfg->base_cfg, m_sub_cfg);
    if(topics_set == 0) {
        LOG_INFO_0("Topics successfully set");
        free_mem(topics_to_be_set);
        return true;
    }
    // Freeing topics_to_be_set
    free_mem(topics_to_be_set);
    return false;
}

// Destructor
SubscriberCfg::~SubscriberCfg() {
   if (m_sub_cfg->sub_config != NULL) {
        config_value_destroy(m_sub_cfg->sub_config);
        free(m_sub_cfg);
    }
    LOG_INFO_0("SubscriberCfg destructor");
}