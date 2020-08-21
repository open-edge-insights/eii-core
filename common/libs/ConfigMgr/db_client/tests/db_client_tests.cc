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

void watch_callback(char* key, char* value, void *user_data){
    std::cout << "db_client: watch_callback is called ....." << std::endl;
    watch_cb++;
}

void watch_prefix_callback(char* key, char* value, void *user_data){
    std::cout << "db_client: watch_prefix_callback is called ....." << std::endl;
    watch_prefix_cb++;
}

db_client_t* get_db_client(){
    db_client_t *db_client = create_etcd_client(hostname, port, "", "", "");
    return db_client;
}

TEST(DBClientTest, create_etcd_client) {
    std::cout << "Test Case: create configmgr instance..\n";
    db_client_t* db_client = get_db_client();
    void *handle = db_client->init(db_client);
    ASSERT_NE(nullptr, db_client);
    ASSERT_NE(nullptr, handle);
    db_client_free(db_client);
}

TEST(DBClientTest, get){
    std::cout << "Test Case: get()\n";
    db_client_t *db_client = get_db_client();
    EXPECT_NE(db_client, nullptr);
    void *handle = db_client->init(db_client);

    int status = db_client->put(handle, "/test_get()", "test_get_1234");
    EXPECT_EQ(status, 0);

    char *get_value = db_client->get(handle, "/test_get()");
    ASSERT_STREQ("test_get_1234", get_value);

    db_client_free(db_client);
}

TEST(DBClientTest, put){
    std::cout << "Test Case: put()\n";
    db_client_t *db_client = get_db_client();
    EXPECT_NE(db_client, nullptr);
    void *handle = db_client->init(db_client);

    int status = db_client->put(handle, "/test_put()", "test_get_1234");
    ASSERT_EQ(0, status);

    db_client_free(db_client);
}

TEST(DBClientTest, watch){
    std::cout << "Test Case: watch()\n";
    db_client_t *db_client = get_db_client();
    EXPECT_NE(db_client, nullptr);
    void *handle = db_client->init(db_client);

    db_client->watch(handle, "/watch_test", watch_callback, NULL);
    sleep(5);
    int status1 = db_client->put(handle, "/watch_test", "test_get_123456");
    int status2 = db_client->put(handle, "/watch_test", "test_get_123456");
    sleep(5);
    ASSERT_EQ(2, watch_cb);
    db_client_free(db_client); 
}

TEST(DBClientTest, watch_prefix){
    std::cout << "Test Case: watch_prefix()\n";
    db_client_t *db_client = get_db_client();
    EXPECT_NE(db_client, nullptr);
    void *handle = db_client->init(db_client);

    db_client->watch_prefix(handle, "/pre", watch_prefix_callback, NULL);
    sleep(5);
    int status1 = db_client->put(handle, "/prefixwatch", "test_get_1234");
    int status2 = db_client->put(handle, "/prefixwatch", "test_get");
    sleep(5);
    ASSERT_EQ(2, watch_prefix_cb);
    db_client_free(db_client); 
}

int main(int argc, char **argv) {

    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();

}
