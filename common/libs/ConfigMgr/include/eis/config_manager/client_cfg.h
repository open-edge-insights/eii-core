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
 * @brief Client config implmentaion
 */


#include "eis/config_manager/base_cfg.h"
#define TYPE "Type"
#define NAME "Name"
#define ENDPOINT "EndPoint"
#define ZMQ_RECV_HWM "zmq_recv_hwm"
#define SERVER_APPNAME "ServerAppName"
#define PUBLIC_KEYS "/Publickeys/"
#define PRIVATE_KEY "/private_key"

#ifndef _EIS_C_CLI_CFG_H
#define _EIS_C_CLI_CFG_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {

    config_t* (*cfgmgr_get_msgbus_config_client)(base_cfg_t* base_cfg, void* cli_conf);

    config_value_t* (*cfgmgr_get_interface_value_client)(void* cli_conf, const char* key);

    config_value_t* (*cfgmgr_get_endpoint_client)(void* cli_conf);

    config_value_t* client_config;

} client_cfg_t;

/**
 * client_cfg_new function to creates a new client_cfg_t
 *  @return NULL for any errors occured or client_cfg_t* on successful
 */
client_cfg_t* client_cfg_new();

/**
 * Destroy client_cfg_t* object.
 *
 * @param cli_cfg_config - configuration to destroy
 */
void client_cfg_config_destroy(client_cfg_t *cli_cfg_config);

#ifdef __cplusplus
}
#endif

#endif