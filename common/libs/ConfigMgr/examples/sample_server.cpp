// // Copyright (c) 2020 Intel Corporation.
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

/**
 * @brief ConfigManager Server usage example
 */

#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "eis/config_manager/config_mgr.hpp"
#include "eis/msgbus/msgbus.h"
#include "eis/utils/logger.h"
#include "eis/utils/json_config.h"

#define SERVICE_NAME "echo_service"

using namespace eis::config_manager;

// Globals for cleaning up nicely
void* g_msgbus_ctx = NULL;
recv_ctx_t* g_service_ctx = NULL;
ConfigMgr* g_config_mgr = NULL;
ServerCfg* g_server_ctx = NULL;

/**
 * Signal handler
 */
void signal_handler(int signo) {
    LOG_INFO_0("Cleaning up");
    if(g_service_ctx != NULL) {
        LOG_INFO_0("Freeing publisher");
        msgbus_recv_ctx_destroy(g_msgbus_ctx, g_service_ctx);
        g_service_ctx = NULL;
    }
    if(g_msgbus_ctx != NULL) {
        LOG_INFO_0("Freeing message bus context");
        msgbus_destroy(g_msgbus_ctx);
        g_msgbus_ctx = NULL;
    }
    if (g_server_ctx != NULL) {
        LOG_INFO_0("Freeing server ctx");
        delete g_server_ctx;
    }
    if (g_config_mgr != NULL) {
        LOG_INFO_0("Freeing config Mgr object")
        delete g_config_mgr;
    }
    LOG_INFO_0("Done.");
}

int main() {
    // Set log level
    set_log_level(LOG_LVL_DEBUG);

    // Setting up signal handlers
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    msgbus_ret_t ret = MSG_SUCCESS;

    msg_envelope_serialized_part_t* parts = NULL;
    msg_envelope_t* msg = NULL;
    int num_parts = 0;

    setenv("AppName","VideoIngestion", 1);
    config_t* config;
    std::vector<std::string> clients;
    config_value_t* interface_value;
    char* name = NULL;

    try {
        g_config_mgr = new ConfigMgr();
    } catch (...) {
        LOG_ERROR_0("Exception occured");
        return -1;
    }

    int num_of_servers = g_config_mgr->getNumServers();
    LOG_DEBUG("Total number of servers : %d", num_of_servers);

    g_server_ctx = g_config_mgr->getServerByIndex(0);
    //g_server_ctx = g_config_mgr->getServerByName("default");

    
    std::string ep = g_server_ctx->getEndpoint();
    if (ep.empty()){
        LOG_ERROR_0(" Get endpoint failed");
        exit(-1);
    }
    LOG_INFO("Endpoint obtained : %s", ep.c_str());

    //Testing getAllowedClients API
    clients = g_server_ctx->getAllowedClients();
    if (clients.empty()){
        LOG_ERROR_0(" Get allowed clients failed");
        exit(-1);
    }

    for (int i = 0; i < clients.size(); i++) {
        LOG_INFO("Allowed clients : %s", clients[i].c_str());
    }

    
    interface_value = g_server_ctx->getInterfaceValue("Name");
    if (interface_value == NULL || interface_value->type != CVT_STRING){
        LOG_ERROR_0("Failed to get expected interface value");
        exit(-1);
    }
    name = interface_value->body.string;
    LOG_INFO("interface value is %s", name);

    config = g_server_ctx->getMsgBusConfig();
    if(config == NULL){
        LOG_ERROR_0(" Get message bus config failed");
        exit(-1);
    }

    g_msgbus_ctx = msgbus_initialize(config);
    if(g_msgbus_ctx == NULL) {
        LOG_ERROR_0("Failed to initialize message bus");
        goto err;
    }

    ret = msgbus_service_new(
            g_msgbus_ctx, name, NULL, &g_service_ctx);
    if(ret != MSG_SUCCESS) {
        LOG_ERROR("Failed to initialize service (errno: %d)", ret);
        goto err;
    }

    LOG_INFO_0("Running...");
    if (interface_value != NULL) {
        config_value_destroy(interface_value);
    }

    while(g_service_ctx != NULL) {
        ret = msgbus_recv_wait(g_msgbus_ctx, g_service_ctx, &msg);
        if(ret != MSG_SUCCESS) {
            // Interrupt is an acceptable error
            if(ret == MSG_ERR_EINTR)
                break;
            LOG_ERROR("Failed to receive message (errno: %d)", ret);
            goto err;
        }

        num_parts = msgbus_msg_envelope_serialize(msg, &parts);
        if(num_parts <= 0) {
            LOG_ERROR_0("Failed to serialize message");
            goto err;
        }

        LOG_INFO("Received Request: %s", parts[0].bytes);

        // Responding
        ret = msgbus_response(g_msgbus_ctx, g_service_ctx, msg);
        if(ret != MSG_SUCCESS) {
            LOG_ERROR("Failed to send response (errno: %d)", ret);
            goto err;
        }

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
    if(g_service_ctx != NULL)
        msgbus_recv_ctx_destroy(g_msgbus_ctx, g_service_ctx);
    if(g_msgbus_ctx != NULL)
        msgbus_destroy(g_msgbus_ctx);
    return -1;
}
