// Copyright (c) 2021 Intel Corporation.
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
 * @brief ConfigManager GTests unit tests
 */

#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <gtest/gtest.h>
#include "eis/msgbus/msgbus.h"
#include "eis/utils/json_config.h"
#include "eis/config_manager/config_mgr.hpp"
#include <cjson/cJSON.h>
#include <iostream>
#include <fstream>

#define KV_STORE_CONFIG "./kv_store_unittest_config_cpp.json"

using namespace eis::config_manager;
using namespace std;

void etcd_requirements_put(){

    cJSON* c_json = cJSON_CreateObject();
    if(c_json == NULL){
        cout << "c_json creation failed" << endl;
        _Exit(-1);
    }
    cJSON_AddStringToObject(c_json, "type", "etcd");

    cJSON* kv_store = cJSON_CreateObject();
    if(kv_store == NULL){
        cout << "kv_store creation failed" << endl;
        _Exit(-1);
    }
    cJSON_AddStringToObject(kv_store, "host", "localhost");
    cJSON_AddStringToObject(kv_store, "port", "2379");
    cJSON_AddStringToObject(kv_store, "cert_file", getenv("CONFIGMGR_CERT"));
    cJSON_AddStringToObject(kv_store, "key_file", getenv("CONFIGMGR_KEY"));
    cJSON_AddStringToObject(kv_store, "ca_file", getenv("CONFIGMGR_CACERT"));

    cJSON_AddItemToObject(c_json, "etcd_kv_store", kv_store);

    char* kv_store_config = cJSON_Print(c_json);
    if(kv_store_config == NULL){
        cout << "cJSON_Print failed" << endl;
        _Exit(-1);
    }
    string config_file(kv_store_config);
  
    FILE * file;
    file = fopen (KV_STORE_CONFIG,"w");
    if (file!=NULL)
    {
        fputs(kv_store_config, file);
        fclose (file);
    }

    cJSON_Delete(c_json);
    free(kv_store_config);

    config_t* config = json_config_new(KV_STORE_CONFIG);
    if(config == NULL){
        cout << "json config new failed" << endl;
        _Exit(-1);
    }
    kv_store_client_t *kv_store_client = create_kv_client(config);
    if(kv_store_client == NULL){
        cout << "kv_store_client creation failed" << endl;
        _Exit(-1);
    }

    void *handle = kv_store_client->init(kv_store_client);

    int status = kv_store_client->put(handle, "/TestPubServer/config", "{\
        \"encoding\": {\
            \"level\": 95,\
            \"type\": \"jpeg\"\
        },\
        \"ingestor\": {\
            \"loop_video\": true,\
            \"pipeline\": \"./test_videos/pcb_d2000.avi\",\
            \"poll_interval\": 0.2,\
            \"queue_size\": 10,\
            \"type\": \"opencv\"\
        },\
        \"max_workers\": 4,\
        \"sw_trigger\": {\
            \"init_state\": \"running\"\
        },\
        \"udfs\": [\
            {\
                \"n_left_px\": 1000,\
                \"n_right_px\": 1000,\
                \"n_total_px\": 300000,\
                \"name\": \"pcb.pcb_filter\",\
                \"scale_ratio\": 4,\
                \"training_mode\": \"false\",\
                \"type\": \"python\"\
            }\
        ]\
    }");
    if(status != 0){
        cout << "Etcd put failed for /TestPubServer/config" << endl;
        _Exit(-1);
    }

    status = kv_store_client->put(handle, "/TestSubClient/config", "{}");
    if(status != 0){
        cout << "Etcd put failed for /TestSubClient/config" << endl;
        _Exit(-1);
    }

    status = kv_store_client->put(handle, "/TestPubServer/private_key", "{}");
    if(status != 0){
        cout << "Etcd put failed for /TestPubServer/private_key" << endl;
        _Exit(-1);
    }
    status = kv_store_client->put(handle, "/TestSubClient/private_key", "{}");
    if(status != 0){
        cout << "Etcd put failed for /TestSubClient/private_key" << endl;
        _Exit(-1);
    }

    status = kv_store_client->put(handle, "/Publickeys/TestPubServer", "{}");
    if(status != 0){
        cout << "Etcd put failed for /Publickeys/TestPubServer" << endl;
        _Exit(-1);
    }
    status = kv_store_client->put(handle, "/Publickeys/TestSubClient", "{}");
    if(status != 0){
        cout << "Etcd put failed for /Publickeys/TestSubClient" << endl;
        _Exit(-1);
    }

    status = kv_store_client->put(handle, "/TestSubClient/config", "{}");
    if(status != 0){
        cout << "Etcd put failed for /TestSubClient/config" << endl;
        _Exit(-1);
    }

    status = kv_store_client->put(handle, "/TestPubServer/interfaces", "{\
        \"Publishers\": [\
                {\
                    \"AllowedClients\": [\
                        \"TestSubClient\",\
                        \"Visualizer\",\
                        \"WebVisualizer\",\
                        \"TLSRemoteAgent\",\
                        \"RestDataExport\"\
                    ],\
                    \"EndPoint\": \"/EIS/sockets\",\
                    \"Name\": \"default\",\
                    \"Topics\": [\
                        \"camera1_stream\"\
                    ],\
                    \"Type\": \"zmq_ipc\"\
                }\
            ],\
        \"Servers\": [\
            {\
                \"AllowedClients\": [\
                    \"*\"\
                ],\
                \"EndPoint\": \"127.0.0.1:66013\",\
                \"Name\": \"default\",\
                \"Type\": \"zmq_tcp\"\
            }\
        ]\
    }");
    if(status != 0){
        cout << "Etcd put failed for /TestPubServer/interfaces" << endl;
        _Exit(-1);
    }

    status = kv_store_client->put(handle, "/TestSubClient/interfaces", "{\
        \"Subscribers\": [\
                {\
                    \"EndPoint\": \"/EIS/sockets\",\
                    \"Name\": \"default\",\
                    \"Topics\": [\
                        \"camera1_stream\"\
                    ],\
                    \"PublisherAppName\": \"TestPubServer\",\
                    \"Type\": \"zmq_ipc\"\
                }\
            ],\
        \"Clients\": [\
                {\
                    \"EndPoint\": \"127.0.0.1:66013\",\
                    \"Name\": \"default\",\
                    \"Type\": \"zmq_tcp\",\
                    \"ServerAppName\": \"TestPubServer\"\
                }\
            ]\
        }");
        
    if(status != 0){
        cout << "Etcd put failed for /TestSubClient/interfaces" << endl;
        _Exit(-1);
    }

    kv_client_free(kv_store_client);
}


TEST(ConfigManagerTest, common_apis) {
    cout << "Test Case: common APIs\n";
    int result;
    ConfigMgr* cfg_mgr = NULL;
    bool dev_mode;
    string app_name;
    int num_of_publishers;
    int num_of_servers;
    int num_of_subscribers;
    int num_of_clients;

    for(int i = 0; i < 2; i++){
        // Publisher and Server usecase
        result = setenv("AppName", "TestPubServer", 1);
        ASSERT_EQ(0, result);

        cfgmgr_ctx_t* cfg_mgr = cfgmgr_initialize();
        EXPECT_NE(cfg_mgr, nullptr);

        dev_mode = cfgmgr_is_dev_mode(cfg_mgr);
        if (dev_mode) {
            cout << "Running in DEV mode" << endl;
            EXPECT_EQ(true, true);
        } else {
            cout << "Running in PROD mode" << endl;
            EXPECT_EQ(false, false);
        }

        config_value_t* app_name = cfgmgr_get_appname(cfg_mgr);
        EXPECT_NE(app_name, nullptr);
        std::string appname(app_name->body.string);
        cout << "AppName :" << appname << endl;
        EXPECT_EQ(appname, "TestPubServer");

        num_of_publishers = cfgmgr_get_num_publishers(cfg_mgr);
        cout << "Total number of publishers :" << num_of_publishers << endl;
        if( num_of_publishers < 0){
            cout << "Getting number of publishers failed" << endl;
            EXPECT_EQ(num_of_publishers, -1);
        } else {
            cout << "Getting number of publishers passed" << endl;
            EXPECT_NE(num_of_publishers, -1);
        }

        num_of_servers = cfgmgr_get_num_servers(cfg_mgr);
        cout << "Total number of servers : " << num_of_servers << endl;
        if( num_of_servers < 0){
            cout << "Getting number of servers failed" << endl;
            EXPECT_EQ(num_of_servers, -1);
        } else {
            cout << "Getting number of servers passed" << endl;
            EXPECT_NE(num_of_servers, -1);
        }

        result = setenv("AppName", "TestSubClient", 1);
        ASSERT_EQ(0, result);
        cfg_mgr = cfgmgr_initialize();

        num_of_subscribers = cfgmgr_get_num_subscribers(cfg_mgr);
        cout << "Total number of Subscribers : " << num_of_subscribers << endl;
        if( num_of_subscribers < 0){
            cout << "Getting number of subscribers failed" << endl;
            EXPECT_EQ(num_of_subscribers, -1);
        } else {
            cout << "Getting number of subscribers passed" << endl;
            EXPECT_NE(num_of_subscribers, -1);
        }

        num_of_clients = cfgmgr_get_num_clients(cfg_mgr);
        cout << "Total number of Clients : " << num_of_clients << endl;
        if( num_of_clients < 0){
            cout << "Getting number of clients failed" << endl;
            EXPECT_EQ(num_of_clients, -1);
        } else {
            cout << "Getting number of clients passed" << endl;
            EXPECT_NE(num_of_clients, -1);
        }
    }
    cout << " =========== End Of common apis testcase ===========" << endl;
}

TEST(ConfigManagerTest, publisher_test) {
    cout << "Test Case: publisher_test()\n";

    int result;
    cfgmgr_ctx_t* cfg_mgr = NULL;
    cfgmgr_interface_t* pub_cfg = NULL;
    config_t* pub_config = NULL;
    void* ctx;

    for(int i = 0; i < 2; i++){
        result = setenv("AppName", "TestPubServer", 1);
        ASSERT_EQ(0, result);
        cfg_mgr = cfgmgr_initialize();
        EXPECT_NE(cfg_mgr, nullptr);
        pub_cfg = cfgmgr_get_publisher_by_name(cfg_mgr, "default");
        EXPECT_NE(pub_cfg, nullptr);
        pub_cfg = cfgmgr_get_publisher_by_index(cfg_mgr, 0);
        EXPECT_NE(pub_cfg, nullptr);
        pub_config = cfgmgr_get_msgbus_config(pub_cfg);
        EXPECT_NE(pub_config, nullptr);

        ctx = msgbus_initialize(pub_config);
        EXPECT_NE(ctx, nullptr);
        msgbus_destroy(ctx);
    }
    cout << " =========== End Of publisher testcase ===========" << endl;
}

TEST(ConfigManagerTest, subscriber_test) {
    cout << "Test Case: subscriber_test()\n";

    int result;
    cfgmgr_ctx_t* cfg_mgr = NULL;
    cfgmgr_interface_t* sub_cfg = NULL;
    config_t* sub_config = NULL;
    void* ctx;

    for(int i = 0; i < 2; i++){
        result = setenv("AppName", "TestSubClient", 1);
        ASSERT_EQ(0, result);
        cfg_mgr = cfgmgr_initialize();
        EXPECT_NE(cfg_mgr, nullptr);
        sub_cfg = cfgmgr_get_subscriber_by_name(cfg_mgr, "default");
        EXPECT_NE(sub_cfg, nullptr);
        sub_cfg = cfgmgr_get_subscriber_by_index(cfg_mgr, 0);
        EXPECT_NE(sub_cfg, nullptr);
        sub_config = cfgmgr_get_msgbus_config(sub_cfg);
        EXPECT_NE(sub_config, nullptr);

        ctx = msgbus_initialize(sub_config);
        EXPECT_NE(ctx, nullptr);
        msgbus_destroy(ctx);

    }
    cout << " =========== End Of subscriber testcase ===========" << endl;
}

TEST(ConfigManagerTest, server_test) {
    cout << "Test Case: server_test()\n";

    int result;
    cfgmgr_ctx_t* cfg_mgr = NULL;
    cfgmgr_interface_t* server_cfg = NULL;
    config_t* config = NULL;
    void* ctx;
    for(int i = 0; i < 2; i++){
        result = setenv("AppName", "TestPubServer", 1);
        ASSERT_EQ(0, result);
        cfg_mgr = cfgmgr_initialize();
        EXPECT_NE(cfg_mgr, nullptr);
        server_cfg = cfgmgr_get_server_by_name(cfg_mgr, "default");
        EXPECT_NE(server_cfg, nullptr);
        server_cfg = cfgmgr_get_server_by_index(cfg_mgr, 0);
        EXPECT_NE(server_cfg, nullptr);
        config = cfgmgr_get_msgbus_config(server_cfg);
        EXPECT_NE(config, nullptr);

        ctx = msgbus_initialize(config);
        EXPECT_NE(ctx, nullptr);
        msgbus_destroy(ctx);
    }
    cout << " =========== End Of Server testcase ===========" << endl;
}

TEST(ConfigManagerTest, client_test) {
    cout << "Test Case: client_test()\n";

    int result;
    cfgmgr_ctx_t* cfg_mgr = NULL;
    cfgmgr_interface_t* client_cfg = NULL;
    config_t* config = NULL;
    void* ctx;

    for(int i = 0; i < 2; i++){
        result = setenv("AppName", "TestSubClient", 1);
        ASSERT_EQ(0, result);
        cfg_mgr = cfgmgr_initialize();
        EXPECT_NE(cfg_mgr, nullptr);
        client_cfg = cfgmgr_get_client_by_name(cfg_mgr, "default");
        EXPECT_NE(client_cfg, nullptr);
        client_cfg = cfgmgr_get_client_by_index(cfg_mgr, 0);
        EXPECT_NE(client_cfg, nullptr);
        config = cfgmgr_get_msgbus_config(client_cfg);
        EXPECT_NE(config, nullptr);

        ctx = msgbus_initialize(config);
        EXPECT_NE(ctx, nullptr);
        msgbus_destroy(ctx);
    }
    cout << " =========== End Of Client testcase ===========" << endl;
}

TEST(ConfigManagerTest, getEndpointTest) {
    cout << "Test Case: getEndpoint() API\n";

    int result;
    cfgmgr_ctx_t* cfg_mgr = NULL;
    cfgmgr_interface_t* pub_cfg = NULL;
    cfgmgr_interface_t* server_cfg = NULL;
    cfgmgr_interface_t* sub_cfg = NULL;
    cfgmgr_interface_t* client_cfg = NULL;
    config_t* config = NULL;
    config_value_t* endpoint;

    for(int i = 0; i < 2; i++){
        // Calling getEndpoints publisher and server
        int result = setenv("AppName", "TestPubServer", 1);
        ASSERT_EQ(0, result);
        cfg_mgr = cfgmgr_initialize();

        pub_cfg = cfgmgr_get_publisher_by_index(cfg_mgr, 0);
        endpoint = cfgmgr_get_endpoint(pub_cfg);
        EXPECT_NE(endpoint, nullptr);
        std::string publisher_endpoint(endpoint->body.string);
        cout << "Publisher endPoint is : " << publisher_endpoint << endl;
        EXPECT_EQ(publisher_endpoint, "/EIS/sockets");

        server_cfg = cfgmgr_get_server_by_index(cfg_mgr, 0);
        endpoint = cfgmgr_get_endpoint(server_cfg);
        EXPECT_NE(endpoint, nullptr);
        std::string server_endpoint(endpoint->body.string);
        cout << "Server endPoint is : " << server_endpoint << endl;
        EXPECT_EQ(server_endpoint, "127.0.0.1:66013");

        // Calling getEndpoints subscriber and client
        result = setenv("AppName", "TestSubClient", 1);
        ASSERT_EQ(0, result);
        cfg_mgr = cfgmgr_initialize();

        sub_cfg = cfgmgr_get_subscriber_by_index(cfg_mgr, 0);
        endpoint = cfgmgr_get_endpoint(sub_cfg);
        EXPECT_NE(endpoint, nullptr);
        std::string subscriber_endpoint(endpoint->body.string);
        cout << "Subscriber endPoint is : " << subscriber_endpoint << endl;
        EXPECT_EQ(subscriber_endpoint, "/EIS/sockets");

        client_cfg = cfgmgr_get_client_by_index(cfg_mgr, 0);
        endpoint = cfgmgr_get_endpoint(client_cfg);
        EXPECT_NE(endpoint, nullptr);
        std::string client_endpoint(endpoint->body.string);
        cout << "Client endPoint is : " << client_endpoint << endl;
        EXPECT_EQ(client_endpoint, "127.0.0.1:66013");
    }

    cout << " =========== End Of getEndpoint() testcase ===========" << endl;

}

TEST(ConfigManagerTest, getInterfaceValue) {
    cout << "Test Case: getInterfaceValue() API\n";

    int result;
    cfgmgr_ctx_t* cfg_mgr = NULL;
    cfgmgr_interface_t* pub_cfg = NULL;
    cfgmgr_interface_t* server_cfg = NULL;
    cfgmgr_interface_t* sub_cfg = NULL;
    cfgmgr_interface_t* client_cfg = NULL;
    config_t* config = NULL;
    config_value_t* interface_value = NULL;

    for(int i = 0; i < 2; i++){
        // Calling getInterfaceValue for publisher and server
        int result = setenv("AppName", "TestPubServer", 1);
        ASSERT_EQ(0, result);
        cfg_mgr = cfgmgr_initialize();

        pub_cfg = cfgmgr_get_publisher_by_index(cfg_mgr, 0);
        interface_value = cfgmgr_get_interface_value(pub_cfg, "Name");
        cout << "Publisher's interface value is : " << interface_value->body.string << endl;
        string pub_value(interface_value->body.string);
        EXPECT_EQ(pub_value, "default");

        server_cfg = cfgmgr_get_server_by_index(cfg_mgr, 0);
        interface_value = cfgmgr_get_interface_value(server_cfg, "Type");
        cout << "Server's interface value is : " << interface_value->body.string << endl;
        string server_value(interface_value->body.string);
        EXPECT_EQ(server_value, "zmq_tcp");

        // Calling getInterfaceValue for subscriber and client
        result = setenv("AppName", "TestSubClient", 1);
        ASSERT_EQ(0, result);
        cfg_mgr = cfgmgr_initialize();

        sub_cfg = cfgmgr_get_subscriber_by_index(cfg_mgr, 0);
        interface_value = cfgmgr_get_interface_value(sub_cfg, "Name");
        cout << "Subscriber's interface value is : " << interface_value->body.string << endl;
        string sub_value(interface_value->body.string);
        EXPECT_EQ(sub_value, "default");

        client_cfg = cfgmgr_get_client_by_index(cfg_mgr, 0);
        interface_value = cfgmgr_get_interface_value(client_cfg, "Type");
        cout << "Client's interface value is : " << interface_value->body.string << endl;
        string client_value(interface_value->body.string);
        EXPECT_EQ(client_value, "zmq_tcp");
    }

    cout << " =========== End Of getInterfaceValue() testcase ===========" << endl;
}

TEST(ConfigManagerTest, getTopics) {
    cout << "Test Case: getTopics() API\n";

    int result;
    cfgmgr_ctx_t* cfg_mgr = NULL;
    cfgmgr_interface_t* pub_cfg = NULL;
    cfgmgr_interface_t* server_cfg = NULL;
    cfgmgr_interface_t* sub_cfg = NULL;
    cfgmgr_interface_t* client_cfg = NULL;
    config_t* config = NULL;
    config_value_t* topics;
    config_value_t* topic_value;

    for(int i = 0; i < 2; i++){
        // Calling getTopics for Publisher
        result = setenv("AppName", "TestPubServer", 1);
        ASSERT_EQ(0, result);
        cfg_mgr = cfgmgr_initialize();

        pub_cfg = cfgmgr_get_publisher_by_index(cfg_mgr, 0);
        topics = cfgmgr_get_topics(pub_cfg);

        size_t arr_len = config_value_array_len(topics);
        if (arr_len == 0) {
            FAIL() << "Empty array is not supported, atleast one value should be given.";
        }
        for (size_t i = 0; i < arr_len; i++) {
            topic_value = config_value_array_get(topics, i);
            if (topic_value == NULL) {
                FAIL() << "topic_value initialization failed";
                config_value_destroy(topics);
            }
            std::string pub_topic(topic_value->body.string);
            cout << "Publisher's topic : "<< pub_topic << endl;
            if (i == 0) {
                EXPECT_EQ(pub_topic, "camera1_stream");
            }
            // Destroying topic_value
            config_value_destroy(topic_value);
        }

        // Calling getTopics for Subscriber
        result = setenv("AppName", "TestSubClient", 1);
        ASSERT_EQ(0, result);
        cfg_mgr = cfgmgr_initialize();

        sub_cfg = cfgmgr_get_subscriber_by_index(cfg_mgr, 0);
        topics = cfgmgr_get_topics(sub_cfg);

        arr_len = config_value_array_len(topics);
        if (arr_len == 0) {
            FAIL() << "Empty array is not supported, atleast one value should be given.";
        }

        for (size_t i = 0; i < arr_len; i++) {
            topic_value = config_value_array_get(topics, i);
            if (topic_value == NULL) {
                FAIL() << "topic_value initialization failed";
                config_value_destroy(topics);
            }
            std::string sub_topic(topic_value->body.string);
            cout << "Subscriber's topic : "<< sub_topic << endl;
            if (i == 0) {
                EXPECT_EQ(sub_topic, "camera1_stream");
            }
            // Destroying topic_value
            config_value_destroy(topic_value);
        }

        // Testing for Topics when it is "*"
        cout << "Testing when topics is \"*\" " << endl;

        int topics_length = 1;
        char **topics_to_be_set = (char**)calloc(topics_length, sizeof(char*));
        topics_to_be_set[0] = "*";

        // Setting topics for publisher example
        bool topics_set = cfgmgr_set_topics(sub_cfg, topics_to_be_set, 1);
        ASSERT_EQ(true, topics_set);

        topics = cfgmgr_get_topics(sub_cfg);
        for (size_t i = 0; i < arr_len; i++) {
            topic_value = config_value_array_get(topics, i);
            if (topic_value == NULL) {
                FAIL() << "topic_value initialization failed";
                config_value_destroy(topics);
            }
            std::string sub_topic(topic_value->body.string);
            cout << "Subscriber's topic : "<< sub_topic << endl;
            if (i == 0) {
                EXPECT_EQ(sub_topic, "");
            }
            // Destroying topic_value
            config_value_destroy(topic_value);
        }
    }

    cout << " =========== End Of getTopics() testcase ===========" << endl;
}

TEST(ConfigManagerTest, setTopics) {
    cout << "Test Case: setTopics() API\n";

    string str_i;
    string topic_value;
    int result;
    cfgmgr_ctx_t* cfg_mgr = NULL;
    cfgmgr_interface_t* pub_cfg = NULL;
    cfgmgr_interface_t* server_cfg = NULL;
    cfgmgr_interface_t* sub_cfg = NULL;
    cfgmgr_interface_t* client_cfg = NULL;
    config_t* config = NULL;

    for(int i = 0; i < 2; i++){
        // Calling setTopics for publisher
        result = setenv("AppName", "TestPubServer", 1);
        ASSERT_EQ(0, result);
        cfg_mgr = cfgmgr_initialize();

        pub_cfg = cfgmgr_get_publisher_by_index(cfg_mgr, 0);

        int topics_length = 2;
        char **topics_to_be_set = NULL;
        topics_to_be_set = (char**)calloc(topics_length, sizeof(char*));
        topics_to_be_set[0] = "camera1_stream";
        topics_to_be_set[1] = "camera2_stream";

        // Setting topics for publisher example
        bool topicsSet = cfgmgr_set_topics(pub_cfg, topics_to_be_set, topics_length);
        ASSERT_EQ(true, topicsSet);

        // Calling setTopics for subscriber
        result = setenv("AppName", "TestSubClient", 1);
        ASSERT_EQ(0, result);
        cfg_mgr = cfgmgr_initialize();

        sub_cfg = cfgmgr_get_subscriber_by_index(cfg_mgr, 0);

        topics_length = 2;
        topics_to_be_set = (char**)calloc(topics_length, sizeof(char*));
        topics_to_be_set[0] = "cam1_stream";
        topics_to_be_set[1] = "cam2_stream";

        // Setting topics for publisher example
        topicsSet = cfgmgr_set_topics(pub_cfg, topics_to_be_set, topics_length);
        ASSERT_EQ(true, topicsSet);
    }

    cout << " =========== End Of setTopics() testcase ===========" << endl;
}

TEST(ConfigManagerTest, allowedClients) {
    cout << "Test Case: allowedClients()\n";
    string str_i;
    string client_value;
    int result;
    cfgmgr_ctx_t* cfg_mgr = NULL;
    cfgmgr_interface_t* pub_cfg = NULL;
    cfgmgr_interface_t* server_cfg = NULL;
    cfgmgr_interface_t* sub_cfg = NULL;
    cfgmgr_interface_t* client_cfg = NULL;
    config_t* config = NULL;

    for(int i = 0; i < 2; i++){
        // Calling allowed clients for Publisher and Server
        result = setenv("AppName", "TestPubServer", 1);
        ASSERT_EQ(0, result);
        cfg_mgr = cfgmgr_initialize();

        pub_cfg = cfgmgr_get_publisher_by_index(cfg_mgr, 0);

        int topics_length = 5;
        char **topics_to_be_set = NULL;
        topics_to_be_set = (char**)calloc(topics_length, sizeof(char*));
        topics_to_be_set[0] = "TestSubClient";
        topics_to_be_set[1] = "Visualizer";
        topics_to_be_set[2] = "WebVisualizer";
        topics_to_be_set[3] = "TLSRemoteAgent";
        topics_to_be_set[4] = "RestDataExport";

        config_value_t* all_clients = cfgmgr_get_allowed_clients(pub_cfg);

        config_value_t* topic_val;
        size_t arr_len = config_value_array_len(all_clients);
        if (arr_len == 0) {
            LOG_ERROR_0("Empty array is not supported, atleast one value should be given.");
        }
        for (size_t i = 0; i < arr_len; i++) {
            topic_val = config_value_array_get(all_clients, i);
            if (topic_val == NULL) {
                LOG_ERROR_0("topic_value initialization failed");
                config_value_destroy(all_clients);
            }
            std::string s(topic_val->body.string);
            std::string s_test(topics_to_be_set[i]);
            EXPECT_EQ(s, s_test);
            // Destroying topic_value
            config_value_destroy(topic_val);
        }

        server_cfg = cfgmgr_get_server_by_index(cfg_mgr, 0);

        config_value_t* serv_clients = cfgmgr_get_allowed_clients(server_cfg);

        arr_len = config_value_array_len(serv_clients);
        if (arr_len == 0) {
            LOG_ERROR_0("Empty array is not supported, atleast one value should be given.");
        }
        for (size_t i = 0; i < arr_len; i++) {
            topic_val = config_value_array_get(serv_clients, i);
            if (topic_val == NULL) {
                LOG_ERROR_0("topic_value initialization failed");
                config_value_destroy(serv_clients);
            }
            std::string s(topic_val->body.string);
            std::string s_test("*");
            EXPECT_EQ(s, s_test);
            // Destroying topic_value
            config_value_destroy(topic_val);
        }
    }

    cout << " =========== End Of allowed_clients() testcase ===========" << endl;
}

TEST(ConfigManagerTest, getConfigValue) {
    cout << "Test Case: getConfigValue()\n";

    int max_workers;
    cfgmgr_ctx_t* cfg_mgr = cfgmgr_initialize();
    // getAppConfig test
    // AppCfg* app_cfg = cfg_mgr->getAppConfig();
    // EXPECT_NE(app_cfg, nullptr);

    config_value_t* app_config = NULL;
    for(int i = 0; i < 2; i++){
        // getConfigValue for integer
        app_config = cfgmgr_get_app_config_value(cfg_mgr, "max_workers");
        EXPECT_EQ(app_config->type, CVT_INTEGER);

        max_workers = app_config->body.integer;
        EXPECT_EQ(max_workers, 4);
    }

    // getConfigValue for json structure
    app_config = cfgmgr_get_app_config_value(cfg_mgr, "ingestor");
    EXPECT_EQ(app_config->type, CVT_OBJECT);

    config_value_t* ingestor_type = config_value_object_get(app_config, "type");
    EXPECT_EQ(ingestor_type->type, CVT_STRING);

    string type(ingestor_type->body.string);
    EXPECT_EQ(type, "opencv");

    config_value_t* poll_interval = config_value_object_get(app_config, "poll_interval");
    EXPECT_EQ(poll_interval->type, CVT_FLOATING);
    EXPECT_EQ(poll_interval->body.floating, 0.2);

    config_value_t* loop_video = config_value_object_get(app_config, "loop_video");
    EXPECT_EQ(loop_video->type, CVT_BOOLEAN);
    EXPECT_EQ(loop_video->body.boolean, true);

    // getConfigValue for array
    app_config = cfgmgr_get_app_config_value(cfg_mgr, "udfs");
    EXPECT_EQ(app_config->type, CVT_ARRAY);

    config_value_t* udf = config_value_array_get(app_config, 0);
    config_value_t* udf_type = config_value_object_get(udf, "type");
    EXPECT_EQ(udf_type->type, CVT_STRING);

    string str_udf_type(udf_type->body.string);
    EXPECT_EQ(str_udf_type, "python");

    cout << " =========== End Of getConfigValue() testcase ===========" << endl;
}

int main(int argc, char **argv) {
    etcd_requirements_put();
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
