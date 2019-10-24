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
// FROM,OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.

#include <gtest/gtest.h>
#include <stdlib.h>
#include <sstream>
#include "eis/utils/msgbus_util.h"
#include "eis/utils/logger.h"

using namespace eis::utils;

TEST(msgbus_util_tests, get_topics_from_env) {
	const char* topics_list = "sub_topic1,sub_topic2";
	std::vector<std::string> topics{"sub_topic1", "sub_topic2"};
    setenv("SubTopics", topics_list, true);
	MsgBusUtil obj;
	std::vector<std::string> results = obj.get_topics_from_env("sub");
    EXPECT_EQ(topics[0], results[0]);
	EXPECT_EQ(topics[1], results[1]);
}

TEST(msgbus_util_tests, get_messagebus_config) {
	std::string topic_type = "sub";
	std::string topic = "camera1_stream_results";
	std::string result = topic + "_cfg";

	// setting required envs
	setenv((char*)&result[0], "zmq_tcp,127.0.0.1:65013", true);
	setenv("DEV_MODE", "true", true);
	setenv("Clients", "Visualizer", true);
	setenv("AppName", "Sample", true);
	MsgBusUtil obj;
	config_t* config = obj.get_messagebus_config(topic, topic_type);
	config_value_t* value = config->get_config_value(config->cfg, "type");
	ASSERT_EQ(value->type, CVT_STRING);
	ASSERT_STRCASEEQ(value->body.string, "zmq_tcp");
    //TODO: Add additional checks
}