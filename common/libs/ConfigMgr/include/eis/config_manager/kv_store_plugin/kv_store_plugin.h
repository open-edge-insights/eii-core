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
 * @brief KV_Store_Plugin abstraction
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <eis/utils/config.h>
#include <eis/utils/logger.h>

#ifndef EIS_KV_STORE_CLIENT_H
#define EIS_KV_STORE_CLIENT_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Format for the user callback to notify the user when any update occurs on a key
 * when watch functions are being called for the key
 * @param key           key is being updated
 * @param value         updated value
 * @param cb_user_data  user data passed
 */
typedef void (*kv_store_watch_callback_t)(const char *key, config_t* value, void *cb_user_data);


/*
 * Representation of kv_store_client object
 */
typedef struct {
        // kv_store_config to hold kv_store details
        void *kv_store_config;

        // handler to connect to kv_store
        void *handler;

        // function pointer to assign to initialize respective kv_store
        void* (*init)(void* kv_store_config);

        // function pointer to assign to get value from kv_store
        char* (*get) (void* handle, char *key);

        // function pointer to assign to get all value of 
        // a prefixed key from kv_store_client
        char* (*get_prefix) (void* handle, char *key);

        // function poiner to assign to store value of a particular key into kv_store
        int (*put) (void* handle, char *key, char *value);

        // function pointer to watch for any changes of a key, registers user_callback,
        // notify user if any change on key occured
        void (*watch) (void* handle, char *key, kv_store_watch_callback_t cb, void* user_data);

        // function pointer to watch for any changes of a key prefix, registers user_callback,
        // notify user if any change on key occured
        void (*watch_prefix) (void* handle, char *key, kv_store_watch_callback_t cb, void* user_data);

        // function pointer to delete respective kv_store
        void (*deinit)(void* handle);
} kv_store_client_t;

/**
 * Create KV_Store based on the config passed. 
 * Ex: if config's `type` is `etcd, it will create etcd_client instance
 * @param config Configuration object pointer
 * @return  @c kv_store_client_t
 */
kv_store_client_t* create_kv_client(config_t* config);

/**
 * Free kv_store_client_t members and kv_store_client_t object
 */
void kv_client_free(kv_store_client_t* kv_store_client);

#ifdef __cplusplus
}
#endif

#endif