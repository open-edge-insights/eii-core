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

        cfg_mgr = new ConfigMgr();
        EXPECT_NE(cfg_mgr, nullptr);

        dev_mode = cfg_mgr->isDevMode();
        if (dev_mode) {
            cout << "Running in DEV mode" << endl;
            EXPECT_EQ(true, true);
        } else {
            cout << "Running in PROD mode" << endl;
            EXPECT_EQ(false, false);
        }

        app_name = cfg_mgr->getAppName();
        cout << "AppName :" << app_name << endl;
        EXPECT_EQ(app_name, "TestPubServer");

        num_of_publishers = cfg_mgr->getNumPublishers();
        cout << "Total number of publishers :" << num_of_publishers << endl;
        if( num_of_publishers < 0){
            cout << "Getting number of publishers failed" << endl;
            EXPECT_EQ(num_of_publishers, -1);
        } else {
            cout << "Getting number of publishers passed" << endl;
            EXPECT_NE(num_of_publishers, -1);
        }

        num_of_servers = cfg_mgr->getNumServers();
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
        cfg_mgr = new ConfigMgr();

        num_of_subscribers = cfg_mgr->getNumSubscribers();
        cout << "Total number of Subscribers : " << num_of_subscribers << endl;
        if( num_of_subscribers < 0){
            cout << "Getting number of subscribers failed" << endl;
            EXPECT_EQ(num_of_subscribers, -1);
        } else {
            cout << "Getting number of subscribers passed" << endl;
            EXPECT_NE(num_of_subscribers, -1);
        }

        num_of_clients = cfg_mgr->getNumClients();
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
    ConfigMgr* cfg_mgr = NULL;
    PublisherCfg* pub_cfg = NULL;
    config_t* pub_config = NULL;
    void* ctx;

    for(int i = 0; i < 2; i++){
        result = setenv("AppName", "TestPubServer", 1);
        ASSERT_EQ(0, result);
        cfg_mgr = new ConfigMgr();
        EXPECT_NE(cfg_mgr, nullptr);
        pub_cfg = cfg_mgr->getPublisherByName("default");
        EXPECT_NE(pub_cfg, nullptr);
        pub_cfg = cfg_mgr->getPublisherByIndex(0);
        EXPECT_NE(pub_cfg, nullptr);
        pub_config = pub_cfg->getMsgBusConfig();
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
    ConfigMgr* cfg_mgr = NULL;
    SubscriberCfg* sub_cfg = NULL;
    config_t* sub_config = NULL;
    void* ctx;

    for(int i = 0; i < 2; i++){
        result = setenv("AppName", "TestSubClient", 1);
        ASSERT_EQ(0, result);
        cfg_mgr = new ConfigMgr();
        EXPECT_NE(cfg_mgr, nullptr);
        sub_cfg = cfg_mgr->getSubscriberByName("default");
        EXPECT_NE(sub_cfg, nullptr);
        sub_cfg = cfg_mgr->getSubscriberByIndex(0);
        EXPECT_NE(sub_cfg, nullptr);
        sub_config = sub_cfg->getMsgBusConfig();
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
    ConfigMgr* cfg_mgr = NULL;
    ServerCfg* server_cfg = NULL;
    config_t* config = NULL;
    void* ctx;
    for(int i = 0; i < 2; i++){
        result = setenv("AppName", "TestPubServer", 1);
        ASSERT_EQ(0, result);
        cfg_mgr = new ConfigMgr();
        EXPECT_NE(cfg_mgr, nullptr);
        server_cfg = cfg_mgr->getServerByName("default");
        EXPECT_NE(server_cfg, nullptr);
        server_cfg = cfg_mgr->getServerByIndex(0);
        EXPECT_NE(server_cfg, nullptr);
        config = server_cfg->getMsgBusConfig();
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
    ConfigMgr* cfg_mgr = NULL;
    ClientCfg* client_cfg = NULL;
    config_t* config = NULL;
    void* ctx;

    for(int i = 0; i < 2; i++){
        result = setenv("AppName", "TestSubClient", 1);
        ASSERT_EQ(0, result);
        cfg_mgr = new ConfigMgr();
        EXPECT_NE(cfg_mgr, nullptr);
        client_cfg = cfg_mgr->getClientByName("default");
        EXPECT_NE(client_cfg, nullptr);
        client_cfg = cfg_mgr->getClientByIndex(0);
        EXPECT_NE(client_cfg, nullptr);
        config = client_cfg->getMsgBusConfig();
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
    ConfigMgr* cfg_mgr = NULL;
    PublisherCfg* pub_cfg = NULL;
    ServerCfg* server_cfg = NULL;
    SubscriberCfg* sub_cfg = NULL;
    ClientCfg* client_cfg = NULL;
    config_t* config = NULL;
    string endpoint;

    for(int i = 0; i < 2; i++){
        // Calling getEndpoints publisher and server
        int result = setenv("AppName", "TestPubServer", 1);
        ASSERT_EQ(0, result);
        cfg_mgr = new ConfigMgr();

        pub_cfg = cfg_mgr->getPublisherByIndex(0);
        endpoint = pub_cfg->getEndpoint();
        cout << "Publisher endPoint is : " << endpoint << endl;
        EXPECT_EQ(endpoint, "/EIS/sockets");

        server_cfg = cfg_mgr->getServerByIndex(0);
        endpoint = server_cfg->getEndpoint();
        cout << "Server endPoint is : " << endpoint << endl;
        EXPECT_EQ(endpoint, "127.0.0.1:66013");

        // Calling getEndpoints subscriber and client
        result = setenv("AppName", "TestSubClient", 1);
        ASSERT_EQ(0, result);
        cfg_mgr = new ConfigMgr();

        sub_cfg = cfg_mgr->getSubscriberByIndex(0);
        endpoint = sub_cfg->getEndpoint();
        cout << "Subscriber endPoint is : " << endpoint << endl;
        EXPECT_EQ(endpoint, "/EIS/sockets");

        client_cfg = cfg_mgr->getClientByIndex(0);
        endpoint = client_cfg->getEndpoint();
        cout << "Client endPoint is : " << endpoint << endl;
        EXPECT_EQ(endpoint, "127.0.0.1:66013");
    }

    cout << " =========== End Of getEndpoint() testcase ===========" << endl;

}

TEST(ConfigManagerTest, getInterfaceValue) {
    cout << "Test Case: getInterfaceValue() API\n";

    int result;
    ConfigMgr* cfg_mgr = NULL;
    PublisherCfg* pub_cfg = NULL;
    ServerCfg* server_cfg = NULL;
    SubscriberCfg* sub_cfg = NULL;
    ClientCfg* client_cfg = NULL;
    config_t* config = NULL;
    config_value_t* interface_value = NULL;

    for(int i = 0; i < 2; i++){
        // Calling getInterfaceValue for publisher and server
        int result = setenv("AppName", "TestPubServer", 1);
        ASSERT_EQ(0, result);
        cfg_mgr = new ConfigMgr();

        pub_cfg = cfg_mgr->getPublisherByIndex(0);
        interface_value = pub_cfg->getInterfaceValue("Name");
        cout << "Publisher's interface value is : " << interface_value->body.string << endl;
        string pub_value(interface_value->body.string);
        EXPECT_EQ(pub_value, "default");

        server_cfg = cfg_mgr->getServerByIndex(0);
        interface_value = server_cfg->getInterfaceValue("Type");
        cout << "Server's interface value is : " << interface_value->body.string << endl;
        string server_value(interface_value->body.string);
        EXPECT_EQ(server_value, "zmq_tcp");

        // Calling getInterfaceValue for subscriber and client
        result = setenv("AppName", "TestSubClient", 1);
        ASSERT_EQ(0, result);
        cfg_mgr = new ConfigMgr();

        sub_cfg = cfg_mgr->getSubscriberByIndex(0);
        interface_value = sub_cfg->getInterfaceValue("Name");
        cout << "Subscriber's interface value is : " << interface_value->body.string << endl;
        string sub_value(interface_value->body.string);
        EXPECT_EQ(sub_value, "default");

        client_cfg = cfg_mgr->getClientByIndex(0);
        interface_value = client_cfg->getInterfaceValue("Type");
        cout << "Client's interface value is : " << interface_value->body.string << endl;
        string client_value(interface_value->body.string);
        EXPECT_EQ(client_value, "zmq_tcp");
    }

    cout << " =========== End Of getInterfaceValue() testcase ===========" << endl;
}

TEST(ConfigManagerTest, getTopics) {
    cout << "Test Case: getTopics() API\n";

    int result;
    ConfigMgr* cfg_mgr = NULL;
    PublisherCfg* pub_cfg = NULL;
    ServerCfg* server_cfg = NULL;
    SubscriberCfg* sub_cfg = NULL;
    ClientCfg* client_cfg = NULL;
    config_t* config = NULL;
    vector<string> topics;
    vector<string> subTopicsStar;

    for(int i = 0; i < 2; i++){
        // Calling getTopics for Publisher
        result = setenv("AppName", "TestPubServer", 1);
        ASSERT_EQ(0, result);
        cfg_mgr = new ConfigMgr();

        pub_cfg = cfg_mgr->getPublisherByIndex(0);
        topics = pub_cfg->getTopics();
        for (int i = 0; i < topics.size(); i++) {
            cout << "Publisher's topic : "<< topics[i] << endl;
        }
        EXPECT_EQ(topics[0], "camera1_stream");

        // Calling getTopics for Subscriber
        result = setenv("AppName", "TestSubClient", 1);
        ASSERT_EQ(0, result);
        cfg_mgr = new ConfigMgr();

        sub_cfg = cfg_mgr->getSubscriberByIndex(0);
        topics = sub_cfg->getTopics();
        for (int i = 0; i < topics.size(); i++) {
            cout << "Subscriber's topic : "<< topics[i] << endl;
        }

        EXPECT_EQ(topics[0], "camera1_stream");

        // Testing for Topics when it is "*"
        cout << "Testing when topics is \"*\" " << endl;
        subTopicsStar.push_back("*");
        bool topicsSet = sub_cfg->setTopics(subTopicsStar);
        ASSERT_EQ(true, topicsSet);

        topics = sub_cfg->getTopics();
        for (int i = 0; i < topics.size(); i++) {
            cout << "Subscriber's topic : "<< topics[i] << endl;
            EXPECT_EQ(topics[i], "");
        }
        subTopicsStar.clear();
    }

    cout << " =========== End Of getTopics() testcase ===========" << endl;
}

TEST(ConfigManagerTest, setTopics) {
    cout << "Test Case: setTopics() API\n";

    string str_i;
    string topic_value;
    int result;
    ConfigMgr* cfg_mgr = NULL;
    PublisherCfg* pub_cfg = NULL;
    ServerCfg* server_cfg = NULL;
    SubscriberCfg* sub_cfg = NULL;
    ClientCfg* client_cfg = NULL;
    config_t* config = NULL;
    vector<string> pubTopicsList;
    vector<string> subTopicsList;

    for(int i = 0; i < 2; i++){
        // Calling setTopics for publisher
        result = setenv("AppName", "TestPubServer", 1);
        ASSERT_EQ(0, result);
        cfg_mgr = new ConfigMgr();

        pub_cfg = cfg_mgr->getPublisherByIndex(0);
    
        pubTopicsList.push_back("camera1_stream");
        pubTopicsList.push_back("camera2_stream");
        bool topicsSet = pub_cfg->setTopics(pubTopicsList);
        ASSERT_EQ(true, topicsSet);
        pubTopicsList.clear();

        // Calling setTopics for subscriber
        result = setenv("AppName", "TestSubClient", 1);
        ASSERT_EQ(0, result);
        cfg_mgr = new ConfigMgr();

        sub_cfg = cfg_mgr->getSubscriberByIndex(0);

        subTopicsList.push_back("cam1_stream");
        subTopicsList.push_back("cam2_stream");
        topicsSet = sub_cfg->setTopics(subTopicsList);
        ASSERT_EQ(true, topicsSet);
        subTopicsList.clear();
    }

    cout << " =========== End Of setTopics() testcase ===========" << endl;
}

TEST(ConfigManagerTest, allowedClients) {
    cout << "Test Case: allowedClients()\n";
    string str_i;
    string client_value;
    int result;
    ConfigMgr* cfg_mgr = NULL;
    PublisherCfg* pub_cfg = NULL;
    ServerCfg* server_cfg = NULL;
    SubscriberCfg* sub_cfg = NULL;
    ClientCfg* client_cfg = NULL;
    config_t* config = NULL;
    vector<string> allowed_clients;
    vector<string> pub_clients;
    vector<string> serv_clients;

    for(int i = 0; i < 2; i++){
        // Calling allowed clients for Publisher and Server
        result = setenv("AppName", "TestPubServer", 1);
        ASSERT_EQ(0, result);
        cfg_mgr = new ConfigMgr();
    
        pub_cfg = cfg_mgr->getPublisherByIndex(0);

        allowed_clients.push_back("TestSubClient");
        allowed_clients.push_back("Visualizer");
        allowed_clients.push_back("WebVisualizer");
        allowed_clients.push_back("TLSRemoteAgent");
        allowed_clients.push_back("RestDataExport");

        pub_clients = pub_cfg->getAllowedClients();
        for (int i = 1; i < pub_clients.size(); i++) {
            cout << "Allowed Clients : "<< pub_clients[i] << endl;
            EXPECT_EQ(pub_clients[i], allowed_clients[i]);
        }
        pub_clients.clear();

        server_cfg = cfg_mgr->getServerByIndex(0);
        serv_clients = server_cfg->getAllowedClients();
        for (int i = 0; i < serv_clients.size(); i++) {
            cout << "Server Allowed Clients : "<< serv_clients[i] << endl;
            EXPECT_EQ(serv_clients[i], "*");
        }
        serv_clients.clear();
    }

    cout << " =========== End Of allowed_clients() testcase ===========" << endl;
}

TEST(ConfigManagerTest, getConfigValue) {
    cout << "Test Case: getConfigValue()\n";
    
    int max_workers;
    ConfigMgr* cfg_mgr = new ConfigMgr();
    // getAppConfig test
    AppCfg* app_cfg = cfg_mgr->getAppConfig();
    EXPECT_NE(app_cfg, nullptr);

    config_value_t* app_config = NULL;
    for(int i = 0; i < 2; i++){
        // getConfigValue for integer
        app_config = app_cfg->getConfigValue("max_workers");
        EXPECT_EQ(app_config->type, CVT_INTEGER);
        
        max_workers = app_config->body.integer;
        EXPECT_EQ(max_workers, 4);
    }

    // getConfigValue for json structure
    app_config = app_cfg->getConfigValue("ingestor");
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
    app_config = app_cfg->getConfigValue("udfs");
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
