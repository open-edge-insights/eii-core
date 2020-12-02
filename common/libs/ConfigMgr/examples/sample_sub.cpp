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

#include "eis/msgbus/msgbus.h"
#include "eis/utils/logger.h"
#include "eis/utils/json_config.h"
#include "eis/config_manager/config_mgr.hpp"

#define TOPIC "publish_test"

using namespace eis::config_manager;

// Globals for cleaning up nicely
recv_ctx_t* g_sub_ctx = NULL;
void* g_msgbus_ctx = NULL;
ConfigMgr* g_sub_ch = NULL;
ConfigMgr* g_sub_ch_vis = NULL;

/**
 * Signal handler
 */
void signal_handler(int signo) {
    LOG_INFO_0("Cleaning up");
    if(g_sub_ctx != NULL) {
        LOG_INFO_0("Freeing publisher");
        msgbus_recv_ctx_destroy(g_msgbus_ctx, g_sub_ctx);
        g_sub_ctx = NULL;
    }
    if(g_msgbus_ctx != NULL) {
        LOG_INFO_0("Freeing message bus context");
        msgbus_destroy(g_msgbus_ctx);
        g_msgbus_ctx = NULL;
    }
    if (g_sub_ch != NULL) {
        LOG_INFO_0("Freeing g_sub_ch");
        delete g_sub_ch;
    }
    if (g_sub_ch_vis != NULL) {
        LOG_INFO_0("Freeing g_sub_ch_vis");
        delete g_sub_ch_vis;
    }
    LOG_INFO_0("Done.");
}

int main(int argc, char** argv) {

    // Set log level
    set_log_level(LOG_LVL_DEBUG);

    // Setting up signal handlers
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    msg_envelope_t* msg = NULL;
    msg_envelope_serialized_part_t* parts = NULL;
    msgbus_ret_t ret;
    bool topicsSet;
    int num_parts = 0;

    // Fetching Subscriber config from
    // VideoAnalytics interface
    setenv("AppName","VideoAnalytics", 1);
    config_t* sub_config = NULL;
    std::vector<std::string> newTopicsList;

    try {
        // create ConfigMgr object
        g_sub_ch = new ConfigMgr();
    } catch (...) {
        LOG_ERROR_0("Exception occured");
        return -1;
    }
    // Testing getNumSubscribers()
    // get number of subscriber interfaces
    int num_of_subscribers = g_sub_ch->getNumSubscribers();
    LOG_DEBUG("Total number of subscribers: %d", num_of_subscribers );

    // get the subscriber object where server's interface 'Name' is 'default'
    // SubscriberCfg* sub_ctx = g_sub_ch->getSubscriberByName("default");
    // if(sub_ctx == NULL){
    //     LOG_ERROR_0("get subscriber by name failed");
    // }

    // get 0th subscriber interface object  
    SubscriberCfg* sub_ctx = g_sub_ch->getSubscriberByIndex(0);
    if(sub_ctx == NULL){
        LOG_ERROR_0("get subscriber by index failed");
        exit(-1);
    }

    // Testing getEndpoint API
    // get Endpoint of a subscriber interface
    std::string ep = sub_ctx->getEndpoint();
    if(ep.empty()){
        LOG_ERROR_0("get subscriber endpoints failed");
        exit(-1);
    }
    LOG_INFO("Endpoint obtained : %s", ep.c_str());

    // Testing getTopics API
    // get topics from subscriber interface
    std::vector<std::string> topics = sub_ctx->getTopics();
    if (topics.empty()) {
        LOG_ERROR_0("Get topics failed");
        exit(-1);
    }

    for(int i = 0; i < topics.size(); i++) {
        LOG_INFO("Sub Topics : %s", topics[i].c_str());
    }
  
    // Testing getInterfaceValue()
    // get config_value_t object to get the value of subscriber interface of key 'Name'
    config_value_t* interface_value = sub_ctx->getInterfaceValue("Name");
    if (interface_value == NULL || interface_value->type != CVT_STRING){
        LOG_ERROR_0("Failed to get expected interface value");
        exit(-1);
    }

    // get the value from object interface_value in string format
    LOG_INFO("interface value is %s", interface_value->body.string);
    config_value_destroy(interface_value);

    // get subscriber msgbus config for application to communicate over EIS message bus
    sub_config = sub_ctx->getMsgBusConfig();
    if(sub_config == NULL){
        LOG_ERROR_0("get subscriber msgbus config failed");
        exit(-1);
    }

    // Testing setTopics API
    newTopicsList.push_back("camera7_stream");
    newTopicsList.push_back("camera8_stream");

    // Update new set of topic for subscriber's interface
    topicsSet = sub_ctx->setTopics(newTopicsList);

    g_msgbus_ctx = msgbus_initialize(sub_config);
    if(g_msgbus_ctx == NULL) {
        LOG_ERROR_0("Failed to initialize message bus");
        goto err;
    }

 

    ret = msgbus_subscriber_new(g_msgbus_ctx, topics[0].c_str(), NULL, &g_sub_ctx);

    if(ret != MSG_SUCCESS) {
        LOG_ERROR("Failed to initialize subscriber (errno: %d)", ret);
        goto err;
    }

    // free sub_config
    config_destroy(sub_config);
    // free sub_ctx
    delete sub_ctx;

    LOG_INFO_0("Running...");
    while(g_sub_ctx != NULL) {
        ret = msgbus_recv_wait(g_msgbus_ctx, g_sub_ctx, &msg);
        if(ret != MSG_SUCCESS) {
            // Interrupt is an acceptable error
            if(ret == MSG_ERR_EINTR) {
                goto err;
            }
            LOG_ERROR("Failed to receive message (errno: %d)", ret);
            goto err;
        }
        LOG_INFO("Topic in the received message on subscriber is %s \n", msg->name);
        num_parts = msgbus_msg_envelope_serialize(msg, &parts);
        if(num_parts <= 0) {
            LOG_ERROR_0("Failed to serialize message");
            goto err;
        }

        LOG_INFO("Received: %s", parts[0].bytes);

        msgbus_msg_envelope_serialize_destroy(parts, num_parts);
        msgbus_msg_envelope_destroy(msg);
        msg = NULL;
        parts = NULL;
    }

    if(parts != NULL)
        msgbus_msg_envelope_serialize_destroy(parts, num_parts);

    return 0;

err:
    if(msg != NULL)
        msgbus_msg_envelope_destroy(msg);
    if(parts != NULL)
        msgbus_msg_envelope_serialize_destroy(parts, num_parts);
    if (sub_config != NULL) {
        config_destroy(sub_config);
    }
    if(g_sub_ctx != NULL)
        msgbus_recv_ctx_destroy(g_msgbus_ctx, g_sub_ctx);
    if(g_msgbus_ctx != NULL)
        msgbus_destroy(g_msgbus_ctx);
    if (g_sub_ch != NULL) {
        LOG_INFO_0("Freeing ConfigManager");
        delete g_sub_ch;
    }
    if (g_sub_ch_vis != NULL) {
        LOG_INFO_0("Freeing g_sub_ch_vis");
        delete g_sub_ch_vis;
    }
    return -1;
}
