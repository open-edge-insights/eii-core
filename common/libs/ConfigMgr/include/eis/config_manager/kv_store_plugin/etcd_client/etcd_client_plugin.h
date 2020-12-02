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
 * @brief Interface between kv_store_plugin and etcd_client
 */

#include <eis/utils/logger.h>
#include <eis/config_manager/kv_store_plugin/kv_store_plugin.h>

#define ETCD_KV_STORE   "etcd_kv_store"

/**
 * etcd_config object
 */
typedef struct {
    char *hostname;
    char *port;
    char *cert_file;
    char *key_file;
    char *ca_file;
} etcd_config_t;

/**
 * Extract config values, create kv_store_client object based on config and
 * fill kv_store_client's function pointers and kv_store_config which internally
 * points to @c etcd_config_t
 * This function would be called by kv_store_plugin's create_kv_client() internally
 * @param config - Configuration object
 * @return kv_store_client instance, or NULL
 */
kv_store_client_t* create_etcd_client(config_t* config);

/**
 * Free etcd_config_t and resources held by kv_store_client object
 @param kv_store_client - @c kv_store_client_t object
 */
void etcd_values_destroy(kv_store_client_t* kv_store_client);