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
 * @brief Config Manager implementation
 */

#include "eis/config_manager/config_manager.h"
#include "eis/config_manager/go_config_manager.h"
#include <ctype.h>

char supported_storage_types[][5] = {"etcd"};
static char* get_config(char *key){
    return getConfig(key);
}

static void register_watch_key(char *key, callback_fcn user_callback){
    registerWatchKey(key, user_callback);
}

static void register_watch_dir(char *key, callback_fcn user_callback){
    registerWatchDir(key, user_callback);
}

static void free_config(void *config){
    if(config) {
        free(config);
    }
}

config_mgr_t* config_mgr_new(config_mgr_config_t *config_mgr_config){
    for(int i = 0; i < sizeof(supported_storage_types) / sizeof(supported_storage_types[0]); i++){
        if (!strcmp(config_mgr_config->storage_type, supported_storage_types[i])){
            break;
        }
        return NULL;
    }
    char* status = initialize(config_mgr_config->storage_type,
                                    config_mgr_config->cert_file,
                                    config_mgr_config->key_file,
                                    config_mgr_config->ca_cert);
    if(!strcmp(status, "-1")) {
        return NULL;
    }
    config_mgr_t *config_mgr = (config_mgr_t *)malloc(sizeof(config_mgr_t));
    config_mgr->get_config = get_config;
    config_mgr->register_watch_key = register_watch_key;
    config_mgr->register_watch_dir = register_watch_dir;
    config_mgr->free_config = free_config;
    return config_mgr;
}
