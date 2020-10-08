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
 * @brief ConfigMgr Util functions implementation
 * Holds the implementaion of util APIs supported by ConfigMgr
 */


#include "eis/config_manager/util_cfg.h"

cJSON* get_ipc_config(cJSON* c_json, config_value_t* config, const char* end_point){

    int ret;
    char* sock_dir = (char*) malloc(MAX_CONFIG_KEY_LENGTH);
    if (sock_dir == NULL) {
        LOG_ERROR_0("Malloc failed for sock_dir");
        goto err;
    }
    
    char* sock_file = (char*) malloc(MAX_CONFIG_KEY_LENGTH);
    if (sock_file == NULL) {
        LOG_ERROR_0("Malloc failed for sock_file");
        goto err;
    }

    config_value_t* json_endpoint = config_value_object_get(config, ENDPOINT);
    if (json_endpoint == NULL) {
        LOG_ERROR_0("json_endpoint initialization failed");
        goto err;
    }

    if(json_endpoint->type == CVT_OBJECT){
        config_value_t* topics_list = config_value_object_get(config, TOPICS);
        if (topics_list == NULL) {
            LOG_ERROR_0("topics_list initialization failed");
            goto err;
        }

        config_value_t* socketdir_cvt = config_value_object_get(json_endpoint, "SocketDir");
        config_value_t* socketfile_cvt = config_value_object_get(json_endpoint, "SocketFile");
        sock_dir = socketdir_cvt->body.string;

        strcmp_s(socketfile_cvt->body.string, strlen(socketfile_cvt->body.string), "*", &ret);
        if (ret == 0) {
            sock_file = NULL;
        } else {
            sock_file = socketfile_cvt->body.string;
        }
    } else {
        char* data = NULL;
        char* ref_ptr = NULL;
        char* str = NULL;
        int i;
        size_t data_len;
        char** socket_ep = NULL;
        socket_ep = (char **)calloc(strlen(end_point) + 1, sizeof(char*));
        if (socket_ep == NULL) {
            LOG_ERROR_0("Calloc failed for socket_ep");
            goto err;
        }

        // Tokenzing the comma separated EndPoint w.r.t IPC to extract the socket directory and socket file
        // Eg: EndPoint: "/SocketDir, SocketFile"
        for (i = 0, str = end_point; ; str = NULL, i++) {
            data = strtok_r(str, ",", &ref_ptr);
            if (data == NULL) {
                break;
            }
            data_len = strlen(data);
            if (socket_ep[i] == NULL) {
                socket_ep[i] = (char*) calloc(data_len + 1, sizeof(char));
            }
            if (socket_ep[i] == NULL) {
                LOG_ERROR_0("Calloc failed for individual socket_ep");
                goto err;
            }
            ret = strncpy_s(socket_ep[i], data_len + 1, data, data_len);
            if (ret != 0) {
                LOG_ERROR("String copy failed (errno: %d): Failed to copy data \" %s \" to socket_ep", ret, data);
                goto err;
            }
            // remove white spaces around socket_ep[i]
            trim(socket_ep[i]);
        }

        // socket_ep[0] contains the socket dir, copying it to sock_dir, so that socket_ep can be freed
        ret = strncpy_s(sock_dir, strlen(socket_ep[0]) + 1, socket_ep[0], strlen(socket_ep[0]));
        if (ret != 0) {
            LOG_ERROR("String copy failed (errno: %d): Failed to copy data \" %s \" to socket directory", ret, socket_ep[0]);
            goto err;
        }

        if (socket_ep[1] != NULL) {
            // socket_ep[1] contains the socket file, copying it to sock_file, so that socket_ep can be freed
            ret = strncpy_s(sock_file, strlen(socket_ep[1]) + 1, socket_ep[1], strlen(socket_ep[1]));
            if (ret != 0) {
                LOG_ERROR("String copy failed (errno: %d): Failed to copy data \" %s \" to socket file", ret, socket_ep[1]);
                goto err;
            }
        } else {
            sock_file = NULL;
        }
        // freeing up the char** socket_ep
        free_mem(socket_ep);
    }

    // When Socket file is explicitly given by the user.
    if (sock_file != NULL) {
        LOG_DEBUG_0("socket_ep file explicitly given by application");
        config_value_t* topics_list = config_value_object_get(config, "Topics");
        if (topics_list == NULL) {
            LOG_ERROR_0("topics_list initialization failed");
            goto err;
        }

        config_value_t* topics;
        // getting the topic list and mapping multiple topics to the socket file
        for (int i=0; i < config_value_array_len(topics_list); ++i) {
            cJSON* socket_file_obj = cJSON_CreateObject();
            topics = config_value_array_get(topics_list, i);
           
            strcmp_s(topics->body.string, strlen(topics->body.string), "*", &ret);
            if(ret == 0){
                cJSON_AddItemToObject(c_json, cJSON_CreateString(""), socket_file_obj);
            } else {
                cJSON_AddItemToObject(c_json, topics->body.string, socket_file_obj);
            }

            cJSON_AddStringToObject(socket_file_obj, SOCKET_FILE, sock_file);
            // Adding brokered value if available
            config_value_t* brokered_value = config_value_object_get(config, BROKERED);
            if (brokered_value != NULL) {
                if (brokered_value->type != CVT_BOOLEAN) {
                    LOG_ERROR_0("brokered_value type is not boolean");
                    goto err;
                } else {
                    if (brokered_value->body.boolean) {
                        cJSON_AddBoolToObject(socket_file_obj, BROKERED, true);
                    } else {
                        cJSON_AddBoolToObject(socket_file_obj, BROKERED, false);
                    }
                }
            }
        }
    } else {
        // Socket file will be created by EIS message bus based on the topic
        LOG_DEBUG_0("socket_ep file not explicitly given by application");
    }
            
    // Add Endpoint directly to socket_dir if IPC mode
    cJSON_AddStringToObject(c_json, "socket_dir", sock_dir);

    if (sock_file != NULL){
        free(sock_file);
    } 
    if (sock_dir != NULL){
        free(sock_dir);
    }

    return c_json;

    err:
        if (sock_file != NULL){
            free(sock_file);
        } 
        if (sock_dir != NULL){
            free(sock_dir);
        }
        return NULL;
}

char* cvt_obj_str_to_char(config_value_t* cvt){
    char* value;
    if (cvt->type == CVT_OBJECT){
        value = (cvt_to_char(cvt));
    } else if (cvt->type == CVT_STRING) {
        value = (cvt->body.string);
    } else {
        LOG_ERROR("EndPoint type mis match: It should be either string or json");
        return NULL;
    }

    return value;
}