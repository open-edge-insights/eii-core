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

#include "etcd_client_plugin.h"

#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#ifdef __cplusplus
extern "C" {
#endif

// void db_client_free(db_client_t *db_client);

// db_client config
typedef struct {
        void *db_config;
        void *handler;
        void* (*init)(void *db_config);
        char* (*get) (void *handle, char *key);
        int (*put) (void *handle, char *key, char *value);
        void (*watch) (void *handle, char *key, void (*callback)(char *key, char *value, void *cb_user_data), void *user_data);
        void (*watch_prefix) (void *handle, char *key, void (*callback)(char *key, char *value, void *cb_user_data), void *user_data);
        void (*deinit)(void *handle);
} db_client_t;


// Add datastore plugins here
db_client_t* create_etcd_client(char *hostname, char *port, char *cert_file, char *key_file, char *ca_cert_file);


#ifdef __cplusplus
}
#endif

#endif
