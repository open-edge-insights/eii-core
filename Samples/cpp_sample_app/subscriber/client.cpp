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
 * @brief Message bus echo service client example
 */

#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "eis/msgbus/msgbus.h"
#include "eis/utils/logger.h"
#include "eis/utils/json_config.h"
#include <eis/config_manager/env_config.h>
#include "client.h"
#include <get_config_mgr.h>
#include <usage.h>

client::client(std::atomic<bool> *loop)
{this->loop = loop;}

client::~client()
{
    if(msg != NULL)
        msgbus_msg_envelope_destroy(msg);
    if(parts != NULL)
        msgbus_msg_envelope_serialize_destroy(parts, num_parts);
    if(g_service_ctx != NULL)
        msgbus_recv_ctx_destroy(g_msgbus_ctx_client, g_service_ctx);
    if(g_msgbus_ctx_client != NULL)
        msgbus_destroy(g_msgbus_ctx_client);
}

bool client::init(char *service_name)
{
    g_env_config_client = env_config_new();
    g_config_mgr = get_config_mgr();
    char* TOPICS[] = {m_app_name};
    size_t num_of_topics_pub = g_env_config_client->get_topics_count(TOPICS);
    config_t* config_client = g_env_config_client->get_messagebus_config(g_config_mgr, TOPICS,num_of_topics_pub ,"client");
    g_msgbus_ctx_client = msgbus_initialize(config_client);
    if(g_msgbus_ctx_client == NULL) {
        LOG_ERROR_0("Failed to initialize message bus");
        goto err;
    }

    msgbus_ret_t ret ;
    ret = msgbus_service_get(g_msgbus_ctx_client, m_app_name, NULL, &g_service_ctx);
    if(ret != MSG_SUCCESS) {
        LOG_ERROR("Failed to initialize service (errno: %d)", ret);
        goto err;
    }

    return true;
err:
    if(g_service_ctx != NULL)
        msgbus_recv_ctx_destroy(g_msgbus_ctx_client, g_service_ctx);
    if(g_msgbus_ctx_client != NULL)
        msgbus_destroy(g_msgbus_ctx_client);

    return false;
}

void* client::start(void *arg)
{
    int ret_val = -1;
    client *obj = (client*)arg ;

    if( ( ret_val = obj -> client_service() ) != 0 ){
        LOG_ERROR("client service failed.(errno: %d)", ret_val);
    }
    delete obj;
}

int client::client_service() {
    
    int ret=0;
    while(*loop) {
        // Initailize request
        msg_envelope_elem_body_t* integer ;
        integer = msgbus_msg_envelope_new_integer(rand() % 100);
        msg_envelope_elem_body_t* fp ;
        fp = msgbus_msg_envelope_new_floating(float(rand())/float(RAND_MAX));

        msg = msgbus_msg_envelope_new(CT_JSON);

        msgbus_msg_envelope_put(msg, "hello", integer);
        msgbus_msg_envelope_put(msg, "world", fp);

        LOG_INFO_0("Sending request");
        num_parts = msgbus_msg_envelope_serialize(msg, &parts);
        if(num_parts <= 0) {
            LOG_ERROR_0("Failed to serialize message");
            goto err;
        }

        LOG_INFO("Message sent from client: %s", parts[0].bytes);
        ret = msgbus_request(g_msgbus_ctx_client, g_service_ctx, msg);
        if(ret != MSG_SUCCESS) {
            LOG_ERROR("Failed to send request (errno: %d)", ret);
            goto err;
        }
        sleep(1);
        msgbus_msg_envelope_destroy(msg);
        msg = NULL;

        LOG_INFO_0("Waiting for response");
        ret = msgbus_recv_wait(g_msgbus_ctx_client, g_service_ctx, &msg);
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

        LOG_INFO("Received Response back at Client: %s", parts[0].bytes);
        msgbus_msg_envelope_serialize_destroy(parts, num_parts);
        msgbus_msg_envelope_destroy(msg);
        msg = NULL;
    }
    msgbus_recv_ctx_destroy(g_msgbus_ctx_client, g_service_ctx);
    msgbus_destroy(g_msgbus_ctx_client);

    return 0;

err:
    if(msg != NULL)
        msgbus_msg_envelope_destroy(msg);
    if(parts != NULL)
        msgbus_msg_envelope_serialize_destroy(parts, num_parts);
    if(g_service_ctx != NULL)
        msgbus_recv_ctx_destroy(g_msgbus_ctx_client, g_service_ctx);
    if(g_msgbus_ctx_client != NULL)
        msgbus_destroy(g_msgbus_ctx_client);
    return -1;
}
