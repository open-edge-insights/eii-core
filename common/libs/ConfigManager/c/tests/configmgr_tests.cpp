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
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.

/**
 * @brief Config Manager GTests unit tests
 * @author Varalakshmi KA (varalakshmi.ka@intel.com)
 */

#include <gtest/gtest.h>
#include <string.h>
#include <unistd.h>
#include "eis/config_manager/config_manager.h"

int watch_key_cb = 0;
int watch_dir_cb = 0;

void watch_key_callback(char* key, char* value){
    watch_key_cb++;
}

void watch_dir_callback(char* key, char* value){
    watch_dir_cb++;
}

config_mgr_t* get_config_mgr_client(char *storage_type){
    config_mgr_t *config_mgr_client = config_mgr_new(storage_type, "", "", "");
    return config_mgr_client;
}

TEST(configmgr_test, configmgr_init) {
    config_mgr_t *config_mgr_client = get_config_mgr_client((char*)"etcd");
    ASSERT_NE(nullptr, config_mgr_client);
    config_mgr_config_destroy(config_mgr_client);
}

TEST(configmgr_test, configmgr_get_config) {
    config_mgr_t *config_mgr_client = get_config_mgr_client((char*)"etcd");
    if (config_mgr_client == NULL){
        FAIL() << "Failed to create config manager client";
    }
    system("./etcdctl put test test123"); 
    char *value = config_mgr_client->get_config((char*)"test");
    config_mgr_config_destroy(config_mgr_client);
    ASSERT_STREQ("test123", value);
}

TEST(configmgr_test, configmgr_put_config) {
    config_mgr_t *config_mgr_client = get_config_mgr_client((char*)"etcd");
    if (config_mgr_client == NULL){
        FAIL() << "Failed to create config manager client";
    }
    char* key = "/Visualizer/datastore";
    char* val = "UnitTesting put_config api";
    int err_status = config_mgr_client->put_config(key, val);
    if (err_status == -1){
        FAIL() << "Failed to create config manager client";
    }
    
    char *value = config_mgr_client->get_config(key);
    config_mgr_config_destroy(config_mgr_client);
    ASSERT_STREQ(val, value);
}

TEST(configmgr_test, configmgr_register_watch_key) {
    config_mgr_t *config_mgr_client = get_config_mgr_client((char*)"etcd");
    if (config_mgr_client == NULL){
        FAIL() << "Failed to create config manager client";
    }
    system("./etcdctl put watch_key_test test123");
    config_mgr_client->register_watch_key((char*)"watch_key_test", watch_key_callback);
    sleep(2);
    system("./etcdctl put watch_key_test test12345678");
    config_mgr_config_destroy(config_mgr_client);
    ASSERT_EQ(1, watch_key_cb);
}

TEST(configmgr_test, configmgr_register_watch_dir) {
    config_mgr_t *config_mgr_client = get_config_mgr_client((char*)"etcd");
    if (config_mgr_client == NULL){
        FAIL() << "Failed to create config manager client";
    }
    system("./etcdctl put test_watch_dir test123");
    config_mgr_client->register_watch_dir((char*)"test_watch", watch_dir_callback);
    sleep(2);
    config_mgr_config_destroy(config_mgr_client);
    system("./etcdctl put test_watch_dir test12345678"); 
    ASSERT_EQ(1, watch_dir_cb);
}

TEST(configmgr_test, configmgr_init_fail) {
    config_mgr_t *config_mgr_client = get_config_mgr_client((char*)"test");
    ASSERT_EQ(nullptr, config_mgr_client);
    config_mgr_config_destroy(config_mgr_client);
}
