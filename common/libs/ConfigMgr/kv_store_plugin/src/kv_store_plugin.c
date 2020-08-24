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
// FROM,OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.


/**
 * @file
 * @brief KV Store Plugin implementation
 */

#include "kv_store_plugin.h"
#include <eis/utils/config.h>
#include <stdlib.h>

kv_store_client_t* create_kv_client(config_t* config){
    kv_store_client_t* kv_store_client = NULL;
    config_value_t* value = config->get_config_value(config->cfg, "type");
        
    if(value == NULL) {
        // LOG_ERROR_0("Config missing 'type' key");
        goto err;
    }

    if(value->type != CVT_STRING) {
        // LOG_ERROR_0("Config 'type' value MUST be a string");
        goto err;
    }

    int ind_etcd;
    ind_etcd = strcmp(value->body.string, KV_ETCD);

    if(ind_etcd == 0) {
        kv_store_client = create_etcd_client(config);
        if(kv_store_client == NULL)
            goto err;
     } else {
        // LOG_ERROR("Unknown protocol type: %s", value->body.string);
        goto err;
    }

    return kv_store_client;
err:
    if(value != NULL)
        free(value);
    return NULL;
}

void kv_client_free(kv_store_client_t* kv_store_client) {
    if(kv_store_client != NULL){
        kv_store_client->deinit(kv_store_client->handler);
        if(kv_store_client->kv_store_config != NULL) {
             free(kv_store_client->kv_store_config);
        }
        free(kv_store_client);
    }
}