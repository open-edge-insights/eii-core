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
 * @brief Etcd Client Plugin GTests unit tests
 * @author Varalakshmi KA (varalakshmi.ka@intel.com)
 */

#include <gtest/gtest.h>
#include <stdio.h>
#include "db_client.h"
#include <stdlib.h>

static char *hostname = "localhost";
static char *port = "2379";
static int watch_cb = 0;
static int watch_prefix_cb = 0;

void watch_callback(char* key, char* value){
    printf("watch_callback is called .....\n");
    watch_cb++;
}

void watch_prefix_callback(char* key, char* value){
    printf("watch_prefix_callback is called .....\n");
    watch_prefix_cb++;
}

db_client_t* get_config_mgr_client(){
    db_client_t *db_client = create_etcd_client(hostname, port, "", "", "");
    return db_client;
}

TEST(EtcdClientPluginTest, create_etcd_client) {
    printf("Test Case: create configmgr instance..\n");
    db_client_t* db_client = get_config_mgr_client();
    void *handle = init(db_client->db_config);
    ASSERT_NE(nullptr, db_client);
    ASSERT_NE(nullptr, handle);
    db_client_destroy(db_client);
}

TEST(EtcdClientPluginTest, get){
    printf("Test Case: get()\n");
    db_client_t *db_client = get_config_mgr_client();
    EXPECT_NE(db_client, nullptr);
    void *handle = init(db_client->db_config);

    int status = db_client->put(handle, "/test_get()", "test_get_1234");
    EXPECT_EQ(status, 0);

    char *get_value = db_client->get(handle, "/test_get()");
    ASSERT_STREQ("test_get_1234", get_value);

    db_client_destroy(db_client);
}

TEST(EtcdClientPluginTest, put){
    printf("Test Case: put()\n");
    db_client_t *db_client = get_config_mgr_client();
    EXPECT_NE(db_client, nullptr);
    void *handle = init(db_client->db_config);

    int status = db_client->put(handle, "/test_put()", "test_get_1234");
    ASSERT_EQ(0, status);

    db_client_destroy(db_client);
}

TEST(EtcdClientPluginTest, watch_prefix){
    printf("Test Case: watch()\n");
    db_client_t *db_client = get_config_mgr_client();
    EXPECT_NE(db_client, nullptr);
    void *handle = init(db_client->db_config);

    db_client->watch_prefix(handle, "/test", watch_prefix_callback);
    int status1 = db_client->put(handle, "/test_put()", "test_get_1234");
    int status2 = db_client->put(handle, "/test_put()", "test_get_1234");
    ASSERT_EQ(2, watch_prefix_cb);
    db_client_destroy(db_client); 
}

TEST(EtcdClientPluginTest, watch){
    printf("Test Case: watch_prefix()\n");
}

int main(int argc, char **argv) {

    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();

}
