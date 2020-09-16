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
    int num_parts = 0;

    // In a dockerized environment,
    // these variables are set in environment
    setenv("DEV_MODE", "FALSE", 1);
    // Replace 2nd parameter with path to certs
    setenv("CONFIGMGR_CERT", "", 1);
    setenv("CONFIGMGR_KEY", "", 1);
    setenv("CONFIGMGR_CACERT", "", 1);

    // Uncomment below lines to test DEV mode
    // setenv("DEV_MODE", "TRUE", 1);
    // setenv("CONFIGMGR_CERT", "", 1);
    // setenv("CONFIGMGR_KEY", "", 1);
    // setenv("CONFIGMGR_CACERT", "", 1);

    // Fetching Subscriber config from
    // VideoAnalytics interface
    setenv("AppName","VideoAnalytics", 1);
    ConfigMgr* sub_ch = new ConfigMgr();

    SubscriberCfg* sub_ctx = sub_ch->getSubscriberByName("VideoData");
    config_t* sub_config = sub_ctx->getMsgBusConfig();

    // Testing getEndpoint API
    std::string ep = sub_ctx->getEndpoint();
    LOG_INFO("Endpoint obtained : %s", ep.c_str());

    // Testing getTopics API
    std::vector<std::string> topics = sub_ctx->getTopics();
    for(int i = 0; i < topics.size(); i++) {
        LOG_INFO("Sub Topics : %s", topics[i].c_str());
    }

    // Testing setTopics API
    std::vector<std::string> newTopicsList;
    newTopicsList.push_back("camera7_stream");
    newTopicsList.push_back("camera8_stream");
    bool topicsSet = sub_ctx->setTopics(newTopicsList);

    // Testing TCP PROD mode
    setenv("AppName","Visualizer", 1);
    ConfigMgr* sub_ch_vis = new ConfigMgr();

    SubscriberCfg* sub_ctx_vis = sub_ch_vis->getSubscriberByName("Cam2_Results");
    config_t* sub_config_vis = sub_ctx_vis->getMsgBusConfig();
    topics = sub_ctx_vis->getTopics();

    // Initializing Subscriber using sub_config obtained
    // from new ConfigManager APIs
    g_msgbus_ctx = msgbus_initialize(sub_config_vis);
    // Uncomment below line to test IPC mode
    // g_msgbus_ctx = msgbus_initialize(sub_config_vis);
    if(g_msgbus_ctx == NULL) {
        LOG_ERROR_0("Failed to initialize message bus");
        goto err;
    }

    msgbus_ret_t ret;

    ret = msgbus_subscriber_new(g_msgbus_ctx, topics[0].c_str(), NULL, &g_sub_ctx);

    if(ret != MSG_SUCCESS) {
        LOG_ERROR("Failed to initialize subscriber (errno: %d)", ret);
        goto err;
    }

    LOG_INFO_0("Running...");
    while(g_sub_ctx != NULL) {
        ret = msgbus_recv_wait(g_msgbus_ctx, g_sub_ctx, &msg);
        if(ret != MSG_SUCCESS) {
            // Interrupt is an acceptable error
            if(ret == MSG_ERR_EINTR)
                break;
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
    if(g_sub_ctx != NULL)
        msgbus_recv_ctx_destroy(g_msgbus_ctx, g_sub_ctx);
    if(g_msgbus_ctx != NULL)
        msgbus_destroy(g_msgbus_ctx);
    return -1;
}
