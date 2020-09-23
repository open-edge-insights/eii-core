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
 * @file
 * @brief Utility for ConfigManager
 */

#include <stdio.h>
#include <stdlib.h>
#include <cjson/cJSON.h>
#include <safe_lib.h>
#include <eis/utils/logger.h>
#include <eis/utils/string.h>
#include <stdbool.h>
#include <ctype.h>
#include "eis/utils/json_config.h"
#include "eis/config_manager/kv_store_plugin.h"

#ifndef _EIS_C_BASE_CFG_H
#define _EIS_C_BASE_CFG_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {

    config_t* m_app_config;

    config_t* m_app_interface;

    config_t* m_data_store;

    char* app_name;

    int dev_mode;

    kv_store_client_t* m_kv_store_handle;

    config_value_t* msgbus_config;

} base_cfg_t;


/**
 * get_endpoint_base function to fetch endpoint
 * @param base_cfg - base_cfg_t object
 *  @return NULL for any errors occured or config_value_t* on success
 */
config_value_t* get_endpoint_base(base_cfg_t* base_cfg);

/**
 * get_topics_base function to fetch topics
 * @param base_cfg - base_cfg_t object
 *  @return NULL for any errors occured or config_value_t* on success
 */
config_value_t* get_topics_base(base_cfg_t* base_cfg);

/**
 * get_allowed_clients_base function to get allowed clients
 * @param base_cfg - base_cfg_t object
 *  @return NULL for any errors occured or config_value_t* on success
 */
config_value_t* get_allowed_clients_base(base_cfg_t* base_cfg);

/**
 * set_topics_base function to set topics
 * @param topics_list - list of topics to be set
 * @param len - total number of topics
 * @param type - Publisher/Subscriber
 * @param base_cfg - base_cfg_t object
 *  @return -1 for any errors occured or 0 on success
 */
int set_topics_base(char** topics_list, int len, const char* type, base_cfg_t* base_cfg);

/**
 * configt_to_char function to convert config_t to char*
 * @param config - config_t object
 *  @return NULL for any errors occured or char* on success
 */
char* configt_to_char(config_t* config);

/**
 * get_app_config function to return app config
 * @param base_cfg - base_cfg_t object
 *  @return NULL for any errors occured or config_t* on success
 */
config_t* get_app_config(base_cfg_t* base_cfg);

/**
 * get_app_interface function to return app interface
 * @param base_cfg - base_cfg_t object
 *  @return NULL for any errors occured or config_t* on success
 */
config_t* get_app_interface(base_cfg_t* base_cfg);

/**
 * get_app_config_value function to fetch config value
 * @param base_cfg - base_cfg_t object
 * @param key - value of key to be fetched
 *  @return NULL for any errors occured or config_value_t* on success
 */
config_value_t* get_app_config_value(base_cfg_t* base_cfg, char* key);

/**
 * get_app_interface_value function to fetch interface value
 * @param base_cfg - base_cfg_t object
 * @param key - value of key to be fetched
 *  @return NULL for any errors occured or config_value_t* on success
 */
config_value_t* get_app_interface_value(base_cfg_t* base_cfg, char* key);

/**
 * base_cfg_new function to creates a new config manager client
 *  @return NULL for any errors occured or base_cfg_t* on success
 */
base_cfg_t* base_cfg_new();

/**
 * Destroy base_cfg_t* object.
 *
 * @param base_cfg_config - configuration to destroy
 */
void base_cfg_config_destroy(base_cfg_t *base_cfg_config);

#ifdef __cplusplus
}
#endif

#endif