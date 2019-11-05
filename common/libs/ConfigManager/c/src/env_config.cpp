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

/**
 * @brief EIS Env configuration interface implementation
 */

#include <string>
#include "eis/config_manager/env_config.h"
#include <cjson/cJSON.h>
#include "eis/utils/json_config.h"
#include "eis/utils/logger.h"


using namespace eis::config_manager;

EnvConfig::EnvConfig() {
    m_app_name = getenv("AppName");
    std::string dev_mode_str = getenv("DEV_MODE");

    if (dev_mode_str == "false") {
        m_dev_mode = false;
    } else if (dev_mode_str == "true") {
        m_dev_mode = true;
    }
    std::string pub_cert_file = "";
    std::string pri_key_file = "";
    std::string trust_file = "";
    if(!m_dev_mode) {
        pub_cert_file = "/run/secrets/etcd_" + m_app_name + "_cert";
        pri_key_file = "/run/secrets/etcd_" + m_app_name + "_key";
        trust_file = "/run/secrets/ca_etcd";
    }
    m_config_mgr_config = new config_mgr_config_t;
    m_config_mgr_config->storage_type = "etcd";
    m_config_mgr_config->ca_cert = (char*) trust_file.c_str();
    m_config_mgr_config->cert_file = (char*)pub_cert_file.c_str();
    m_config_mgr_config->key_file = (char*) pri_key_file.c_str();

    m_config_mgr_client = config_mgr_new(m_config_mgr_config);
    if(m_config_mgr_client == NULL) {
        const char* err = "Config manager client creation failed";
        LOG_ERROR("%s", err);
        throw(err);
    }
}

EnvConfig::~EnvConfig() {
    if(m_config_mgr_config) {
        delete m_config_mgr_config;
    }
    if(m_config_mgr_client) {
        delete m_config_mgr_client;
    }
}

config_mgr_t* EnvConfig::get_config_mgr_client() {
    return m_config_mgr_client;
}

std::vector<std::string> EnvConfig::get_topics_from_env(const std::string& topic_type) {
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

void EnvConfig::tokenize(const std::string& tokenizable_data,
                          std::vector<std::string>& tokenized_data,
                          const char delimeter) {
    std::stringstream topic_stream(tokenizable_data);
    std::string data;

    // Tokenizing based on delimeter
    while(getline(topic_stream, data, delimeter)) {
        trim(data);
        tokenized_data.push_back(data);
    }
}


std::string EnvConfig::ltrim(const std::string& value) {
    size_t start = value.find_first_not_of(whitespace);
    return (start == std::string::npos) ? "" : value.substr(start);
}


std::string EnvConfig::rtrim(const std::string& value) {
    size_t end = value.find_last_not_of(whitespace);
    return (end == std::string::npos) ? "" : value.substr(0, end + 1);
}


std::string EnvConfig::trim(const std::string& value) {
    return rtrim(ltrim(value));
}


config_t* EnvConfig::get_messagebus_config(std::string& topic,
                                            std::string& topic_type) {

    try {
        std::string topic_cfg;
        std::string mode;
        std::string address;
        std::string host;
        std::string port;

        std::vector<std::string> mode_address;
        std::vector<std::string> host_port;
        std::vector<std::string> pub_topic;

        transform(topic_type.begin(), topic_type.end(),
                  topic_type.begin(), ::tolower);
        
        if (topic_type == "sub"){
            tokenize(topic, pub_topic, '/');
            topic = pub_topic[1];
        }

        topic_cfg = topic + "_cfg";
        topic_cfg = getenv(topic_cfg.c_str());
        tokenize(topic_cfg, mode_address, ',');

        mode = trim(mode_address[0]);
        address = trim(mode_address[1]);

        cJSON* json = cJSON_CreateObject();
        if(json == NULL){
            const char* err = "Unable to create JSON Object";
            LOG_ERROR("%s", err);
            throw(err);
        }
        cJSON_AddStringToObject(json, "type", mode.c_str());

        if(mode == "zmq_tcp") {
            tokenize(address, host_port, ':');
            host = trim(host_port[0]);
            port = trim(host_port[1]);
            u_int64_t i_port;
            sscanf(&port[0], "%lu", &i_port);

            if(topic_type == "pub") {

                cJSON* zmq_tcp_publish = cJSON_CreateObject();
                if(zmq_tcp_publish == NULL){
                    const char* err = "Unable to create JSON Object";
                    LOG_ERROR("%s", err);
                    throw(err);
                }
                cJSON_AddItemToObject(json, "zmq_tcp_publish", zmq_tcp_publish);
                cJSON_AddStringToObject(zmq_tcp_publish, "host", host.c_str());
                cJSON_AddNumberToObject(zmq_tcp_publish, "port", i_port);

                if(!m_dev_mode) {

                    std::vector<std::string> allowed_clients;
                    std::string clients = getenv("Clients");
                    tokenize(clients, allowed_clients, ',');

                    cJSON* all_clients = cJSON_CreateArray();
                    if(all_clients == NULL){
                        const char* err = "Unable to create JSON Object";
                        LOG_ERROR("%s", err);
                        throw(err);
                    }
                    for(std::string client : allowed_clients) {
                        std::string client_pub_key = m_config_mgr_client->get_config(&("/Publickeys/" + client)[0]);
                        // TODO: should we assert here
                        if(!client_pub_key.empty()) {
                            cJSON_AddItemToArray(all_clients, cJSON_CreateString(client_pub_key.c_str()));
                        }

                    }
                    // cJSON_AddArrayToObject(json, "allowed_clients", &allowed_clients[0]);
                    const char* server_secret_key = m_config_mgr_client->get_config(&("/" + m_app_name + "/private_key")[0]);
                    cJSON_AddStringToObject(zmq_tcp_publish, "server_secret_key",
                                          server_secret_key);

                }

            } else if(topic_type == "sub") {
                cJSON* pub_sub_topic = cJSON_CreateObject();
                if(pub_sub_topic == NULL){
                    const char* err = "Unable to create JSON Object";
                    LOG_ERROR("%s", err);
                    throw(err);
                }
                cJSON_AddItemToObject(json, topic.c_str(), pub_sub_topic);
                cJSON_AddStringToObject(pub_sub_topic, "host", host.c_str());
                cJSON_AddNumberToObject(pub_sub_topic, "port", i_port);

                if(!m_dev_mode) {

                    const char* server_pub_key = m_config_mgr_client->get_config(
                        &("/Publickeys/" + m_app_name)[0]
                    );
                    cJSON_AddStringToObject(pub_sub_topic, "server_public_key",
                                            server_pub_key);

                    const char* client_pub_key = m_config_mgr_client->get_config(
                        &("/Publickeys/" + m_app_name)[0]
                    );
                    cJSON_AddStringToObject(pub_sub_topic, "server_public_key",
                                            server_pub_key);

                    const char* client_secret_key = m_config_mgr_client->get_config(
                        &("/" + m_app_name + "/private_key")[0]
                    );
                    cJSON_AddStringToObject(pub_sub_topic, "server_public_key",
                                            client_secret_key);

                    const char* server_secret_key = m_config_mgr_client->get_config(
                        &("/" + m_app_name + "/private_key")[0]);
                    cJSON_AddStringToObject(pub_sub_topic, "server_secret_key",
                                          server_secret_key);
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
    } catch(const std::exception& ex) {
        LOG_ERROR("Exception occurred: %s", ex.what());
        return NULL;
    }
}
