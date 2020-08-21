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
 * @brief Etcd Client Plugin implementation
 */

#include "db_client.h"

#include <stdlib.h>


db_client_t* create_etcd_client(char *hostname, char *port, char *cert_file, char *key_file, char *ca_cert_file) {
    db_client_t *db_client = NULL;
    etcd_config_t *etcd_config = NULL;
    etcd_config = (etcd_config_t*)malloc(sizeof(etcd_config_t));
    db_client = (db_client_t*)malloc(sizeof(db_client_t));

    etcd_config->hostname = hostname;
    etcd_config->port = port;
    etcd_config->cert_file = cert_file;
    etcd_config->key_file = key_file;
    etcd_config->ca_cert_file = ca_cert_file;

    db_client->db_config = etcd_config;
    db_client->get = etcd_get;
    db_client->put = etcd_put;
    db_client->watch = etcd_watch;
    db_client->watch_prefix = etcd_watch_prefix;
    db_client->init = etcd_init;
    db_client->deinit = etcd_client_free;
    // db_client->db_destroy =  etcd_client_destroy;
    return db_client;
}

void db_client_free(db_client_t* db_client) {
    if(db_client != NULL){
        db_client->deinit(db_client->handler);
        if(db_client->db_config != NULL) {
             free(db_client->db_config);
        }
        free(db_client);
    }
}
