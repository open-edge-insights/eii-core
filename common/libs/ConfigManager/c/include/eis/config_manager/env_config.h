// Copyright (c) 2019 Intel Corporation.
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
// FROM,OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.


/**
 * @file
 * @brief Env Config interface
 */

#ifndef _EIS_ENV_CONFIG_H
#define _EIS_ENV_CONFIG_H

#include <cjson/cJSON.h>
#include <eis/config_manager/config_manager.h>
#include "eis/utils/json_config.h"
#include "eis/utils/logger.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {

    /**
     * get_topics_from_env function gives the topics with respect to publisher/subscriber.
     *
     * @param topic_type       - Topic type. Either pub or sub
     * @return char**          - topics returned from env config based on topic type
     */

    char** (*get_topics_from_env)(const char* topic_type);

    /**
     * get_messagebus_config function gives the configuration that needs in connecting to EIS messagebus
     *
     * @param configmgr       - Config Manager object
     * @param topic           - Topic for which msg bus config needs to be constructed
     * @param topic_type      - TopicType for which msg bus config needs to be constructed
     * @return config_t*      - JSON msg bus config of type config_t
     */
    config_t* (*get_messagebus_config)(const config_mgr_t* configmgr, const char topic[], const char* topic_type);

    /**
     * trim function removes white spaces around the string value
     *
     * @param str_value       - str_value will be trimmed of from white spaces
     */
    void (*trim)(char* str_value);

} env_config_t;

void env_config_destroy(env_config_t* env_config);
env_config_t* env_config_new();

#ifdef __cplusplus
} //end extern "C"
#endif

#endif
