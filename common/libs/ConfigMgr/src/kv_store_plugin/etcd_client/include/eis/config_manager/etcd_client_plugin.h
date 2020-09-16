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

#include <eis/utils/logger.h>
#include <eis/config_manager/kv_store_plugin.h>

#define ETCD_KV_STORE   "etcd_kv_store"

/**
 * Create etcd client to store key-value for the kv_store_plugin
 *
 * @param config - Configuration
 * @return kv_store_client instance, or NULL
 */

typedef struct {
    char *hostname;
    char *port;
    char *cert_file;
    char *key_file;
    char *ca_file;
} etcd_config_t;

typedef void (*callback)(char *key, char *value, void* cb_user_data);

kv_store_client_t* create_etcd_client(config_t* config);
