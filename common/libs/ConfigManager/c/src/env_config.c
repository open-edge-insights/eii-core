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

#define SIZE 100
#define PUB "pub"
#define SUB "sub"

 static char** get_topics_from_env(const char* topic_type){
    char* topic_list = NULL;
    char* individual_topic = NULL;
    char **topics = (char **)malloc(SIZE * sizeof(char*));

    if(!strcmp(topic_type, PUB)){
        topic_list = getenv("PubTopics");
    }
    else if(!strcmp(topic_type, SUB)){
        topic_list = getenv("SubTopics");
    }

    int j=0;
    while (individual_topic = strtok_r(topic_list, ",", &topic_list)){
        topics[j] = individual_topic;
        LOG_DEBUG("Topics are : %s \n", topics[j]);
        j++;
    }
    topics[j] = NULL;
    return topics;
 }


static config_t* get_messagebus_config(config_mgr_t* configmgr, const char *topics, const char* topic_type){
    
    char* topic_cfg = NULL;
    char* mode_address[SIZE];
    char* host_port[SIZE];
    char* data = NULL;
    char* mode = NULL;
    char* address = NULL;
    char* host = NULL;
    char* port = NULL;
    int i;
    int j;

    char* dev_mode=getenv("DEV_MODE");
    const char* m_app_name = getenv("AppName");
    const char cfg[]= "_cfg";

    bool m_dev_mode;
    if(!strcmp(dev_mode,"true")){
        m_dev_mode=true;
    }else if(!strcmp(dev_mode,"false")){
        m_dev_mode=false;
    }
    
    char* topic = (char*) malloc(sizeof(char*));
    const char* publisher = (char*) malloc(sizeof(char*));
    char publisher_topic[SIZE];
    int ret = 0;

    if (!strcmp(topic_type,SUB)){

        char* individual_topic;
        ret = strncpy_s(publisher_topic, SIZE,topics, strlen(topics));
        if(ret != 0) {
            LOG_ERROR("String copy failed (errno: %d)", ret);
            goto err;
        } 
        char* temp = publisher_topic;
        const char* pub_topic[SIZE];
        j=0;
        while (individual_topic = strtok_r(temp, "/", &temp)){
            pub_topic[j] = individual_topic;            
            j++;
        }
        ret = strncpy_s(topic, SIZE, pub_topic[1], strlen(pub_topic[1]));
        if(ret != 0) {
            LOG_ERROR("String copy failed (errno: %d)", ret);
            goto err;
        } 
        ret = strncpy_s(publisher, SIZE, pub_topic[0],strlen(pub_topic[0]));
        if(ret != 0) {
            LOG_ERROR("String copy failed (errno: %d)", ret);
            goto err;
        } 
        ret = strncpy_s(publisher_topic, SIZE, pub_topic[1], strlen(pub_topic[1]));
        if(ret != 0) {
            LOG_ERROR("String copy failed (errno: %d)", ret);
            goto err;
        } 
        ret = strncat_s(publisher_topic, SIZE, cfg, strlen(cfg));
        if(ret != 0) {
            LOG_ERROR("String concatenation failed (errno: %d)", ret);
            goto err;
        } 
        topic_cfg = getenv(publisher_topic);
     }
     else if(!strcmp(topic_type,PUB)){
        ret = strncpy_s(publisher_topic,SIZE, topics,strlen(topics));
        if(ret != 0) {
            LOG_ERROR("String copy failed (errno: %d)", ret);
            goto err;
        } 
        ret = strncat_s(publisher_topic, SIZE, cfg, strlen(cfg));
        if(ret != 0) {
            LOG_ERROR("String concatenation failed (errno: %d)", ret);
            goto err;
        } 
        topic_cfg = getenv(publisher_topic);
    }
    
    i=0;
    while (data = strtok_r(topic_cfg, ",", &topic_cfg)){
        mode_address[i] = data;
        i++;
    }

    mode = mode_address[0];
    trim(mode);
    address = mode_address[1];
    trim(address);
    cJSON* json = cJSON_CreateObject();
    cJSON_AddStringToObject(json, "type", mode);

    char* zmq_recv_hwm = getenv("ZMQ_RECV_HWM");
    if(zmq_recv_hwm != NULL){
        cJSON_AddNumberToObject(json, "ZMQ_RECV_HWM", atoi(zmq_recv_hwm));
    }

    if(!strcmp(mode,"zmq_tcp")) {
        i=0;
        while (data = strtok_r(address, ":", &address)){
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

            if(!m_dev_mode){
                i = 0;
                const char* allowed_clients[SIZE];
                char* clients = getenv("Clients");
                while (data = strtok_r(clients, ",", &clients)){
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
                    char* all_clients_print= cJSON_Print(all_clients);
                }

                char* app_private_key = (char*) calloc(SIZE, sizeof(char));
                app_private_key[0] = '/';
                ret = strncat_s(app_private_key, SIZE, m_app_name, strlen(m_app_name));
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
            cJSON_AddItemToObject(json, topic, pub_sub_topic);
            cJSON_AddStringToObject(pub_sub_topic, "host", host);
            cJSON_AddNumberToObject(pub_sub_topic, "port", i_port);
            free(topic);

            if(!m_dev_mode){                
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
                ret = strncat_s(publicKey, SIZE, m_app_name, strlen(m_app_name));
                if(ret != 0) {
                    LOG_ERROR("String concatenation failed (errno: %d)", ret);
                    goto err;
                } 
                free(publisher);
                
                const char* client_pub_key = configmgr->get_config(publicKey);
                cJSON_AddStringToObject(pub_sub_topic, "client_public_key",client_pub_key);
                
                char* app_private_key = (char*) calloc(SIZE, sizeof(char));
                app_private_key[0] = '/';
                ret = strncat_s(app_private_key,SIZE,m_app_name, strlen(m_app_name));
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

        }
        
    }
    else if(!strcmp(mode,"zmq_ipc")){
        cJSON_AddStringToObject(json, "socket_dir", mode_address[1]);

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
    free(topic);
    free(publisher);
    return NULL;
}


void trim(char* str_value)
{
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
    env_config->get_topics_from_env = get_topics_from_env;
    env_config->get_messagebus_config = get_messagebus_config;
    env_config->trim = trim;
    return env_config;
}