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

/**
 * @brief ConfigManager Client usage example
 */

#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "eii/config_manager/config_mgr.hpp"
#include "eii/msgbus/msgbus.h"
#include "eii/utils/logger.h"
#include "eii/utils/json_config.h"

#define SERVICE_NAME "echo_service"

<<<<<<< HEAD
=======

>>>>>>> master
using namespace eii::config_manager;

// Globals for cleaning up nicely
void* g_msgbus_ctx = NULL;
recv_ctx_t* g_service_ctx = NULL;
ConfigMgr* g_ch = NULL;

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
    if (g_ch != NULL) {
        LOG_INFO_0("Freeing ConfigManager")
        delete g_ch;
    }
    LOG_INFO_0("Done.");
}

void etcd_requirements_put() {

    bool config_set_result = false;

    config_t* config = json_config_new_from_buffer("{}");
    if (config == NULL) {
        LOG_ERROR_0("Error creating config object");
        _Exit(-1);
    }

    config_t* kv_store = json_config_new_from_buffer("{}");
    if (kv_store == NULL) {
        LOG_ERROR_0("Error creating kv_store object");
        _Exit(-1);
    }

    config_value_t* host = config_value_new_string("localhost");
    if (host == NULL) {
        LOG_ERROR_0("Error creating host object");
        _Exit(-1);
    }
    config_set_result = config_set(kv_store, "host", host);
    if (!config_set_result) {
        LOG_ERROR_0("Unable to set config value");
        _Exit(-1);
    }

    config_value_t* port = config_value_new_string("2379");
    if (port == NULL) {
        LOG_ERROR_0("Error creating port object");
        _Exit(-1);
    }
    config_set_result = config_set(kv_store, "port", port);
    if (!config_set_result) {
        LOG_ERROR_0("Unable to set config value");
        _Exit(-1);
    }

    config_value_t* cert_file = config_value_new_string(getenv("CONFIGMGR_CERT"));
    if (cert_file == NULL) {
        LOG_ERROR_0("Error creating cert_file object");
        _Exit(-1);
    }
    config_set_result = config_set(kv_store, "cert_file", cert_file);
    if (!config_set_result) {
        LOG_ERROR_0("Unable to set config value");
        _Exit(-1);
    }

    config_value_t* key_file = config_value_new_string(getenv("CONFIGMGR_KEY"));
    if (key_file == NULL) {
        LOG_ERROR_0("Error creating key_file object");
        _Exit(-1);
    }
    config_set_result = config_set(kv_store, "key_file", key_file);
    if (!config_set_result) {
        LOG_ERROR_0("Unable to set config value");
        _Exit(-1);
    }

    config_value_t* ca_file = config_value_new_string(getenv("CONFIGMGR_CACERT"));
    if (ca_file == NULL) {
        LOG_ERROR_0("Error creating ca_file object");
        _Exit(-1);
    }
    config_set_result = config_set(kv_store, "ca_file", ca_file);
    if (!config_set_result) {
        LOG_ERROR_0("Unable to set config value");
        _Exit(-1);
    }

    config_value_t* type = config_value_new_string("etcd");
    if (type == NULL) {
        LOG_ERROR_0("Error creating type object");
        _Exit(-1);
    }
    config_set_result = config_set(config, "type", type);
    if (!config_set_result) {
        LOG_ERROR_0("Unable to set config value");
        _Exit(-1);
    }

    config_value_t* kv_store_cvt = config_value_new_object(kv_store->cfg, get_config_value, NULL);
    if (kv_store_cvt == NULL) {
        LOG_ERROR_0("Error creating kv_store_cvt object");
        _Exit(-1);
    }

    config_set_result = config_set(config, "etcd_kv_store", kv_store_cvt);
    if (!config_set_result) {
        LOG_ERROR_0("Unable to set config value");
        _Exit(-1);
    }

    kv_store_client_t *kv_store_client = create_kv_client(config);
    if (kv_store_client == NULL) {
        LOG_ERROR_0("kv_store_client creation failed");
        _Exit(-1);
    }

    void *handle = kv_store_client->init(kv_store_client);

    int status = kv_store_client->put(handle, "/VideoAnalytics/interfaces", "{\
        \"Publishers\": [\
            {\
                \"AllowedClients\": [\
                    \"*\"\
                ],\
                \"EndPoint\": \"127.0.0.1:65013\",\
                \"Name\": \"default\",\
                \"Topics\": [\
                    \"camera1_stream_results\"\
                ],\
                \"Type\": \"zmq_tcp\"\
            }\
        ],\
        \"Subscribers\": [\
            {\
                \"EndPoint\": \"/EII/sockets\",\
                \"Name\": \"default\",\
                \"PublisherAppName\": \"VideoIngestion\",\
                \"Topics\": [\
                    \"camera1_stream\"\
                ],\
                \"Type\": \"zmq_ipc\",\
                \"zmq_recv_hwm\": 50\
            }\
        ],\
        \"Clients\": [\
                {\
                    \"Name\": \"default\",\
                    \"Type\": \"zmq_tcp\",\
                    \"EndPoint\": \"127.0.0.1:66013\",\
                    \"ServerAppName\": \"VideoIngestion\"\
                }\
            ]\
    }");
    if (status != 0) {
        LOG_ERROR_0("Etcd put failed for /VideoAnalytics/interfaces");
        _Exit(-1);
    }

    // Freeing used variables
    if (kv_store_client) {
        kv_client_free(kv_store_client);
    }
    if (config) {
        config_destroy(config);
    }
    if (kv_store) {
        free(kv_store);
    }
    if (host) {
        config_value_destroy(host);
    }
    if (port) {
        config_value_destroy(port);
    }
    if (cert_file) {
        config_value_destroy(cert_file);
    }
    if (key_file) {
        config_value_destroy(key_file);
    }
    if (ca_file) {
        config_value_destroy(ca_file);
    }
    if (type) {
        config_value_destroy(type);
    }
    if (kv_store_cvt) {
        config_value_destroy(kv_store_cvt);
    }
}

int main() {

    // Set log level
    set_log_level(LOG_LVL_DEBUG);

    // Setting up signal handlers
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    msg_envelope_serialized_part_t* parts = NULL;
    msg_envelope_t* msg = NULL;
    int num_parts = 0;
    msgbus_ret_t ret = MSG_SUCCESS;
    int num_of_clients;
    std::string ep;

    // Initailize request
    msg_envelope_elem_body_t* integer = msgbus_msg_envelope_new_integer(42);
    msg_envelope_elem_body_t* fp = msgbus_msg_envelope_new_floating(55.5);

    setenv("AppName","VideoAnalytics", 1);
    config_t* config = NULL;

    // Putting Client interface
    etcd_requirements_put();

    try {
        // get ConfigMgr object
        g_ch = new ConfigMgr();
    } catch (...) {
        LOG_ERROR_0("Exception occured");
        return -1;
    }

    // get the client object where client's interface 'Name' is 'default'
    // ClientCfg* client_ctx = g_ch->getClientByName("default");

    // get 0th client interface object
    ClientCfg* client_ctx = g_ch->getClientByIndex(0);

    char* name = NULL;

    // get the value of client interface of key 'Name'
    config_value_t* interface_value = client_ctx->getInterfaceValue("Name");
    if (interface_value == NULL || interface_value->type != CVT_STRING){
        LOG_ERROR_0("Failed to get expected interface value");
        goto err;
    }
    if (interface_value->type != CVT_STRING) {
        LOG_ERROR_0("interface_value type is not string");
        goto err;
    }

    // get the value from object interface_value in string format
    name = interface_value->body.string;
    
    LOG_INFO("interface value is %s", name);

    // get number of client interfaces
    num_of_clients = g_ch->getNumClients();
    LOG_DEBUG("Total number of clients : %d", num_of_clients);

    // get endpoint from client interfaces
    ep = client_ctx->getEndpoint();
    if(ep.empty()){
        LOG_ERROR_0("get endpoint failed");
        goto err;
    }
    LOG_INFO("Endpoint obtained : %s", ep.c_str());

    // get client msgbus config for application to communicate over EII message bus
    config = client_ctx->getMsgBusConfig();
    if(config == NULL) {
        LOG_ERROR_0("Failed to load JSON configuration");
        goto err;
    }

    g_msgbus_ctx = msgbus_initialize(config);
    if(g_msgbus_ctx == NULL) {
        LOG_ERROR_0("Failed to initialize message bus");
        goto err;
    }

    ret = msgbus_service_get(
            g_msgbus_ctx, name, NULL, &g_service_ctx);
    if(ret != MSG_SUCCESS) {
        LOG_ERROR("Failed to initialize service (errno: %d)", ret);
        goto err;
    }

    msg = msgbus_msg_envelope_new(CT_JSON);

    msgbus_msg_envelope_put(msg, "hello", integer);
    msgbus_msg_envelope_put(msg, "world", fp);

    // free interface_value
    config_value_destroy(interface_value);

    // delete client_ctx
    delete client_ctx;

    LOG_INFO_0("Sending request");
    ret = msgbus_request(g_msgbus_ctx, g_service_ctx, msg);
    if(ret != MSG_SUCCESS) {
        LOG_ERROR("Failed to send request (errno: %d)", ret);
        goto err;
    }

    msgbus_msg_envelope_destroy(msg);
    msg = NULL;
    
    LOG_INFO_0("Waiting for response");
    ret = msgbus_recv_wait(g_msgbus_ctx, g_service_ctx, &msg);
    if(ret != MSG_SUCCESS) {
        // Interrupt is an acceptable error
        if(ret != MSG_ERR_EINTR)
            LOG_ERROR("Failed to receive message (errno: %d)", ret);
        goto err;
    }

    num_parts = msgbus_msg_envelope_serialize(msg, &parts);

    if(num_parts <= 0) {
        LOG_ERROR_0("Failed to serialize message");
        goto err;
    }

    LOG_INFO("Received Response: %s", parts[0].bytes);

    msgbus_msg_envelope_serialize_destroy(parts, num_parts);
    msgbus_msg_envelope_destroy(msg);
    msg = NULL;
  
    msgbus_recv_ctx_destroy(g_msgbus_ctx, g_service_ctx);
    msgbus_destroy(g_msgbus_ctx);
 
    if (g_ch != NULL) {
        LOG_INFO_0("Freeing ConfigManager")
        delete g_ch;
    }
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
    if (g_ch != NULL) {
        LOG_INFO_0("Freeing ConfigManager")
        delete g_ch;
    }
    return -1;
}
