// Copyright (c) 2019 Intel Corporation.

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.


#include "eis/utils/msgbus_util.h"
#include <cjson/cJSON.h>
#include "eis/utils/json_config.h"
#include "eis/utils/logger.h"

using namespace eis::utils;

#ifdef __cplusplus
extern "C" {
#endif
// prototypes
void free_json(void* ctx);
config_value_t* get_config_value(const void* o, const char* key);
#ifdef __cplusplus
}
#endif

std::vector<std::string> MsgBusUtil::get_topics_from_env(const std::string& topic_type) {
    std::vector<std::string> topics;
    try {
        std::string topic_list;
        if(topic_type == "pub"){
            topic_list = getenv("PubTopics");
        }
        else if(topic_type == "sub"){
            topic_list = getenv("SubTopics");
        }
        tokenize(topic_list, topics, ',');
        return topics;
    } catch(const std::exception ex){
        LOG_ERROR("Exception occurred: %s", ex.what());
        return topics;
    }
}

void MsgBusUtil::tokenize(const std::string& tokenizable_data,
                          std::vector<std::string>& tokenized_data,
                          const char delimeter) {
    std::stringstream topic_stream(tokenizable_data);
    std::string data;
    int count = 0;

    // Tokenizing based on delimeter
    while(getline(topic_stream, data, delimeter)) {
        tokenized_data.push_back(data);
        count++;
    }
}


std::string MsgBusUtil::ltrim(const std::string& value) {
    size_t start = value.find_first_not_of(WhiteSpace);
    return (start == std::string::npos) ? "" : value.substr(start);
}


std::string MsgBusUtil::rtrim(const std::string& value) {
    size_t end = value.find_last_not_of(WhiteSpace);
    return (end == std::string::npos) ? "" : value.substr(0, end + 1);
}


std::string MsgBusUtil::trim(const std::string& value) {
    return rtrim(ltrim(value));
}


config_t* MsgBusUtil::get_messagebus_config(std::string& topic,
                                            std::string& topic_type,
                                            std::string& clients) {

    try {
        std::string app_name;
        std::string topics;
        std::string mode;
        std::string address;
        std::string host;
        std::string port;

        std::vector<std::string> mode_address;
        std::vector<std::string> host_port;

        std::string dev_mode_str = getenv("DEV_MODE");
        bool dev_mode;
        if (dev_mode_str == "false") {
            dev_mode = false;
        } else if (dev_mode_str == "true") {
            dev_mode = true;
        }
        app_name = trim(getenv("AppName"));

        topic = topic + "_cfg";
        topics = getenv(topic.c_str());
        tokenize(topics, mode_address, ',');

        mode = trim(mode_address[0]);
        address = trim(mode_address[1]);

        cJSON* json = cJSON_CreateObject();
        cJSON_AddStringToObject(json, "type", mode.c_str());

        if(mode == "zmq_tcp") {
            tokenize(address, host_port, ':');
            host = trim(host_port[0]);
            port = trim(host_port[1]);

            transform(topic_type.begin(), topic_type.end(),
                        topic_type.begin(), ::tolower);

            if(topic_type == "pub") {

                cJSON* zmq_tcp_publish = cJSON_CreateObject();
                cJSON_AddItemToObject(json,"zmq_tcp_publish", zmq_tcp_publish);
                cJSON_AddStringToObject(zmq_tcp_publish, "host", host.c_str());
                cJSON_AddNumberToObject(zmq_tcp_publish, "port", stoi(port));

                if(dev_mode == false) {
                    //TODO : Prod_Mode
                }

            } else if(topic_type == "sub") {
                cJSON* pub_sub_topic = cJSON_CreateObject();
                cJSON_AddItemToObject(json, topic.c_str(), pub_sub_topic);
                cJSON_AddStringToObject(pub_sub_topic, "host", host.c_str());
                cJSON_AddNumberToObject(pub_sub_topic, "port", stoi(port));

                if(dev_mode == false) {
                    //TODO : Prod_Mode
                }

            }

        } else if(mode == "zmq_ipc") {
            cJSON_AddStringToObject(json, "socket_dir", address.c_str());
        }

        // Create configuration object
        config_t* config = config_new(
                (void*) json, free_json, get_config_value);
        if(config == NULL) {
            LOG_ERROR_0("Failed to initialize configuration object");
            return NULL;
        }

        return config;
    } catch (const std::exception& ex) {
        LOG_ERROR("Exception occurred: %s", ex.what());
        return NULL;
    }
}
