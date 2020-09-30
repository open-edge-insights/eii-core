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
 * @brief Util functions for ConfigMgr
 */


#include "eis/config_manager/base_cfg.h"

#define SOCKET_FILE "socket_file"
#define ENDPOINT "EndPoint"
#define TOPICS "Topics"
#define MAX_CONFIG_KEY_LENGTH 250


#ifndef _EIS_C_UTIL_CFG_H
#define _EIS_C_UTIL_CFG_H

#ifdef __cplusplus
extern "C" {
#endif


/**
 * get_ipc_config function to creates json structure for ipc mode messagebus config
 * @param c_json - cJSON object for which ipc config to be added
 * @param config - Config from which values are extracted
 * @param end_point - endpoint of the application
 * @return cJSON object which consists of IPC messagebus config
 */
cJSON* get_ipc_config(cJSON* c_json, config_value_t* config, const char* end_point);

/**
 * cvt_obj_str_to_char function converts cvt object to char* provided for ipc
 * @param cvt : config_value_t* object
 * @return char* value of the cvt
 */
char* cvt_obj_str_to_char(config_value_t* cvt);

#ifdef __cplusplus
}
#endif

#endif