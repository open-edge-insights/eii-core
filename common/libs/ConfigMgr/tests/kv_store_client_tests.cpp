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
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.

/**
 * @brief KV Store Client Plugin GTests unit tests
 */

#include <gtest/gtest.h>
#include <stdio.h>
#include <stdlib.h>

#include "eis/config_manager/kv_store_plugin/kv_store_plugin.h"
#include "eis/utils/json_config.h"

#define KV_STORE_CONFIG "./kv_store_unittest_config.json"

static int watch_cb = 0;
static int watch_prefix_cb = 0;

void watch_callback(char* key, char* value, void *user_data){
    std::cout << "kv_store_client: watch_callback is called ....." << std::endl;
    watch_cb++;
}

void watch_prefix_callback(char* key, char* value, void *user_data){
    std::cout << "kv_store_client: watch_prefix_callback is called ....." << std::endl;
    watch_prefix_cb++;
}

kv_store_client_t* get_kv_store_client(){
    config_t* config = json_config_new(KV_STORE_CONFIG);
    kv_store_client_t *kv_store_client = create_kv_client(config);
    return kv_store_client;
}

TEST(KVStoreClientTest, create_kv_client) {
    std::cout << "Test Case: create configmgr instance..\n";
    kv_store_client_t* kv_store_client = get_kv_store_client();
    void *handle = kv_store_client->init(kv_store_client);
    ASSERT_NE(nullptr, kv_store_client);
    ASSERT_NE(nullptr, handle);
    kv_client_free(kv_store_client);
}

TEST(KVStoreClientTest, get){
    std::cout << "Test Case: get()\n";
    kv_store_client_t *kv_store_client = get_kv_store_client();
    EXPECT_NE(kv_store_client, nullptr);
    void *handle = kv_store_client->init(kv_store_client);

    int status = kv_store_client->put(handle, "/test_get()", "test_get_1234");
    EXPECT_EQ(status, 0);

    char *get_value = kv_store_client->get(handle, "/test_get()");
    ASSERT_STREQ("test_get_1234", get_value);

    kv_client_free(kv_store_client);
}

TEST(KVStoreClientTest, put){
    std::cout << "Test Case: put()\n";
    kv_store_client_t *kv_store_client = get_kv_store_client();
    EXPECT_NE(kv_store_client, nullptr);
    void *handle = kv_store_client->init(kv_store_client);

    int status = kv_store_client->put(handle, "/test_put()", "test_get_1234");
    ASSERT_EQ(0, status);

    kv_client_free(kv_store_client);
}

TEST(KVStoreClientTest, watch){
    std::cout << "Test Case: watch()\n";
    kv_store_client_t *kv_store_client = get_kv_store_client();
    EXPECT_NE(kv_store_client, nullptr);
    void *handle = kv_store_client->init(kv_store_client);

    kv_store_client->watch(handle, "/watch_test", watch_callback, NULL);
    sleep(5);
    int status1 = kv_store_client->put(handle, "/watch_test", "test_get_123456");
    int status2 = kv_store_client->put(handle, "/watch_test", "test_get_123456");
    sleep(5);
    ASSERT_EQ(2, watch_cb);
    kv_client_free(kv_store_client); 
}

TEST(KVStoreClientTest, watch_prefix){
    std::cout << "Test Case: watch_prefix()\n";
    kv_store_client_t *kv_store_client = get_kv_store_client();
    EXPECT_NE(kv_store_client, nullptr);
    void *handle = kv_store_client->init(kv_store_client);

    kv_store_client->watch_prefix(handle, "/pre", watch_prefix_callback, NULL);
    sleep(5);
    int status1 = kv_store_client->put(handle, "/prefixwatch", "test_get_1234");
    int status2 = kv_store_client->put(handle, "/prefixwatch", "test_get");
    sleep(5);
    ASSERT_EQ(2, watch_prefix_cb);
    kv_client_free(kv_store_client); 
}

int main(int argc, char **argv) {

    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();

}
