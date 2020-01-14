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
 * @file
 * @brief Env Config implementation
 */

#include "eis/config_manager/env_config.h"
#include <safe_str_lib.h>

#define SIZE 100
#define PUB "pub"
#define SUB "sub"
#define CLIENTS_ENV "Clients"
#define PUBTOPICS_ENV "PubTopics"
#define SUBTOPICS_ENV "SubTopics"
#define DEV_MODE_ENV "DEV_MODE"
#define APPNAME_ENV "AppName"
#define ZMQ_RECV_HWM_ENV "ZMQ_RECV_HWM"
#define CFG "_cfg"
#define SOCKET_FILE "socket_file"

// Forward declaration
void trim(char* str_value);

static char** get_topics_from_env(const char* topic_type) {
    char* topic_list = NULL;
    char* individual_topic = NULL;
    char* topics_env = NULL;
    if(!strcmp(topic_type, PUB)){
        topics_env = PUBTOPICS_ENV;
    }
    else if(!strcmp(topic_type, SUB)){
        topics_env = SUBTOPICS_ENV;
    } else {
        LOG_ERROR("topic type: %s is not supported", topic_type);
        return NULL;
    }

    topic_list = getenv(topics_env);
    if(topic_list == NULL) {
        LOG_ERROR("%s env doesn't exist", topics_env);
        return NULL;
    }

    char **topics = (char **)calloc(SIZE, sizeof(char*));
    if(topics == NULL) {
        LOG_ERROR_0("Calloc failed");
        return NULL;
    }
    int j=0;
    while((individual_topic = strtok_r(topic_list, ",", &topic_list))) {
        topics[j] = individual_topic;
        j++;
    }
    topics[j] = NULL;
    return topics;
 }

 static size_t get_topics_count(char* topics[]) {
    size_t i=0;
    size_t topic_count=0;
    while(topics[i++] != NULL) {
        ++topic_count;
    }
    return topic_count;
 }

static config_t* get_messagebus_config(const config_mgr_t* configmgr, char* topic[], size_t num_of_topics, const char* topic_type){
    char* topic_cfg = NULL;
    char* mode_address[SIZE];
    char* host_port[SIZE];
    char* data = NULL;
    char* mode = NULL;
    char* address = NULL;
    char* host = NULL;
    char* port = NULL;
    char* actual_topic = NULL;
    char* publisher = NULL;
    int i;
    int j;

    mode_address[2] = NULL;
    char* dev_mode_env = getenv(DEV_MODE_ENV);
    if(dev_mode_env == NULL) {
        LOG_ERROR("%s env doesn't exist", DEV_MODE_ENV);
        goto err;
    }
    const char* app_name_env = getenv(APPNAME_ENV);
    if(app_name_env == NULL) {
        LOG_ERROR("%s env doesn't exist", APPNAME_ENV);
        goto err;
    }

    bool dev_mode = false;
    if(!strcmp(dev_mode_env,"true")){
        dev_mode = true;
    } else if(!strcmp(dev_mode_env,"false")){
        dev_mode = false;
    } else {
        LOG_WARN("%s env is not set to true or false, so using false as default", DEV_MODE_ENV);
    }

    char publisher_topic[SIZE];
    int ret = 0;

    if (!strcmp(topic_type,SUB)){
        char* individual_topic;
        ret = strncpy_s(publisher_topic, SIZE, topic[0], strlen(topic[0]));
        if(ret != 0) {
            LOG_ERROR("String copy failed (errno: %d)", ret);
            goto err;
        }
        char* temp = publisher_topic;
        const char* pub_topic[SIZE];
        j=0;
        while((individual_topic = strtok_r(temp, "/", &temp))) {
            pub_topic[j] = individual_topic;
            j++;
        }

        if (j == 1 || j > 2) {
            LOG_ERROR("sub topic should be of the format [AppName]/[stream_name]");
            goto err;
        }
        actual_topic = (char*) malloc(strlen(pub_topic[1]) + 1);
        if(actual_topic == NULL) {
            LOG_ERROR_0("malloc failed for pub_topic");
            goto err;
        }
        memset(actual_topic, 0, strlen(pub_topic[1]) + 1);
        publisher = (char*) malloc(strlen(pub_topic[0]) + 1);
        if(publisher == NULL) {
            LOG_ERROR_0("malloc failed for pub_topic");
            goto err;
        }
        memset(publisher, 0, strlen(pub_topic[0]) + 1);
        ret = strncpy_s(actual_topic, SIZE, pub_topic[1], strlen(pub_topic[1]));
        if(ret != 0) {
            LOG_ERROR("String copy failed (errno: %d)", ret);
            goto err;
        }
        LOG_DEBUG("Actual topic: %s", actual_topic);
        ret = strncpy_s(publisher, SIZE, pub_topic[0],strlen(pub_topic[0]));
        if(ret != 0) {
            LOG_ERROR("String copy failed (errno: %d)", ret);
            goto err;
        }
        LOG_DEBUG("publisher: %s", publisher);
        ret = strncpy_s(publisher_topic, SIZE, actual_topic, strlen(actual_topic));
        if(ret != 0) {
            LOG_ERROR("String copy failed (errno: %d)", ret);
            goto err;
        }
        ret = strncat_s(publisher_topic, SIZE, CFG, strlen(CFG));
        if(ret != 0) {
            LOG_ERROR("String concatenation failed (errno: %d)", ret);
            goto err;
        }
     } else if(!strcmp(topic_type,PUB)){
        ret = strncpy_s(publisher_topic,SIZE, topic[0], strlen(topic[0]));
        if(ret != 0) {
            LOG_ERROR("String copy failed (errno: %d)", ret);
            goto err;
        }
        ret = strncat_s(publisher_topic, SIZE, CFG, strlen(CFG));
        if(ret != 0) {
            LOG_ERROR("String concatenation failed (errno: %d)", ret);
            goto err;
        }
    } else {
        LOG_ERROR("topic type: %s is not supported", topic_type);
        goto err;
    }
    LOG_DEBUG("publisher_topic: %s", publisher_topic);
    topic_cfg = getenv(publisher_topic);
    if(topic_cfg == NULL) {
        LOG_ERROR("%s env doens't exist", publisher_topic);
        goto err;
    }
    i=0;

    while((data = strtok_r(topic_cfg, ",", &topic_cfg))) {
        mode_address[i] = data;
        i++;
    }

    if(mode_address[2] == NULL) {
        LOG_DEBUG_0("socket file not explicitly given by application");
    }
    else {
        LOG_DEBUG_0("socket file explicitly given by application");
    }

    mode = mode_address[0];
    trim(mode);
    address = mode_address[1];
    trim(address);
    cJSON* json = cJSON_CreateObject();
    cJSON_AddStringToObject(json, "type", mode);

    char* zmq_recv_hwm = getenv(ZMQ_RECV_HWM_ENV);
    if(zmq_recv_hwm != NULL){
        cJSON_AddNumberToObject(json, "ZMQ_RECV_HWM", atoi(zmq_recv_hwm));
    } else {
        LOG_DEBUG("%s env is not set", ZMQ_RECV_HWM_ENV);
    }

    if(!strcmp(mode,"zmq_tcp")) {
        i=0;
        while((data = strtok_r(address, ":", &address))) {
            host_port[i] = data;
            i++;
        }

        host = host_port[0];
        trim(host);
        port = host_port[1];
        trim(port);

        __int64_t i_port = atoi(port);
        char* publicKey = (char*) malloc(SIZE);
        if(!strcmp(topic_type,PUB)){
            cJSON* zmq_tcp_publish = cJSON_CreateObject();
            cJSON_AddItemToObject(json, "zmq_tcp_publish", zmq_tcp_publish);
            cJSON_AddStringToObject(zmq_tcp_publish, "host", host);
            cJSON_AddNumberToObject(zmq_tcp_publish, "port", i_port);

            if(!dev_mode){
                i = 0;
                const char* allowed_clients[SIZE];
                char* clients = getenv(CLIENTS_ENV);
                if(clients == NULL) {
                    LOG_ERROR("%s env doens't exist", CLIENTS_ENV);
                    goto err;
                }

                while((data = strtok_r(clients, ",", &clients))) {
                    allowed_clients[i] = data;
                    i++;
                }

                cJSON* all_clients = cJSON_CreateArray();
                for(j=0; j<i; j++){
                    ret = strncpy_s(publicKey, SIZE, "/Publickeys/", strlen("/Publickeys/"));
                    if(ret != 0) {
                        LOG_ERROR("String copy failed (errno: %d)", ret);
                        goto err;
                    }
                    ret = strncat_s(publicKey, SIZE, allowed_clients[j], strlen(allowed_clients[j]));
                    if(ret != 0) {
                        LOG_ERROR("String concatenation failed (errno: %d)", ret);
                        goto err;
                    }
                    char* client_pub_key = configmgr->get_config(publicKey);

                    if(strcmp(client_pub_key,"")){
                        cJSON_AddItemToArray(all_clients, cJSON_CreateString(client_pub_key));
                    }
                }

                char* app_private_key = (char*) calloc(SIZE, sizeof(char));
                if(app_private_key == NULL) {
                    goto err;
                }
                app_private_key[0] = '/';
                ret = strncat_s(app_private_key, SIZE, app_name_env, strlen(app_name_env));
                if(ret != 0) {
                    LOG_ERROR("String concatenation failed (errno: %d)", ret);
                    goto err;
                }
                ret = strncat_s(app_private_key, SIZE, "/private_key", strlen("/private_key"));
                if(ret != 0) {
                    LOG_ERROR("String concatenation failed (errno: %d)", ret);
                    goto err;
                }

                const char* server_secret_key = configmgr->get_config(app_private_key);

                cJSON_AddStringToObject(zmq_tcp_publish, "server_secret_key",server_secret_key);
                cJSON_AddItemToObject(json, "allowed_clients", all_clients);
            }

        }
        else if(!strcmp(topic_type,SUB)){
            cJSON* pub_sub_topic = cJSON_CreateObject();
            cJSON_AddItemToObject(json, actual_topic, pub_sub_topic);
            cJSON_AddStringToObject(pub_sub_topic, "host", host);
            cJSON_AddNumberToObject(pub_sub_topic, "port", i_port);
            free(actual_topic);

            if(!dev_mode){
                ret = strncpy_s(publicKey, SIZE, "/Publickeys/", strlen("/Publickeys/"));
                if(ret != 0) {
                    LOG_ERROR("String copy failed (errno: %d)", ret);
                    goto err;
                }
                ret = strncat_s(publicKey,SIZE, publisher, strlen(publisher));
                if(ret != 0) {
                    LOG_ERROR("String concatenation failed (errno: %d)", ret);
                    goto err;
                }

                const char* server_pub_key = configmgr->get_config(publicKey);
                cJSON_AddStringToObject(pub_sub_topic, "server_public_key",server_pub_key);
                ret = strncpy_s(publicKey, SIZE, "/Publickeys/", strlen("/Publickeys/"));
                if(ret != 0) {
                    LOG_ERROR("String copy failed (errno: %d)", ret);
                    goto err;
                }
                ret = strncat_s(publicKey, SIZE, app_name_env, strlen(app_name_env));
                if(ret != 0) {
                    LOG_ERROR("String concatenation failed (errno: %d)", ret);
                    goto err;
                }
                free(publisher);

                const char* client_pub_key = configmgr->get_config(publicKey);
                cJSON_AddStringToObject(pub_sub_topic, "client_public_key",client_pub_key);

                char* app_private_key = (char*) calloc(SIZE, sizeof(char));
                if(app_private_key == NULL) {
                    LOG_ERROR_0("Calloc failed");
                    goto err;
                }
                app_private_key[0] = '/';
                ret = strncat_s(app_private_key,SIZE,app_name_env, strlen(app_name_env));
                if(ret != 0) {
                    LOG_ERROR("String concatenation failed (errno: %d)", ret);
                    goto err;
                }
                ret = strncat_s(app_private_key, SIZE, "/private_key", strlen("/private_key"));
                if(ret != 0) {
                    LOG_ERROR("String concatenation failed (errno: %d)", ret);
                    goto err;
                }

                const char* client_secret_key = configmgr->get_config(app_private_key);
                cJSON_AddStringToObject(pub_sub_topic, "client_secret_key",client_secret_key);
            }

        } else {
            LOG_ERROR_0("topic is neither PUB nor SUB");
            goto err;
        }

    } else if(!strcmp(mode,"zmq_ipc")) {
        char* socket_file = NULL;
        if(mode_address[2] == NULL) {
            LOG_DEBUG_0("Socket file explicitly not given by application ");
        } else {
            LOG_DEBUG("Socket file given by the application is = %s",mode_address[2]);
            socket_file = mode_address[2];
        }
        cJSON_AddStringToObject(json, "socket_dir", mode_address[1]);

        // If socket_file is given by the application
        if(socket_file != NULL) {
            if(!strcmp(topic_type,PUB)) {
                LOG_DEBUG_0(" topic type is Pub");
                for (size_t i=0; i < num_of_topics; ++i) {
                    cJSON* socket_file_obj = cJSON_CreateObject();
                    cJSON_AddItemToObject(json, topic[i], socket_file_obj);
                    cJSON_AddStringToObject(socket_file_obj, SOCKET_FILE, socket_file);
                }
            } else if(!strcmp(topic_type,SUB)) {
                LOG_DEBUG_0(" topic type is Sub");
                cJSON* socket_file_obj = cJSON_CreateObject();
                cJSON_AddItemToObject(json, actual_topic, socket_file_obj);
                cJSON_AddStringToObject(socket_file_obj, SOCKET_FILE, socket_file);
            }
        }

    } else {
        LOG_ERROR("mode: %s is not supported", mode);
        return NULL;
    }

    char* config_value = cJSON_Print(json);
    LOG_DEBUG("Env Config is : %s \n", config_value);

    config_t* config = config_new(
            (void*) json, free_json, get_config_value);
    if(config == NULL) {
        LOG_ERROR_0("Failed to initialize configuration object");
        return NULL;
    }
    return config;

err:
    if(actual_topic != NULL) {
        free(actual_topic);
    }
    if(publisher != NULL) {
        free(publisher);
    }

    return NULL;
}

void trim(char* str_value) {
    int index;
    int i;

    // Trimming leading white spaces
    index = 0;
    while(str_value[index] == ' ' || str_value[index] == '\t' || str_value[index] == '\n'){
        index++;
    }

    i = 0;
    while(str_value[i + index] != '\0'){
        str_value[i] = str_value[i + index];
        i++;
    }
    str_value[i] = '\0'; // Terminate string with NULL

    // Trim trailing white spaces
    i = 0;
    index = -1;
    while(str_value[i] != '\0'){
        if(str_value[i] != ' ' && str_value[i] != '\t' && str_value[i] != '\n'){
            index = i;
        }
        i++;
    }
    // Mark the next character to last non white space character as NULL
    str_value[index + 1] = '\0';
}

void env_config_destroy(env_config_t* env_config){
    if(env_config != NULL)
        free(env_config);
}


env_config_t* env_config_new(){

    env_config_t *env_config = (env_config_t *)malloc(sizeof(env_config_t));
    if(env_config == NULL) {
        return NULL;
    }
    env_config->get_topics_from_env = get_topics_from_env;
    env_config->get_messagebus_config = get_messagebus_config;
    env_config->get_topics_count = get_topics_count;
    env_config->trim = trim;
    return env_config;
}