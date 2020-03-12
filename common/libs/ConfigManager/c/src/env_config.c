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
// FROM,OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.


/**
 * @file
 * @brief Env Config implementation
 */

#include "eis/config_manager/env_config.h"
#include <safe_str_lib.h>

#define SIZE 500
#define PUB "pub"
#define SUB "sub"
#define CLIENTS_ENV "Clients"
#define PUBTOPICS_ENV "PubTopics"
#define SUBTOPICS_ENV "SubTopics"
#define DEV_MODE_ENV "DEV_MODE"
#define RequestEP "RequestEP"
#define APPNAME_ENV "AppName"
#define ZMQ_RECV_HWM_ENV "ZMQ_RECV_HWM"
#define CFG "_cfg"
#define SOCKET_FILE "socket_file"
#define SERVER "Server"
#define CLIENT "client"

// Forward declaration
void trim(char* str_value);

static void free_mem(void* arr[]){
    int k = 0;
    while(arr[k] != NULL){
        free(arr[k]);
        k++;
    }
    free(arr);
}

static char** get_topics_from_env(const char* topic_type) {
    char* topic_list = NULL;
    char env_topics[SIZE] = {0,};
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
    int ret = 0;
    ret = strncpy_s(env_topics, SIZE, getenv(topics_env), SIZE);
    if(ret != 0) {
        LOG_ERROR("String copy failed (errno: %d): Failed to copy topics_env", ret);
        return NULL;
    }

    topic_list = env_topics;
    if(topic_list == NULL) {
        LOG_ERROR("%s env doesn't exist", topics_env);
        return NULL;
    }

    char **topics = (char **)calloc(SIZE, sizeof(char*));
    if(topics == NULL) {
        LOG_ERROR_0("Calloc failed for topics");
        return NULL;
    }
    int j=0;
    while((individual_topic = strtok_r(topic_list, ",", &topic_list))) {
        if(topics[j] == NULL){
            topics[j] = (char*) malloc(strlen(individual_topic));
        }
        if(topics[j] == NULL){
            LOG_ERROR_0("Malloc failed for individual topics");
            free_mem(topics);
            return NULL;
        }
        ret = strncpy_s(topics[j], SIZE, individual_topic, strlen(individual_topic));
        if(ret != 0) {
            LOG_ERROR("String copy failed (errno: %d): Failed to copy individual_topic to topics", ret);
            free_mem(topics);
            return NULL;
        }
        j++;
    }
    topics[j] = NULL;
    // freeing up the "char** topics" memory has to be done from caller's (application's) side, with the help of free_mem() api.
    // Its left to caller if he wants to free char** topics or not.
    /* usage:
       free_mem(topics);
    */
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
    config_t* config = NULL;
    char* topic_cfg = NULL;
    char* data = NULL;
    char* actual_topic = NULL;
    char* publisher = NULL;
    char* temp_topic = NULL;
    char* address = NULL;
    char mode[SIZE] = {0,};
    char host[SIZE] = {0,};
    char port[SIZE] = {0,};
    char publicKey[SIZE] = {0,};
    char app_private_key[SIZE] = {0,};
    char topic_arr[SIZE] = {0,};
    char topic_cfg_arr[SIZE] = {0,};
    char* client = NULL;
    char clients[SIZE] = {0,};
    char publisher_topic[SIZE] = {0,};
    int ret = 0;
    int i;
    int j;

    char** mode_address = (char **)calloc(SIZE, sizeof(char*));
    if(mode_address == NULL) {
        LOG_ERROR_0("Calloc failed for mode_address");
        goto err;
    }
    char** host_port = (char **)calloc(SIZE, sizeof(char*));
    if(host_port == NULL) {
        LOG_ERROR_0("Calloc failed for host_port");
        goto err;
    }
    const char** allowed_clients = (char **)calloc(SIZE, sizeof(char*));
    if(allowed_clients == NULL) {
        LOG_ERROR_0("Calloc failed for allowed_clients");
        goto err;
    }
    const char **pub_topic = (char **)calloc(SIZE, sizeof(char*));
    if(pub_topic == NULL) {
        LOG_ERROR_0("Calloc failed for pub_topic");
        goto err;
    }

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

    if (!strcmp(topic_type,SUB)){
        char* individual_topic;
        ret = strncpy_s(publisher_topic, SIZE, topic[0], strlen(topic[0]));
        if(ret != 0) {
            LOG_ERROR("String copy failed (errno: %d): Failed to copy topic to publisher_topic", ret);
            goto err;
        }        

        ret = strncpy_s(topic_arr, SIZE, topic[0], strlen(topic[0]));
        if(ret != 0) {
            LOG_ERROR("String copy failed (errno: %d): Failed to copy topic to topic_arr", ret);
            goto err;
        }

        temp_topic = topic_arr;

        j=0;
        while((individual_topic = strtok_r(temp_topic, "/", &temp_topic))) {
            if(pub_topic[j] == NULL){
                pub_topic[j] = (char*) malloc(strlen(individual_topic));
            }
            if(pub_topic[j] == NULL){
                LOG_ERROR_0("Malloc failed for individual pub_topic");
                goto err;
            }
            ret = strncpy_s(pub_topic[j], SIZE, individual_topic, strlen(individual_topic));
            if(ret != 0) {
                LOG_ERROR("String copy failed (errno: %d): Failed to copy individual_topic to pub_topic", ret);
                goto err;
            }
            j++;
        }

        if (j == 1 || j > 2) {
            LOG_ERROR("sub topic should be of the format [AppName]/[stream_name]");
            goto err;
        }
        actual_topic = (char*) malloc(strlen(pub_topic[1]) + 1);
        if(actual_topic == NULL) {
            LOG_ERROR_0("malloc failed for actual_topic");
            goto err;
        }
        memset(actual_topic, 0, strlen(pub_topic[1]) + 1);
        publisher = (char*) malloc(strlen(pub_topic[0]) + 1);
        if(publisher == NULL) {
            LOG_ERROR_0("malloc failed for publisher");
            goto err;
        }
        if(publisher != NULL){
            memset(publisher, 0, strlen(pub_topic[0]) + 1);
        }
        ret = strncpy_s(actual_topic, SIZE, pub_topic[1], strlen(pub_topic[1]));
        if(ret != 0) {
            LOG_ERROR("String copy failed (errno: %d) : Failed to copy pub_topic[1] to actual_topic", ret);
            goto err;
        }
        LOG_DEBUG("Actual topic: %s", actual_topic);
        if(publisher != NULL){
            ret = strncpy_s(publisher, SIZE, pub_topic[0],strlen(pub_topic[0]));
            if(ret != 0) {
                LOG_ERROR("String copy failed (errno: %d) : Failed to copy pub_topic[0] to publisher", ret);
                goto err;
            }
        }
        LOG_DEBUG("publisher: %s", publisher);
        ret = strncpy_s(publisher_topic, SIZE, actual_topic, strlen(actual_topic));
        if(ret != 0) {
            LOG_ERROR("String copy failed (errno: %d): Failed to copy actual_topic to publisher_topic", ret);
            goto err;
        }
        ret = strncat_s(publisher_topic, SIZE, CFG, strlen(CFG));
        if(ret != 0) {
            LOG_ERROR("String concatenation failed (errno: %d): Failed to concatenate cfg to publisher_topic", ret);
            goto err;
        }
        ret = strncpy_s(topic_cfg_arr, SIZE, getenv(publisher_topic), SIZE);
        if(ret != 0) {
            LOG_ERROR("String copy failed (errno: %d): Failed to copy publisher_topic to topic_cfg_arr", ret);
            goto err;
        }
    } else if(!strcmp(topic_type,PUB)){
        ret = strncpy_s(publisher_topic,SIZE, topic[0], strlen(topic[0]));
        if(ret != 0) {
            LOG_ERROR("String copy failed (errno: %d): Failed to copy topic to publisher_topic", ret);
            goto err;
        }
        ret = strncat_s(publisher_topic, SIZE, CFG, strlen(CFG));
        if(ret != 0) {
            LOG_ERROR("String concatenation failed (errno: %d):  Failed to concatenate cfg to publisher_topic", ret);
            goto err;
        }
        ret = strncpy_s(topic_cfg_arr, SIZE, getenv(publisher_topic), SIZE);
        if(ret != 0) {
            LOG_ERROR("String copy failed (errno: %d): Failed to copy publisher topic env value to topic_cfg_arr", ret);
            goto err;
        }
    }else if(!strcmp(topic_type,"server")){
        ret = strncpy_s(topic_cfg_arr, SIZE, getenv("Server"), SIZE);
        if(ret != 0) {
            LOG_ERROR("String copy failed (errno: %d): Failed to copy server env value to topic_cfg_arr", ret);
            goto err;
        }
    }else if(!strcmp(topic_type,CLIENT)){
        ret = strncpy_s(publisher_topic,SIZE, topic[0], strlen(topic[0]));
        if(ret != 0) {
            LOG_ERROR("String copy failed (errno: %d): Failed to copy topic to publisher_topic", ret);
            goto err;
        }
        ret = strncat_s(publisher_topic, SIZE, CFG, strlen(CFG));
        if(ret != 0) {
            LOG_ERROR("String concatenation failed (errno: %d): Failed to concatenate cfg to publisher_topic", ret);
            goto err;
        }
        ret = strncpy_s(topic_cfg_arr, SIZE, getenv(publisher_topic), SIZE);
        if(ret != 0) {
            LOG_ERROR("String copy failed (errno: %d): Failed to copy publisher topic env value to topic_cfg_arr", ret);
            goto err;
        }
    } else {
        LOG_ERROR("topic type: %s is not supported", topic_type);
        goto err;
    }
    LOG_DEBUG("publisher_topic: %s", publisher_topic);

    topic_cfg = topic_cfg_arr;
    i=0;
    while((data = strtok_r(topic_cfg, ",", &topic_cfg))) {
        if(mode_address[i] == NULL){
            mode_address[i] = (char*) malloc(strlen(data));
        }
        if(mode_address[i] == NULL){
            LOG_ERROR_0("Malloc failed for individual mode_address");
            goto err;
        }
        ret = strncpy_s(mode_address[i], SIZE, data, strlen(data));
        if(ret != 0) {
            LOG_ERROR("String copy failed (errno: %d): Failed to copy data to mode_address", ret);
            goto err;
        }
        i++;
    }
    if(mode_address[2] == NULL) {
        LOG_DEBUG_0("socket file not explicitly given by application");
    }
    else {
        LOG_DEBUG_0("socket file explicitly given by application");
    }

    ret = strncpy_s(mode, SIZE, mode_address[0], strlen(mode_address[0]));
    if(ret != 0) {
        LOG_ERROR("String copy failed (errno: %d) :  Failed to copy mode", ret);
        goto err;
    }
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
            if(host_port[i] == NULL){
                host_port[i] = (char*) malloc(strlen(data));
            }
            if(host_port[i] == NULL){
                LOG_ERROR_0("Malloc failed for individual host_port");
                goto err;
            }
            ret = strncpy_s(host_port[i], SIZE, data, strlen(data));
            if(ret != 0) {
                LOG_ERROR("String copy failed (errno: %d) : Failed to copy data to host_port", ret);
                goto err;
            }
            i++;
        }

        ret = strncpy_s(host, SIZE, host_port[0], strlen(host_port[0]));
        if(ret != 0) {
            LOG_ERROR("String copy failed (errno: %d): Failed to copy host", ret);
            goto err;
        }
        trim(host);

        ret = strncpy_s(port, SIZE, host_port[1], strlen(host_port[1]));
        if(ret != 0) {
            LOG_ERROR("String copy failed (errno: %d): Failed to copy port", ret);
            goto err;
        }
        trim(port);

        __int64_t i_port = atoi(port);
        if(!strcmp(topic_type,PUB)){
            cJSON* zmq_tcp_publish = cJSON_CreateObject();
            cJSON_AddItemToObject(json, "zmq_tcp_publish", zmq_tcp_publish);
            cJSON_AddStringToObject(zmq_tcp_publish, "host", host);
            cJSON_AddNumberToObject(zmq_tcp_publish, "port", i_port);

            if(!dev_mode){
                i = 0;
                ret = strncpy_s(clients, SIZE+1, getenv(CLIENTS_ENV), SIZE);
                if(ret != 0) {
                    LOG_ERROR("String copy failed (errno: %d) : Failed to copy clients", ret);
                    goto err;
                }
                client = clients;
                while((data = strtok_r(client, ",", &client))) {
                    if(allowed_clients[i] == NULL){
                        allowed_clients[i] = (char*) malloc(strlen(data));
                    }
                    if(allowed_clients[i] == NULL){
                        LOG_ERROR_0("Malloc failed for individual allowed_clients");
                        goto err;
                    }
                    ret = strncpy_s(allowed_clients[i], SIZE, data, strlen(data));
                    if(ret != 0) {
                        LOG_ERROR("String copy failed (errno: %d) : Failed to copy allowed clients", ret);
                        goto err;
                    }
                    i++;
                }
                cJSON* all_clients = cJSON_CreateArray();
                for(j=0; j<i; j++){
                    ret = strncpy_s(publicKey, SIZE, "/Publickeys/", strlen("/Publickeys/"));
                    if(ret != 0) {
                        LOG_ERROR("String copy failed (errno: %d): Failed to copy /Publickeys/", ret);
                        goto err;
                    }
                    ret = strncat_s(publicKey, SIZE, allowed_clients[j], strlen(allowed_clients[j]));
                    if(ret != 0) {
                        LOG_ERROR("String concatenation failed (errno: %d): Failed to concatenate client name to /Publickeys/", ret);
                        goto err;
                    }
                    char* client_pub_key = configmgr->get_config(publicKey);

                    if(strcmp(client_pub_key,"")){
                        cJSON_AddItemToArray(all_clients, cJSON_CreateString(client_pub_key));
                    }
                }

                app_private_key[0] = '/';
                ret = strncat_s(app_private_key, SIZE, app_name_env, strlen(app_name_env));
                if(ret != 0) {
                    LOG_ERROR("String concatenation failed (errno: %d): Failed to concatenate appname", ret);
                    goto err;
                }
                ret = strncat_s(app_private_key, SIZE, "/private_key", strlen("/private_key"));
                if(ret != 0) {
                    LOG_ERROR("String concatenation failed (errno: %d): Failed to concatenate /private_key to appname", ret);
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

            if(!dev_mode){
                ret = strncpy_s(publicKey, SIZE, "/Publickeys/", strlen("/Publickeys/"));
                if(ret != 0) {
                    LOG_ERROR("String copy failed (errno: %d): Failed to copy /Publickeys/", ret);
                    goto err;
                }
                if(publisher != NULL){
                    ret = strncat_s(publicKey,SIZE, publisher, strlen(publisher));
                    if(ret != 0) {
                        LOG_ERROR("String concatenation failed (errno: %d): Failed to concatenate publisher's name to /Publickeys/", ret);
                        goto err;
                    }
                }

                const char* server_pub_key = configmgr->get_config(publicKey);
                cJSON_AddStringToObject(pub_sub_topic, "server_public_key",server_pub_key);
                ret = strncpy_s(publicKey, SIZE, "/Publickeys/", strlen("/Publickeys/"));
                if(ret != 0) {
                    LOG_ERROR("String copy failed (errno: %d): Failed to copy /Publickeys/", ret);
                    goto err;
                }
                ret = strncat_s(publicKey, SIZE, app_name_env, strlen(app_name_env));
                if(ret != 0) {
                    LOG_ERROR("String concatenation failed (errno: %d): Failed to concatenate app name to /Publickeys/", ret);
                    goto err;
                }

                const char* client_pub_key = configmgr->get_config(publicKey);
                cJSON_AddStringToObject(pub_sub_topic, "client_public_key",client_pub_key);

                app_private_key[0] = '/';
                ret = strncat_s(app_private_key,SIZE,app_name_env, strlen(app_name_env));
                if(ret != 0) {
                    LOG_ERROR("String concatenation failed (errno: %d): Failed to concatenate app name to /", ret);
                    goto err;
                }
                ret = strncat_s(app_private_key, SIZE, "/private_key", strlen("/private_key"));
                if(ret != 0) {
                    LOG_ERROR("String concatenation failed (errno: %d): Failed to concatenate /private_key to app name", ret);
                    goto err;
                }

                const char* client_secret_key = configmgr->get_config(app_private_key);
                cJSON_AddStringToObject(pub_sub_topic, "client_secret_key",client_secret_key);
            }

        }
        else if (!strcmp(topic_type,"server")){
            cJSON* zmq_tcp_server = cJSON_CreateObject();
            cJSON_AddItemToObject(json, topic[0], zmq_tcp_server);
            cJSON_AddStringToObject(zmq_tcp_server, "host", host);
            cJSON_AddNumberToObject(zmq_tcp_server, "port", i_port);

            if(!dev_mode){
                i = 0;
                ret = strncpy_s(clients, SIZE+1, getenv(CLIENTS_ENV), SIZE);
                if(ret != 0) {
                    LOG_ERROR("String copy failed (errno: %d) : Failed to copy clients", ret);
                    goto err;
                }
                client = clients;
                while((data = strtok_r(client, ",", &client))) {
                    if(allowed_clients[i] == NULL){
                        allowed_clients[i] = (char*) malloc(strlen(data));
                    }
                    if(allowed_clients[i] == NULL){
                        LOG_ERROR_0("Malloc failed for individual allowed_clients");
                        goto err;
                    }
                    ret = strncpy_s(allowed_clients[i], SIZE, data, strlen(data));
                    if(ret != 0) {
                        LOG_ERROR("String copy failed (errno: %d) : Failed to copy allowed clients", ret);
                        goto err;
                    }
                    i++;
                }

		cJSON* all_clients = cJSON_CreateArray();
                for(j=0; j<i; j++){
                    ret = strncpy_s(publicKey, SIZE, "/Publickeys/", strlen("/Publickeys/"));
                    if(ret != 0) {
                        LOG_ERROR("String copy failed (errno: %d): Failed to copy publicKey", ret);
                        goto err;
                    }
                    ret = strncat_s(publicKey, SIZE, allowed_clients[j], strlen(allowed_clients[j]));
                    if(ret != 0) {
                        LOG_ERROR("String concatenation failed (errno: %d): Failed to concatenate for getting key of client", ret);
                        goto err;
                    }
                    char* client_pub_key = configmgr->get_config(publicKey);

                    if(strcmp(client_pub_key,"")){
                        cJSON_AddItemToArray(all_clients, cJSON_CreateString(client_pub_key));
                    }
                }

                app_private_key[0] = '/';
                ret = strncat_s(app_private_key, SIZE, app_name_env, strlen(app_name_env));
                if(ret != 0) {
                    LOG_ERROR("String concatenation failed (errno: %d): Failed to concatenate for getting key of publisher", ret);
                    goto err;
                }
                ret = strncat_s(app_private_key, SIZE, "/private_key", strlen("/private_key"));
                if(ret != 0) {
                    LOG_ERROR("String concatenation failed (errno: %d): Failed to concatenate for getting key of publisher", ret);
                    goto err;
                }

	        const char* server_secret_key = configmgr->get_config(app_private_key);

                cJSON_AddStringToObject(zmq_tcp_server, "server_secret_key",server_secret_key);
                cJSON_AddItemToObject(json, "allowed_clients", all_clients);
	    }
	}
	else if(!strcmp(topic_type,"client")){
            cJSON* zmq_tcp_client = cJSON_CreateObject();
            cJSON_AddItemToObject(json, topic[0], zmq_tcp_client);
            cJSON_AddStringToObject(zmq_tcp_client, "host", host);
            cJSON_AddNumberToObject(zmq_tcp_client, "port", i_port);

	    if(!dev_mode){
		char* end_point_list = NULL;
                char* end_point = NULL;
                end_point_list = getenv(RequestEP);
                while((end_point = strtok_r(end_point_list, ",", &end_point_list))) {
                    if (!strcmp(topic[0],end_point)) {
                        ret = strncpy_s(publicKey, SIZE, "/Publickeys/", strlen("/Publickeys/"));
                        if(ret != 0) {
                            LOG_ERROR("String copy failed (errno: %d): Failed to copy public keys", ret);
                            goto err;
                        }
                        ret = strncat_s(publicKey, SIZE, app_name_env, strlen(app_name_env));
                        if(ret != 0) {
                            LOG_ERROR("String concatenation failed (errno: %d): Failed to concatenate for getting key of client", ret);
                            goto err;
                        }

                        const char* client_pub_key = configmgr->get_config(publicKey);
                        cJSON_AddStringToObject(zmq_tcp_client, "client_public_key",client_pub_key);
                        ret = strncpy_s(publicKey, SIZE, "/Publickeys/", strlen("/Publickeys/"));

                        if(ret != 0) {
                            LOG_ERROR("String copy failed (errno: %d): Failed to copy public keys", ret);
                            goto err;
                        }
                        ret = strncat_s(publicKey,SIZE, topic[0], strlen(topic[0]));
                        if(ret != 0) {
                            LOG_ERROR("String concatenation failed (errno: %d): Failed to concatenate topic to public key", ret);
                            goto err;
                        }

                        const char* server_pub_key = configmgr->get_config(publicKey);
                        cJSON_AddStringToObject(zmq_tcp_client, "server_public_key",server_pub_key);

                        app_private_key[0] = '/';
                        ret = strncat_s(app_private_key,SIZE,app_name_env, strlen(app_name_env));
                        if(ret != 0) {
                            LOG_ERROR("String concatenation failed (errno: %d): Failed to concatenate app_name_env to app_private_key", ret);
                            goto err;
                        }
                        ret = strncat_s(app_private_key, SIZE, "/private_key", strlen("/private_key"));
                        if(ret != 0) {
                            LOG_ERROR("String concatenation failed (errno: %d): Failed to concatenate private_key to app_private_key", ret);
                            goto err;
                        }
                        const char* client_secret_key = configmgr->get_config(app_private_key);
                        cJSON_AddStringToObject(zmq_tcp_client, "client_secret_key",client_secret_key);
                    }
                }
            }
        } else {
            LOG_ERROR_0("topic is neither PUB nor SUB / neither Server nor Client");
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

    }else {
        LOG_ERROR("mode: %s is not supported", mode);
        goto err;
    }

    char* config_value = cJSON_Print(json);
    LOG_DEBUG("Env Config is : %s \n", config_value);

    config = config_new(
            (void*) json, free_json, get_config_value);
    if(config == NULL) {
        LOG_ERROR_0("Failed to initialize configuration object");
        goto err;
    }

err:
    if(actual_topic != NULL) {
        free(actual_topic);
    }
    if(publisher != NULL) {
        free(publisher);
    }
    if(mode_address != NULL) {
        free_mem(mode_address);
    }
    if(host_port != NULL) {
        free_mem(host_port);
    }
    if(allowed_clients != NULL) {
        free_mem(allowed_clients);
    }
    if(pub_topic != NULL) {
        free_mem(pub_topic);
    }

    return config;
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
    env_config->free_mem = free_mem;
    return env_config;
}
