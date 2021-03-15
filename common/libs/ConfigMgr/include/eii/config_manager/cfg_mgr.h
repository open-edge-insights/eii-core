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
 * @brief Base C implementation for ConfigManager
 */

#include <ctype.h>
#include "eii/config_manager/pub_cfg.h"
#include "eii/config_manager/sub_cfg.h"
#include "eii/config_manager/server_cfg.h"
#include "eii/config_manager/client_cfg.h"

#define PUBLISHERS "Publishers"
#define SUBSCRIBERS "Subscribers"
#define SERVERS "Servers"
#define CLIENTS "Clients"

#ifndef _EII_C_CONFIG_MGR_H
#define _EII_C_CONFIG_MGR_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {

    config_t* (*create_kv_store_config)();

    base_cfg_t* base_cfg;

    char* env_var;

} app_cfg_t;

/**
 * cfgmgr_get_publisher_by_name function to fetch a publisher config using its name
 * @param self - app_cfg_t object
 * @param name - name of config to be fetched
 * @return NULL for any errors occured or pub_cfg_t* on success
 */
pub_cfg_t* cfgmgr_get_publisher_by_name(app_cfg_t* self, const char* name);

/**
 * cfgmgr_get_publisher_by_index function to fetch a publisher config using its index
 * @param self - app_cfg_t object
 * @param index - index of config to be fetched
 * @return NULL for any errors occured or pub_cfg_t* on success
 */
pub_cfg_t* cfgmgr_get_publisher_by_index(app_cfg_t* app_cfg, int index);

/**
 * cfgmgr_get_subscriber_by_name function to fetch a subscriber config using its name
 * @param self - app_cfg_t object
 * @param name - name of config to be fetched
 * @return NULL for any errors occured or sub_cfg_t* on success
 */
sub_cfg_t* cfgmgr_get_subscriber_by_name(app_cfg_t* self, const char* name);

/**
 * cfgmgr_get_subscriber_by_index function to fetch a subscriber config using its index
 * @param self - app_cfg_t object
 * @param index - index of config to be fetched
 * @return NULL for any errors occured or sub_cfg_t* on success
 */
sub_cfg_t* cfgmgr_get_subscriber_by_index(app_cfg_t* app_cfg, int index);

/**
 * cfgmgr_get_server_by_name function to fetch a server config using its name
 * @param self - app_cfg_t object
 * @param name - name of config to be fetched
 * @return NULL for any errors occured or server_cfg_t* on success
 */
server_cfg_t* cfgmgr_get_server_by_name(app_cfg_t* self, const char* name);

/**
 * cfgmgr_get_server_by_index function to fetch a server config using its index
 * @param self - app_cfg_t object
 * @param index - index of config to be fetched
 * @return NULL for any errors occured or server_cfg_t* on success
 */
server_cfg_t* cfgmgr_get_server_by_index(app_cfg_t* app_cfg, int index);

/**
 * cfgmgr_get_client_by_name function to fetch a client config using its name
 * @param self - app_cfg_t object
 * @param name - name of config to be fetched
 * @return NULL for any errors occured or client_cfg_t* on success
 */
client_cfg_t* cfgmgr_get_client_by_name(app_cfg_t* self, const char* name);

/**
 * cfgmgr_get_client_by_index function to fetch a client config using its index
 * @param self - app_cfg_t object
 * @param index - index of config to be fetched
 * @return NULL for any errors occured or client_cfg_t* on success
 */
client_cfg_t* cfgmgr_get_client_by_index(app_cfg_t* app_cfg, int index);

/**
 * app_cfg_new function to create a new app_cfg_t object
 *  @return NULL for any errors occured or app_cfg_t* on successful
 */
app_cfg_t* app_cfg_new();

/**
 * Destroy app_cfg_t* object.
 *
 * @param app_cfg_config - configuration to destroy
 */
void app_cfg_config_destroy(app_cfg_t *app_cfg_config);

#ifdef __cplusplus
}
#endif

#endif