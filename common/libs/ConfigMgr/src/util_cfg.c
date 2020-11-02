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

bool get_ipc_config(cJSON* c_json, config_value_t* config, const char* end_point){

    int ret;
    bool result = false;
    char *sock_file = NULL;
    char* sock_dir = (char*) malloc(MAX_CONFIG_KEY_LENGTH);
    if (sock_dir == NULL) {
        LOG_ERROR_0("Malloc failed for sock_dir");
        goto err;
    }
    
    config_value_t* json_endpoint = config_value_object_get(config, ENDPOINT);
    if (json_endpoint == NULL) {
        LOG_ERROR_0("json_endpoint initialization failed");
        goto err;
    }

    if(json_endpoint->type == CVT_OBJECT) {
        config_value_t* topics_list = config_value_object_get(config, TOPICS);
        if (topics_list == NULL) {
            LOG_ERROR_0("topics_list initialization failed");
            goto err;
        }

        config_value_t* socketdir_cvt = config_value_object_get(json_endpoint, "SocketDir");
        config_value_t* socketfile_cvt = config_value_object_get(json_endpoint, "SocketFile");
        sock_dir = socketdir_cvt->body.string;

        strcmp_s(socketfile_cvt->body.string, strlen(socketfile_cvt->body.string), "*", &ret);
        if (ret != 0) {
            sock_file = socketfile_cvt->body.string;
        }
        
        if (json_endpoint != NULL) {
            config_value_destroy(json_endpoint);
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
            sock_file = (char*) malloc(MAX_CONFIG_KEY_LENGTH);
            if (sock_file == NULL) {
                LOG_ERROR_0("Malloc failed for sock_file");
                goto err;
            }
            ret = strncpy_s(sock_file, strlen(socket_ep[1]) + 1, socket_ep[1], strlen(socket_ep[1]));
            if (ret != 0) {
                LOG_ERROR("String copy failed (errno: %d): Failed to copy data \" %s \" to socket file", ret, socket_ep[1]);
                goto err;
            }
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

    // This line should be last to execut in sucess path, Add your code above it. :-)
    result = true;

err:
    if (sock_file != NULL){
        free(sock_file);
    } 
    if (sock_dir != NULL){
        free(sock_dir);
    }
    if (json_endpoint != NULL) {
        config_value_destroy(json_endpoint);
    }
    return result;
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

bool construct_tcp_publisher_prod(char* app_name, cJSON* c_json, cJSON* inner_json, void* handle, config_value_t* config, kv_store_client_t* m_kv_store_handle){
    int ret;
    bool ret_val = false;
    config_value_t* value = NULL;
    config_value_t* publish_json_clients = NULL;
    config_value_t* pub_key_values = NULL;
    config_value_t* array_value = NULL;
    char *grab_public_key = NULL;
    const char* sub_public_key = NULL;

    publish_json_clients = config_value_object_get(config, ALLOWED_CLIENTS);
    if (publish_json_clients == NULL) {
        LOG_ERROR_0("publish_json_clients initialization failed");
        goto err;
    }

    // Checking if Allowed clients is empty string
    if (config_value_array_len(publish_json_clients) == 0){
        LOG_ERROR_0("Empty String is not supported in AllowedClients. Atleast one allowed clients is required");
        goto err;
    }

    // Fetch the first item in allowed_clients
    config_value_t* temp_array_value = config_value_array_get(publish_json_clients, 0);
    if (temp_array_value == NULL) {
        LOG_ERROR_0("temp_array_value initialization failed");
        goto err;
    }
    int result;
    strcmp_s(temp_array_value->body.string, strlen(temp_array_value->body.string), "*", &result);
    // If only one item in allowed_clients and it is *
    // Add all available Publickeys
    if ((config_value_array_len(publish_json_clients) == 1) && (result == 0)) {
        cJSON* all_clients = cJSON_CreateArray();
        if (all_clients == NULL) {
            LOG_ERROR_0("all_clients initialization failed");
            goto err;
        }
        pub_key_values = m_kv_store_handle->get_prefix(handle, "/Publickeys/");
        if (pub_key_values == NULL) {
            LOG_ERROR_0("pub_key_values initialization failed");
            goto err;
        }
        
        for (int i = 0; i < config_value_array_len(pub_key_values); i++) {
            value = config_value_array_get(pub_key_values, i);
            if (value == NULL) {
                LOG_ERROR_0("value initialization failed");
                goto err;
            }
            cJSON_AddItemToArray(all_clients, cJSON_CreateString(value->body.string));
        }
        cJSON_AddItemToObject(c_json, "allowed_clients",  all_clients);
    } else {
        
        cJSON* all_clients = cJSON_CreateArray();
        if (all_clients == NULL) {
            LOG_ERROR_0("all_clients initialization failed");
            goto err;
        }
        for (int i =0; i < config_value_array_len(publish_json_clients); i++) {
            // Fetching individual public keys of all AllowedClients
            array_value = config_value_array_get(publish_json_clients, i);
            if (array_value == NULL) {
                LOG_ERROR_0("array_value initialization failed");
                goto err;
            }
            size_t init_len = strlen(PUBLIC_KEYS) + strlen(array_value->body.string) + 2;
            grab_public_key = concat_s(init_len, 2, PUBLIC_KEYS, array_value->body.string);
            sub_public_key = m_kv_store_handle->get(handle, grab_public_key);
            if (sub_public_key == NULL) {
                // If any service isn't provisioned, ignore if key not found
                LOG_WARN("Value is not found for the key: %s", grab_public_key);
            }

            cJSON_AddItemToArray(all_clients, cJSON_CreateString(sub_public_key));
            // Before Loop iterates, release all allocated mems.
            if (grab_public_key != NULL) {
                free(grab_public_key);
            }
            if (sub_public_key != NULL) {
                free(sub_public_key);
            }
            if (array_value != NULL) {
                config_value_destroy(array_value);
            }
        }
        // Adding all public keys of clients to allowed_clients of config
        cJSON_AddItemToObject(c_json, "allowed_clients",  all_clients);
    }
    // Fetching Publisher private key & adding it to zmq_tcp_publish object
    size_t init_len = strlen("/") + strlen(app_name) + strlen(PRIVATE_KEY) + 2;
    char* pub_pri_key = concat_s(init_len, 3, "/", app_name, PRIVATE_KEY);
    const char* publisher_secret_key = m_kv_store_handle->get(handle, pub_pri_key);
    if (publisher_secret_key == NULL) {
        LOG_ERROR("Value is not found for the key: %s", pub_pri_key);
        goto err;
    }

    cJSON_AddStringToObject(inner_json, "server_secret_key", publisher_secret_key);

    // We should add all success-path code above this line.
    ret_val = true;

    err:
        if(value != NULL) {
            config_value_destroy(value);
        }
        if (publisher_secret_key) {
            free(publisher_secret_key);
        }
        if (pub_pri_key != NULL) {
            free(pub_pri_key);
        }
        if (publish_json_clients != NULL) {
            config_value_destroy(publish_json_clients);
        }
        if (temp_array_value != NULL) {
            config_value_destroy(temp_array_value);
        }
        if (pub_key_values != NULL) {
            config_value_destroy(pub_key_values);
        }
        if (array_value != NULL) {
            config_value_destroy(array_value);
        }
        return ret_val;
                    
}

bool add_keys_to_config(cJSON* sub_topic, char* app_name, kv_store_client_t* m_kv_store_handle, void* handle, config_value_t* publisher_appname, config_value_t* sub_config) {
    bool ret_val = false;
    size_t init_len = strlen(PUBLIC_KEYS) + strlen(publisher_appname->body.string) + 2;
    char* grab_public_key = concat_s(init_len, 2, PUBLIC_KEYS, publisher_appname->body.string);
    if (grab_public_key == NULL){
        LOG_ERROR_0("Failed to conact PUBLIC_KEYS and PublisherAppName value");
        goto err;
    }

    const char* pub_public_key = m_kv_store_handle->get(handle, grab_public_key);
    if(pub_public_key == NULL){
        LOG_WARN("Value is not found for the key: %s", grab_public_key);
    }

    // Adding Publisher public key to config
    cJSON_AddStringToObject(sub_topic, "server_public_key", pub_public_key);

    // Adding Subscriber public key to config
    init_len = strlen(PUBLIC_KEYS) + strlen(app_name) + 2;
    char* s_sub_public_key = concat_s(init_len, 2, PUBLIC_KEYS, app_name);
    if (s_sub_public_key == NULL){
        LOG_ERROR_0("Failed to conact PUBLIC_KEYS and AppName");
        goto err;
    }
    const char* sub_public_key = m_kv_store_handle->get(handle, s_sub_public_key);
    if(sub_public_key == NULL){
        LOG_ERROR("Value is not found for the key: %s", s_sub_public_key);
        ret_val=false;
        goto err;
    }

    cJSON_AddStringToObject(sub_topic, "client_public_key", sub_public_key);

    // Adding Subscriber private key to config
    init_len = strlen("/") + strlen(app_name) + strlen(PRIVATE_KEY) + 2;
    char* s_sub_pri_key = concat_s(init_len, 3, "/", app_name, PRIVATE_KEY);
    if (s_sub_pri_key == NULL){
        LOG_ERROR_0("Failed to conact /AppName and PRIVATE_KEY");
        goto err;
    }

    const char* sub_pri_key = m_kv_store_handle->get(handle, s_sub_pri_key);
    if(sub_pri_key == NULL){
        LOG_ERROR("Value is not found for the key: %s", s_sub_pri_key);
        goto err;
    }

    cJSON_AddStringToObject(sub_topic, "client_secret_key", sub_pri_key);

    // Add all success-path code above this line.
    ret_val = true;
    err:
        if(grab_public_key != NULL) {
            free(grab_public_key);
        }
        if(s_sub_public_key != NULL) {
            free(s_sub_public_key);
        }
        if(s_sub_pri_key != NULL) {
            free(s_sub_pri_key);
        }
        if(pub_public_key != NULL) {
            free(pub_public_key);
        }
        if(sub_public_key != NULL) {
            free(sub_public_key);
        }
        if(sub_pri_key != NULL) {
            free(sub_pri_key);
        }

    return ret_val;
}
