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
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.

#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "eii/msgbus/msgbus.h"
#include "eii/utils/logger.h"
#include "eii/utils/json_config.h"
#include "eii/config_manager/config_mgr.hpp"

#define TOPIC "publish_test"

using namespace eii::config_manager;

// Globals for cleaning up nicely
publisher_ctx_t* g_pub_ctx = NULL;
msg_envelope_t* g_msg = NULL;
void* g_msgbus_ctx = NULL;
ConfigMgr* g_ctx = NULL;

/**
 * Helper to initailize the message to be published
 */
void initialize_message() {
    // Creating message to be published
    msg_envelope_elem_body_t* integer = msgbus_msg_envelope_new_integer(42);
    msg_envelope_elem_body_t* fp = msgbus_msg_envelope_new_floating(55.5);
    g_msg = msgbus_msg_envelope_new(CT_JSON);
    msgbus_msg_envelope_put(g_msg, "hello", integer);
    msgbus_msg_envelope_put(g_msg, "world", fp);
}

/**
 * Signal handler
 */
void signal_handler(int signo) {
    LOG_INFO_0("Cleaning up");

    if (g_pub_ctx != NULL) {
        LOG_INFO_0("Freeing publisher");
        msgbus_publisher_destroy(g_msgbus_ctx, g_pub_ctx);
        g_pub_ctx = NULL;
    }
    if (g_msg != NULL) {
        LOG_INFO_0("Freeing message");
        msgbus_msg_envelope_destroy(g_msg);
        g_msg = NULL;
    }
    if (g_msgbus_ctx != NULL) {
        LOG_INFO_0("Freeing message bus context");
        msgbus_destroy(g_msgbus_ctx);
        g_msgbus_ctx = NULL;
    }
    if (g_ctx != NULL) {
        LOG_INFO_0("Freeing ConfigManager");
        delete g_ctx;
    }
    LOG_INFO_0("Done.");
}

int main(int argc, char** argv) {
    PublisherCfg* pub_ctx;

    // Set log level
    set_log_level(LOG_LVL_DEBUG);

    // Setting up signal handlers
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    // Fetching Publisher config from
    // VideoIngestion interface
    setenv("AppName","VideoIngestion", 1);
    std::string ep = "";
    config_value_t* interface_value;
    std::vector<std::string> topics;
    std::vector<std::string> newTopicsList;
    std::vector<std::string> clients;
    std::vector<std::string> topics_new;
    config_t* pub_config;
    bool topicsSet;
    try {
        // create ConfigMgr object
        g_ctx = new ConfigMgr();
    } catch (...) {
        LOG_ERROR_0("Exception occured");
        return -1;
    }

    // check if service is running in devmode
    bool dev_mode = g_ctx->isDevMode();
    if (dev_mode) {
        LOG_INFO_0("Running in DEV mode");
    } else {
        LOG_INFO_0("Running in PROD mode");
    }

    // get applictaion's AppName
    std::string app_name = g_ctx->getAppName();
    std::cout << "AppName :" << app_name << std::endl;

    // get number of publisher interfaces
    int num_of_publishers = g_ctx->getNumPublishers();
    LOG_DEBUG("Total number of publishers : %d", num_of_publishers);

    // get number of server interfaces
    int num_of_servers = g_ctx->getNumServers();
    LOG_DEBUG("Total number of servers : %d", num_of_servers);

    // Uncomment this for getting Publisher using name, else
    // user can use get publisher by index as mentioned below.

    // get the publisher object where publisher's interface 'Name' is 'default'
    // pub_ctx = g_ctx->getPublisherByName("default");
    // if (pub_ctx == NULL){
    //     LOG_ERROR_0("Failed to get publisher by name");
    //     goto err;
    // }

    // get 0th publisher interface object    
    pub_ctx = g_ctx->getPublisherByIndex(0);
    if (pub_ctx == NULL){
        LOG_ERROR_0("Failed to get publisher by index");
        goto err;
    }
    
    // Testing getEndpoint API
    // get Endpoint of a publisher interface
    ep = pub_ctx->getEndpoint();
    if(ep.empty()){
        LOG_ERROR_0("Failed to get endpoint");
        goto err;
    }
    LOG_INFO("Endpoint obtained : %s", ep.c_str());

    // get config_value_t object to get the value of client interface of key 'Name'
    interface_value = pub_ctx->getInterfaceValue("Name");
    if (interface_value == NULL) {
        LOG_ERROR_0("Failed to get expected interface value");
        goto err;
    }
    // print the value from object interface_value in string format
    LOG_INFO("Obtained interface value: %s", interface_value->body.string)

    // Testing getTopics API
    // get topics from publisher interface on which data will be published
    topics = pub_ctx->getTopics();
    if(topics.empty()){
        LOG_ERROR_0("Failed to get topics");
        goto err;
    }
    for (int i = 0; i < topics.size(); i++) {
        LOG_INFO("Pub Topics : %s", topics[i].c_str());
    }

    // Testing setTopics API
    
    newTopicsList.push_back("camera5_stream");
    newTopicsList.push_back("camera6_stream");

    // Update new set of topic for publisher's interface
    topicsSet = pub_ctx->setTopics(newTopicsList);

    // get 'Topics' from publisher interface
    topics_new = pub_ctx->getTopics();
    if(topics_new.empty()){
        LOG_ERROR_0("Failed to get topics_new");
        goto err;
    }
    for (int i = 0; i < topics_new.size(); i++) {
        LOG_INFO("Pub Topics : %s", topics_new[i].c_str());
    }

    // Testing getAllowedClients API
    // get 'AllowedClients' from publisher interface
    clients = pub_ctx->getAllowedClients();
    if(clients.empty()){
        LOG_ERROR_0("Failed to get getAllowedClients");
        goto err;
    }
    for (int i = 0; i < clients.size(); i++) {
        LOG_INFO("Allowed clients : %s", clients[i].c_str());
    }

    // get publisher msgbus config for application to communicate over EII message bus
    pub_config = pub_ctx->getMsgBusConfig();
    if (pub_config == NULL) {
        LOG_ERROR_0("Failed to get message bus config");
        goto err;
    }

    // Initializing Publisher using pub_config obtained
    // from new ConfigManager APIs
    g_msgbus_ctx = msgbus_initialize(pub_config);
    // Uncomment below line to test IPC mode
    // g_msgbus_ctx = msgbus_initialize(pub_config);
    if (g_msgbus_ctx == NULL) {
        LOG_ERROR_0("Failed to initialize message bus");
        goto err;
    }

    msgbus_ret_t ret;
    ret = msgbus_publisher_new(g_msgbus_ctx, topics[0].c_str(), &g_pub_ctx);
    if (ret != MSG_SUCCESS) {
        LOG_ERROR("Failed to initialize publisher (errno: %d)", ret);
        goto err;
    }

    // Initialize message to be published
    initialize_message();

    config_value_destroy(interface_value);
    delete pub_ctx;

    LOG_INFO_0("Running...");
    while (g_pub_ctx != NULL) {
        LOG_INFO_0("Publishing message");
        ret = msgbus_publisher_publish(g_msgbus_ctx, g_pub_ctx, g_msg);
        if (ret != MSG_SUCCESS) {
            LOG_ERROR("Failed to publish message (errno: %d)", ret);
            goto err;
        }
        sleep(1);
    }
    return 0;

err:
    if (g_pub_ctx != NULL)
        msgbus_publisher_destroy(g_msgbus_ctx, g_pub_ctx);
    if (g_msgbus_ctx != NULL)
        msgbus_destroy(g_msgbus_ctx);
    else if(pub_config != NULL)
        config_destroy(pub_config);
    if (g_ctx != NULL) {
        delete g_ctx;
    }
    return -1;
}
