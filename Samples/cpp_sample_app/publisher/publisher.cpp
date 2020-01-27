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
 * @brief Message bus publisher example
 */

#include <signal.h>
#include <unistd.h>
#include <eis/config_manager/env_config.h>
#include "eis/msgbus/msgbus.h"
#include "publisher.h"
#include <get_config_mgr.h>

char* TOPIC[] = {"publish_test"};

using namespace eis::utils;


publisher::publisher(std::atomic<bool> *loop)
{this->loop = loop;}

publisher::~publisher()
{
    if(g_pub_ctx != NULL)
        msgbus_publisher_destroy(g_msgbus_ctx, g_pub_ctx);
    if(g_msgbus_ctx != NULL)
        msgbus_destroy(g_msgbus_ctx);
}

bool publisher::init(char *topic_name)
{
    g_env_config_client = env_config_new();
    g_config_mgr = get_config_mgr();
    size_t num_of_topics_pub = g_env_config_client->get_topics_count(TOPIC);
    config_t* pub_config = g_env_config_client->get_messagebus_config(g_config_mgr,TOPIC,num_of_topics_pub,"pub");

    g_msgbus_ctx = msgbus_initialize(pub_config);
    if(g_msgbus_ctx == NULL) {
        LOG_ERROR_0("Failed to initialize message bus");
        goto err;
    }

    msgbus_ret_t ret;
    ret = msgbus_publisher_new(g_msgbus_ctx, topic_name, &g_pub_ctx);

    if(ret != MSG_SUCCESS) {
        LOG_ERROR("Failed to initialize publisher (errno: %d)", ret);
        goto err;
    }

    return true;

err:
    if(g_pub_ctx != NULL)
        msgbus_publisher_destroy(g_msgbus_ctx, g_pub_ctx);
    if(g_msgbus_ctx != NULL)
        msgbus_destroy(g_msgbus_ctx);

    return false;
}

void* publisher::start(void *arg)
{
    int ret_val = -1;
    publisher *obj = (publisher*)arg ;

    if( ( ret_val = obj->publish() ) != 0 ){
        LOG_ERROR("publish failed.(errno: %d)", ret_val);
    }

    delete obj;
}

int publisher::publish(){

   int ret = 0;
   LOG_INFO_0("Running...");
   while(*loop){
        LOG_INFO_0("Publishing message");
        // Initialize message to be published
        msg_envelope_elem_body_t* integer ;
        integer = msgbus_msg_envelope_new_integer(rand() % 100);
        msg_envelope_elem_body_t* fp ;
        fp = msgbus_msg_envelope_new_floating(float(rand())/float(RAND_MAX));

        g_msg = msgbus_msg_envelope_new(CT_JSON);

        msgbus_msg_envelope_put(g_msg, "hello", integer);
        msgbus_msg_envelope_put(g_msg, "world", fp);
        num_parts = msgbus_msg_envelope_serialize(g_msg, &parts);
        if(num_parts <= 0) {
            LOG_ERROR_0("Failed to serialize message");
            goto err;
        }

        LOG_INFO("Message Published at Publisher: %s", parts[0].bytes)
        ret = msgbus_publisher_publish(g_msgbus_ctx, g_pub_ctx, g_msg);
        if(ret != MSG_SUCCESS) {
            LOG_ERROR("Failed to publish message (errno: %d)", ret);
            goto err;
        }
        sleep(1);
     }

    return 0;

err:

    if(g_pub_ctx != NULL)
        msgbus_publisher_destroy(g_msgbus_ctx, g_pub_ctx);
    if(g_msgbus_ctx != NULL)
        msgbus_destroy(g_msgbus_ctx);
    return -1;
}
