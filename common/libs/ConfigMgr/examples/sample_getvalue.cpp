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
 * @brief ConfigManager getvalue usage example
 */

#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "eis/config_manager/config_mgr.hpp"

using namespace eis::config_manager;

void watch_config_example(char* key, char* value, void *user_data) {
    LOG_INFO("Callback triggered for change in App config, obtained user data: %d", (int *)(user_data));
}

void watch_interface_example(char* key, char* value, void *user_data) {
    LOG_INFO("Callback triggered for change in App interfaces, obtained user data: %d", (int *)(user_data));
}

int main() {

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

    // Set log level
    set_log_level(LOG_LVL_DEBUG);

    setenv("AppName","VideoIngestion", 1);
    ConfigMgr* config_mgr = new ConfigMgr();
    AppCfg* cfg = config_mgr->getAppConfig();

    LOG_INFO_0("========================================\n");
    config_value_t* app_config = cfg->getConfigValue("max_workers");
    if (app_config->type != CVT_INTEGER) {
        LOG_ERROR_0("Max_worker is not integer");
        exit(1);
    }
    int max_workers = app_config->body.integer;
    LOG_INFO("max_workers value is %d\n", max_workers);

    app_config = cfg->getConfigValue("ingestor");
    if (app_config->type != CVT_OBJECT) {
        LOG_ERROR_0("ingestor type is is not object");
        exit(1);
    }
    config_value_t* ingestor_type = config_value_object_get(app_config, "type");
    char* type = ingestor_type->body.string;
    LOG_INFO("ingestor_type value is %s\n", type);

    app_config = cfg->getConfigValue("udfs");
    if (app_config->type != CVT_ARRAY) {
        LOG_ERROR_0("udfs type is is not array");
        exit(1);
    }
    config_value_t* udf = config_value_array_get(app_config, 0);
    config_value_t* udf_type = config_value_object_get(udf, "type");
    type = udf_type->body.string;
    LOG_INFO("udf_type value is %s\n", type);

    bool ret = cfg->watchConfig(watch_config_example, (void*)1);
    if (!ret) {
        LOG_ERROR_0("Failed to register callback");
    }
    ret = cfg->watchInterface(watch_interface_example, (void*)2);
    if (!ret) {
        LOG_ERROR_0("Failed to register callback");
    }
    LOG_INFO_0("Watching on app config & app interface for 20 seconds");
    sleep(20);

    LOG_INFO_0("========================================\n");

}