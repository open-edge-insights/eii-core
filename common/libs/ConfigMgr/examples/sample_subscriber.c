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
#include "eis/config_manager/cfgmgr.h"

int main(int argc, char** argv) {

    // Set log level
    set_log_level(LOG_LVL_DEBUG);

    // Fetching Subscriber config from
    // VideoAnalytics interface
    setenv("AppName", "VideoAnalytics", 1);

    // Initializing ConfigMgr
    cfgmgr_ctx_t* cfg_mgr = cfgmgr_initialize();

    // Testing cfgmgr_is_dev_mode API
    bool dev_mode = cfgmgr_is_dev_mode(cfg_mgr);

    // Testing cfgmgr_get_appname API
    config_value_t* app_name = cfgmgr_get_appname(cfg_mgr);
    LOG_INFO("AppName is %s\n", app_name->body.string);

    // Testing cfgmgr_get_num_subscribers API
    int num_subscribers = cfgmgr_get_num_subscribers(cfg_mgr);
    LOG_INFO("Number of subscribers %d\n", num_subscribers);

    // Fetching cfgmgr_interface_t for a subscribers example
    cfgmgr_interface_t* subscriber_interface = cfgmgr_get_subscriber_by_index(cfg_mgr, 0);
    // cfgmgr_interface_t* subscriber_interface = cfgmgr_get_subscriber_by_name(cfg_mgr, "default");

    // Fetching msgbus config for subscriber example
    // using the cfgmgr_interface_t obtained
    config_t* sub_config = cfgmgr_get_msgbus_config(subscriber_interface);
    char* sub_config_char = configt_to_char(sub_config);
    LOG_INFO("Msgbus config for subscriber is %s\n", sub_config_char);

    // Fetching EndPoint for subscriber example
    // using the cfgmgr_interface_t obtained
    config_value_t* ep = cfgmgr_get_endpoint(subscriber_interface);
    LOG_INFO("EndPoint for subscriber is %s\n", ep->body.string);

    // Fetching Topics for publisher example
    // using the cfgmgr_interface_t obtained
    config_value_t* topics = cfgmgr_get_topics(subscriber_interface);
    config_value_t* topic_value;
    size_t arr_len = config_value_array_len(topics);
    if (arr_len == 0) {
        LOG_ERROR_0("Empty array is not supported, atleast one value should be given.");
    }
    for (size_t i = 0; i < arr_len; i++) {
        topic_value = config_value_array_get(topics, i);
        if (topic_value == NULL) {
            LOG_ERROR_0("topic_value initialization failed");
            config_value_destroy(topics);
        }
        LOG_INFO("topic value is %s\n", topic_value->body.string);
        // Destroying topic_value
        config_value_destroy(topic_value);
    }

    // Testing cfgmgr_get_app_config API
    config_t* app_config = cfgmgr_get_app_config(cfg_mgr);
    char* app_config_char = configt_to_char(app_config);
    LOG_INFO("App config is %s\n", app_config_char);

    // Testing cfgmgr_get_app_interface API
    config_t* app_interface = cfgmgr_get_app_interface(cfg_mgr);
    char* app_interface_char = configt_to_char(app_interface);
    LOG_INFO("App interface is %s\n", app_interface_char);

    // Testing cfgmgr_get_app_config_value API
    config_value_t* app_config_value = cfgmgr_get_app_config_value(cfg_mgr, "encoding");
    char* app_config_value_char = cvt_to_char(app_config_value);
    LOG_INFO("App config value of encoding is %s\n", app_config_value_char);

    // Testing cfgmgr_get_app_interface_value API
    config_value_t* app_interface_value = cfgmgr_get_app_interface_value(cfg_mgr, "Subscribers");
    char* app_interface_value_char = cvt_to_char(app_interface_value);
    LOG_INFO("App interface value of Subscribers is %s\n", app_interface_value_char);

    // Freeing all variables
    config_value_destroy(app_name);
    cfgmgr_interface_destroy(subscriber_interface);
    config_value_destroy(ep);
    config_value_destroy(app_interface_value);
    config_value_destroy(app_config_value);
    config_value_destroy(topics);
    config_destroy(sub_config);
    config_destroy(app_interface);
    config_destroy(app_config);
    free(sub_config_char);
    free(app_config_char);
    free(app_interface_char);
    free(app_config_value_char);
    free(app_interface_value_char);

    return 0;
}
