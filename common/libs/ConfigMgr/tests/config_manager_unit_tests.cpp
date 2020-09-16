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

using namespace eis::config_manager;


TEST(ConfigManagerTest, set_required_certs) {
    std::cout << "Test Case: set_required_certs()\n";

    // In a dockerized environment,
    // these variables are set in environment
    int result = setenv("DEV_MODE", "FALSE", 1);
    ASSERT_EQ(0, result);
    // Replace 2nd parameter with path to certs
    result = setenv("", 1);
    ASSERT_EQ(0, result);
    result = setenv("", 1);
    ASSERT_EQ(0, result);
    result = setenv("", 1);
    ASSERT_EQ(0, result);

    // Uncomment below lines to test DEV mode
    // setenv("DEV_MODE", "TRUE", 1);
    // setenv("CONFIGMGR_CERT", "", 1);
    // setenv("CONFIGMGR_KEY", "", 1);
    // setenv("CONFIGMGR_CACERT", "", 1);
}

TEST(ConfigManagerTest, publisher_test) {
    std::cout << "Test Case: publisher_test()\n";

    int result = setenv("AppName", "VideoIngestion", 1);
    ASSERT_EQ(0, result);
    ConfigMgr* pub_ch = new ConfigMgr();
    EXPECT_NE(pub_ch, nullptr);
    PublisherCfg* pub_ctx = pub_ch->getPublisherByName("default");
    EXPECT_NE(pub_ctx, nullptr);
    config_t* pub_config = pub_ctx->getMsgBusConfig();
    EXPECT_NE(pub_config, nullptr);

    std::string endpoint = pub_ctx->getEndpoint();
    std::cout << endpoint << std::endl;

    std::vector<std::string> topics = pub_ctx->getTopics();
    for (int i = 0; i < topics.size(); i++) {
        std::cout << topics[i] << std::endl;
    }

    std::vector<std::string> clients = pub_ctx->getAllowedClients();
    for (int i = 0; i < clients.size(); i++) {
        std::cout << clients[i] << std::endl;
    }

    std::vector<std::string> newTopicsList;
    newTopicsList.push_back("camera5_stream");
    newTopicsList.push_back("camera6_stream");
    bool topicsSet = pub_ctx->setTopics(newTopicsList);
    ASSERT_EQ(true, topicsSet);

    void* ctx = msgbus_initialize(pub_config);
    EXPECT_NE(ctx, nullptr);
    msgbus_destroy(ctx);
}

TEST(ConfigManagerTest, subscriber_test) {
    std::cout << "Test Case: subscriber_test()\n";

    int result = setenv("AppName", "VideoAnalytics", 1);
    ASSERT_EQ(0, result);
    ConfigMgr* sub_ch = new ConfigMgr();
    EXPECT_NE(sub_ch, nullptr);
    SubscriberCfg* sub_ctx = sub_ch->getSubscriberByName("VideoData");
    EXPECT_NE(sub_ctx, nullptr);
    config_t* sub_config = sub_ctx->getMsgBusConfig();
    EXPECT_NE(sub_config, nullptr);

    std::string endpoint = sub_ctx->getEndpoint();
    std::cout << endpoint << std::endl;

    std::vector<std::string> topics = sub_ctx->getTopics();
    for (int i = 0; i < topics.size(); i++) {
        std::cout << topics[i] << std::endl;
    }

    std::vector<std::string> newTopicsList;
    newTopicsList.push_back("camera5_stream");
    newTopicsList.push_back("camera6_stream");
    bool topicsSet = sub_ctx->setTopics(newTopicsList);
    ASSERT_EQ(true, topicsSet);

    void* ctx = msgbus_initialize(sub_config);
    EXPECT_NE(ctx, nullptr);
    msgbus_destroy(ctx);
}

TEST(ConfigManagerTest, server_test) {
    std::cout << "Test Case: server_test()\n";

    int result = setenv("AppName", "VideoIngestion", 1);
    ASSERT_EQ(0, result);
    ConfigMgr* config_mgr = new ConfigMgr();
    EXPECT_NE(config_mgr, nullptr);
    ServerCfg* server_ctx = config_mgr->getServerByName("sample_server");
    EXPECT_NE(server_ctx, nullptr);
    config_t* config = server_ctx->getMsgBusConfig();
    EXPECT_NE(config, nullptr);

    std::string endpoint = server_ctx->getEndpoint();
    std::cout << endpoint << std::endl;

    std::vector<std::string> clients = server_ctx->getAllowedClients();
    for (int i = 0; i < clients.size(); i++) {
        std::cout << clients[i] << std::endl;
    }

    void* ctx = msgbus_initialize(config);
    EXPECT_NE(ctx, nullptr);
    msgbus_destroy(ctx);
}

TEST(ConfigManagerTest, client_test) {
    std::cout << "Test Case: client_test()\n";

    int result = setenv("AppName", "VideoAnalytics", 1);
    ASSERT_EQ(0, result);
    ConfigMgr* ch = new ConfigMgr();
    EXPECT_NE(ch, nullptr);
    ClientCfg* client_ctx = ch->getClientByName("default");
    EXPECT_NE(client_ctx, nullptr);
    config_t* config = client_ctx->getMsgBusConfig();
    EXPECT_NE(config, nullptr);

    std::string endpoint = client_ctx->getEndpoint();
    std::cout << endpoint << std::endl;

    void* ctx = msgbus_initialize(config);
    EXPECT_NE(ctx, nullptr);
    msgbus_destroy(ctx);
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
