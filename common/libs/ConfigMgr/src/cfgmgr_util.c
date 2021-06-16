// Copyright (c) 2021 Intel Corporation.
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
 * @brief ConfigMgr utility C Implementation
 * Holds the implementaion of utilities supported for ConfigMgr
 */

#include <stdarg.h>
#include "eii/config_manager/cfgmgr_util.h"

#define MAX_CONFIG_KEY_LENGTH 250

// Helper function to convert config_t object to char*
char* configt_to_char(config_t* config) {
    cJSON* temp = (cJSON*)config->cfg;
    if(temp == NULL) {
        LOG_ERROR_0("cJSON temp object is NULL");
        return NULL;
    }
    char* config_value_cr = cJSON_Print(temp);
    if(config_value_cr == NULL) {
        LOG_ERROR_0("config_value_cr object is NULL");
        return NULL;
    }
    return config_value_cr;
}

// Helper function to convert config_value_t object to char*
char* cvt_to_char(config_value_t* config) {

    cJSON* c_json = (cJSON*)config->body.object->object;
    char* config_value_cr = cJSON_Print(c_json);
    if(config_value_cr == NULL) {
        LOG_ERROR_0("config_value_cr object is NULL");
        return NULL;
    }
    return config_value_cr;
}

bool get_ipc_config(config_t* c_json, config_value_t* config, const char* end_point, cfgmgr_iface_type_t type){

    int ret;
    config_value_t* topics = NULL;
    bool result = false;
    char *sock_file = NULL;
    char* sock_dir = NULL;
    config_value_t* json_endpoint = NULL;
    char** socket_ep = NULL;
    config_value_t* topics_list = NULL;
    config_value_t* service_name = NULL;
    config_value_t* socketdir_cvt = NULL;
    config_value_t* socketfile_cvt = NULL;
    config_t* socket_file_obj = NULL;
    config_value_t* sock_file_cvt = NULL;
    config_value_t* brokered_value = NULL;
    config_value_t* brokered_cvt = NULL;
    config_value_t* socket_file_obj_cvt = NULL;
    config_value_t* sock_dir_cvt = NULL;
    bool config_set_result = false;

    sock_dir = (char*) malloc(MAX_CONFIG_KEY_LENGTH);
    if (sock_dir == NULL) {
        LOG_ERROR_0("Malloc failed for sock_dir");
        goto err;
    }

    json_endpoint = config_value_object_get(config, ENDPOINT);
    if (json_endpoint == NULL) {
        LOG_ERROR_0("json_endpoint initialization failed");
        goto err;
    }

    // Do not check for Topics if interface type is CFGMGR_SERVER/CFGMGR_CLIENT
    // since CFGMGR_SERVER/CFGMGR_CLIENT interfaces don't have Topics
    if (type != CFGMGR_SERVER && type != CFGMGR_CLIENT) {
        topics_list = config_value_object_get(config, TOPICS);
        if (topics_list == NULL) {
            LOG_ERROR_0("topics_list initialization failed");
            goto err;
        }
    }

    if (json_endpoint->type == CVT_OBJECT) {

        socketdir_cvt = config_value_object_get(json_endpoint, "SocketDir");
        if (socketdir_cvt == NULL || socketdir_cvt->body.string == NULL) {
            LOG_ERROR_0("socketdir_cvt initialization failed");
            goto err;
        }
        socketfile_cvt = config_value_object_get(json_endpoint, "SocketFile");
        if (socketfile_cvt == NULL || socketfile_cvt->body.string == NULL) {
            LOG_ERROR_0("socketfile_cvt initialization failed");
            goto err;
        }

        if (sock_dir != NULL) {
            LOG_DEBUG_0("Re-assign the sock_dir with new allocation, hence freeing the old one.");
            free(sock_dir);
        }

        sock_dir = socketdir_cvt->body.string;

        strcmp_s(socketfile_cvt->body.string, strlen(socketfile_cvt->body.string), "*", &ret);
        if (ret != 0) {
            sock_file = socketfile_cvt->body.string;
        }
    } else {
        char* data = NULL;
        char* ref_ptr = NULL;
        char* str = NULL;
        int i;
        size_t data_len;
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
        socket_ep = NULL;
    }

    // When Socket file is explicitly given by the user.
    if (sock_file != NULL) {
        LOG_INFO("socket_ep file explicitly given by application");
        // For Publisher & Subscriber use case
        if (type != CFGMGR_SERVER && type != CFGMGR_CLIENT) {
            topics_list = config_value_object_get(config, "Topics");
            if (topics_list == NULL) {
                LOG_ERROR_0("topics_list initialization failed");
                goto err;
            }

            size_t arr_len = config_value_array_len(topics_list);
            if (arr_len == 0) {
                LOG_ERROR_0("Empty array is not supported, atleast one value should be given.");
            }
            // getting the topic list and mapping multiple topics to the socket file
            for (size_t i=0; i < arr_len; ++i) {
                socket_file_obj = json_config_new_from_buffer("{}");
                if (socket_file_obj == NULL) {
                    LOG_ERROR_0("c_json initialization failed");
                    goto err;
                }
                topics = config_value_array_get(topics_list, i);
                if (topics == NULL) {
                    LOG_ERROR("Failed to fetch topic at %d", i);
                    goto err;
                }

                strcmp_s(topics->body.string, strlen(topics->body.string), "*", &ret);

                sock_file_cvt = config_value_new_string(sock_file);
                if (sock_file_cvt == NULL) {
                    LOG_ERROR_0("sock_file_cvt initialization failed");
                    goto err;
                }
                config_set_result = config_set(socket_file_obj, SOCKET_FILE, sock_file_cvt);
                if (!config_set_result) {
                    LOG_ERROR("Unable to set config value");
                }
                // Adding brokered value if available
                brokered_value = config_value_object_get(config, BROKERED);
                if (brokered_value != NULL) {
                    if (brokered_value->type != CVT_BOOLEAN) {
                        LOG_ERROR_0("brokered_value type is not boolean");
                        goto err;
                    } else {
                        if (brokered_value->body.boolean) {
                            brokered_cvt = config_value_new_boolean(true);
                            if (brokered_cvt == NULL) {
                                LOG_ERROR_0("brokered_cvt initialization failed");
                                goto err;
                            }
                            config_set_result = config_set(socket_file_obj, BROKERED, brokered_cvt);
                            if (!config_set_result) {
                                LOG_ERROR_0("Unable to set config value");
                                goto err;
                            }
                        } else {
                            brokered_cvt = config_value_new_boolean(false);
                            if (brokered_cvt == NULL) {
                                LOG_ERROR_0("brokered_cvt initialization failed");
                                goto err;
                            }
                            config_set_result = config_set(socket_file_obj, BROKERED, brokered_cvt);
                            if (!config_set_result) {
                                LOG_ERROR_0("Unable to set config value");
                                goto err;
                            }
                        }
                    }
                }
                socket_file_obj_cvt = config_value_new_object(socket_file_obj->cfg, get_config_value, NULL);;
                if (socket_file_obj_cvt == NULL) {
                    LOG_ERROR_0("socket_file_obj_cvt initialization failed");
                    goto err;
                }
                if (ret == 0) {
                    config_set_result = config_set(c_json, "", socket_file_obj_cvt);
                    if (!config_set_result) {
                        LOG_ERROR("Unable to set config value");
                        goto err;
                    }
                } else {
                    config_set_result = config_set(c_json, topics->body.string, socket_file_obj_cvt);
                    if (!config_set_result) {
                        LOG_ERROR("Unable to set config value");
                        goto err;
                    }
                }
            }
        } else {
            // For Server & Client use case
            socket_file_obj = json_config_new_from_buffer("{}");
            if (socket_file_obj == NULL) {
                LOG_ERROR_0("c_json initialization failed");
                goto err;
            }

            // Add the socket file name to socket_file_obj
            sock_file_cvt = config_value_new_string(sock_file);
            if (sock_file_cvt == NULL) {
                LOG_ERROR_0("sock_file_cvt initialization failed");
                goto err;
            }
            config_set_result = config_set(socket_file_obj, SOCKET_FILE, sock_file_cvt);
            if (!config_set_result) {
                LOG_ERROR_0("Unable to set config value");
                goto err;
            }

            // Fetch the service name associated with the interface
            service_name = config_value_object_get(config, NAME);
            if (service_name == NULL) {
                LOG_ERROR_0("service_name initialization failed");
                goto err;
            }
            if (service_name->type != CVT_STRING) {
                LOG_ERROR_0("Interface value Name should be a string");
                goto err;
            } else {
                socket_file_obj_cvt = config_value_new_object(socket_file_obj->cfg, get_config_value, NULL);;
                if (socket_file_obj_cvt == NULL) {
                    LOG_ERROR_0("socket_file_obj_cvt initialization failed");
                    goto err;
                }
                config_set_result = config_set(c_json, service_name->body.string, socket_file_obj_cvt);
                if (!config_set_result) {
                    LOG_ERROR_0("Unable to set config value");
                    goto err;
                }
            }
        }
    } else {
        // Socket file will be created by EII message bus based on the topic
        LOG_DEBUG_0("socket_ep file not explicitly given by application");
        // Do not check for Topics if interface type is CFGMGR_SERVER/CFGMGR_CLIENT
        // since CFGMGR_SERVER/CFGMGR_CLIENT interfaces don't have Topics
        if (type != CFGMGR_SERVER && type != CFGMGR_CLIENT) {
            topics = config_value_array_get(topics_list, 0);
            strcmp_s(topics->body.string, strlen(topics->body.string), "*", &ret);
            if (ret == 0) {
                LOG_ERROR_0("Topics cannot be \"*\" if socket file is not explicitly mentioned");
                goto err;
            }
            config_value_destroy(topics);
        }
    }

    // Add Endpoint directly to socket_dir if IPC mode
    sock_dir_cvt = config_value_new_string(sock_dir);
    if (sock_dir_cvt == NULL) {
        LOG_ERROR_0("sock_dir_cvt initialization failed");
        goto err;
    }
    config_set_result = config_set(c_json, "socket_dir", sock_dir_cvt);
    if (!config_set_result) {
        LOG_ERROR_0("Unable to set config value");
        goto err;
    }

    // This line should be last to execute in success path, Add your code above it. :)
    result = config_set_result;

err:
    if (sock_file != NULL) {
        free(sock_file);
    }
    if (sock_dir != NULL) {
        free(sock_dir);
    }
    if (service_name != NULL) {
        config_value_destroy(service_name);
    }
    if (topics_list != NULL) {
        config_value_destroy(topics_list);
    }
    if (json_endpoint != NULL) {
        config_value_destroy(json_endpoint);
    }
    if (socket_ep != NULL) {
        free_mem(socket_ep);
    }
    if (sock_dir_cvt != NULL) {
        config_value_destroy(sock_dir_cvt);
    }
    return result;
}

char* cvt_obj_str_to_char(config_value_t* cvt){
    char* value;
    if (cvt->type == CVT_OBJECT){
        value = cvt_to_char(cvt);
        if(value == NULL){
            LOG_ERROR_0("cvt to char failed, value is NULL");
            return NULL;
        }
    } else if (cvt->type == CVT_STRING) {
        value = cvt->body.string;
        if(value == NULL){
            LOG_ERROR_0("cvt string value is NULL");
            return NULL;
        }
    } else {
        LOG_ERROR_0("EndPoint type mis match: It should be either string or json");
        return NULL;
    }

    return value;
}

bool construct_tcp_publisher_prod(char* app_name, config_t* c_json, config_t* inner_json, void* handle, config_value_t* config, kv_store_client_t* kv_store_client){
    bool ret_val = false;
    config_value_t* value = NULL;
    config_value_t* publish_json_clients = NULL;
    config_value_t* pub_key_values = NULL;
    config_value_t* array_value = NULL;
    char* grab_public_key = NULL;
    char* sub_public_key = NULL;
    config_value_t* temp_array_value = NULL;
    char* publisher_secret_key = NULL;
    char* pub_pri_key = NULL;
    config_t* all_clients_arr_config = NULL;
    config_value_t* publisher_secret_key_cvt = NULL;
    config_value_t* all_clients_arr = NULL;
    char **all_clients = NULL;

    publish_json_clients = config_value_object_get(config, ALLOWED_CLIENTS);
    if (publish_json_clients == NULL) {
        LOG_ERROR_0("publish_json_clients initialization failed");
        goto err;
    }

    // Checking if Allowed clients is empty string
    if (config_value_array_len(publish_json_clients) == 0) {
        LOG_ERROR_0("Empty String is not supported in AllowedClients. Atleast one allowed clients is required");
        goto err;
    }

    // Fetch the first item in allowed_clients
    temp_array_value = config_value_array_get(publish_json_clients, 0);
    if (temp_array_value == NULL) {
        LOG_ERROR_0("temp_array_value initialization failed");
        goto err;
    }
    int result;
    strcmp_s(temp_array_value->body.string, strlen(temp_array_value->body.string), "*", &result);

    size_t arr_len = config_value_array_len(publish_json_clients);
    if (arr_len == 0) {
        LOG_ERROR_0("Empty array is not supported, atleast one value should be given.");
        goto err;
    }

    // If only one item in allowed_clients and it is *
    // Add all available Publickeys
    if ((arr_len == 1) && (result == 0)) {
        pub_key_values = kv_store_client->get_prefix(handle, "/Publickeys/");
        if (pub_key_values == NULL) {
            LOG_ERROR_0("pub_key_values initialization failed");
            goto err;
        }
        bool config_set_result = config_set(c_json, "allowed_clients", pub_key_values);
        if (!config_set_result) {
            LOG_ERROR("Unable to set config value");
        }
    } else {
        arr_len = config_value_array_len(publish_json_clients);
        if (arr_len == 0) {
            LOG_ERROR_0("Empty array is not supported, atleast one value should be given.");
        }
        all_clients = (char**)calloc(arr_len, sizeof(char*));
        if (all_clients == NULL) {
            LOG_ERROR_0("all_clients initialization failed");
            goto err;
        }
        for (int i =0; i < arr_len; i++) {
            // Fetching individual public keys of all AllowedClients
            array_value = config_value_array_get(publish_json_clients, i);
            if (array_value == NULL) {
                LOG_ERROR_0("array_value initialization failed");
                goto err;
            }
            size_t init_len = strlen(PUBLIC_KEYS) + strlen(array_value->body.string) + 2;
            grab_public_key = concat_s(init_len, 2, PUBLIC_KEYS, array_value->body.string);
            if (grab_public_key == NULL) {
                LOG_ERROR_0("Concatenation failed for getting public keys");
                goto err;
            }
            sub_public_key = kv_store_client->get(handle, grab_public_key);
            if (sub_public_key == NULL) {
                // If any service isn't provisioned, ignore if key not found
                LOG_DEBUG("Value is not found for the key: %s", grab_public_key);
                all_clients[i] = NULL;
            } else {
                all_clients[i] = sub_public_key;
            }
            // Before Loop iterates, release all allocated mems.
            if (grab_public_key != NULL) {
                free(grab_public_key);
            }
            if (array_value != NULL) {
                config_value_destroy(array_value);
            }
        }

        all_clients_arr_config = json_config_new_array(all_clients, arr_len);
        if (all_clients_arr_config == NULL) {
            LOG_ERROR_0("Failed to create all_clients_arr_config config_t object");
            goto err;
        }

        all_clients_arr = config_value_new_object(all_clients_arr_config->cfg, get_config_value, NULL);
        if (all_clients_arr == NULL) {
            LOG_ERROR_0("Unable to create config_value_t all_clients_arr object");
            goto err;
        }
        // Adding all public keys of clients to allowed_clients of config
        bool config_set_result = config_set(c_json, "allowed_clients", all_clients_arr);
        if (!config_set_result) {
            LOG_ERROR("Unable to set config value");
            goto err;
        }
    }
    // Fetching Publisher private key & adding it to zmq_tcp_publish object
    size_t init_len = strlen("/") + strlen(app_name) + strlen(PRIVATE_KEY) + 2;
    pub_pri_key = concat_s(init_len, 3, "/", app_name, PRIVATE_KEY);
    if (pub_pri_key == NULL) {
        LOG_ERROR_0("Concatenation failed for getting private keys");
        goto err;
    }
    publisher_secret_key = kv_store_client->get(handle, pub_pri_key);
    if (publisher_secret_key == NULL) {
        LOG_ERROR("Value is not found for the key: %s", pub_pri_key);
        goto err;
    }

    publisher_secret_key_cvt = config_value_new_string(publisher_secret_key);
    if (publisher_secret_key_cvt == NULL) {
        LOG_ERROR_0("Get publisher_secret_key_cvt failed");
        goto err;
    }
    bool config_set_result = config_set(inner_json, "server_secret_key", publisher_secret_key_cvt);
    if (!config_set_result) {
        LOG_ERROR("Unable to set config value");
        goto err;
    }

    // We should add all success-path code above this line.
    ret_val = true;

    err:
        if (value != NULL) {
            config_value_destroy(value);
        }
        if (publisher_secret_key != NULL) {
            free(publisher_secret_key);
        }
        if (pub_pri_key != NULL) {
            free(pub_pri_key);
        }
        if (sub_public_key != NULL) {
            free(sub_public_key);
        }
        if (publish_json_clients != NULL) {
            config_value_destroy(publish_json_clients);
        }
        if (pub_key_values != NULL) {
            config_value_destroy(pub_key_values);
        }
        if (array_value != NULL) {
            config_value_destroy(array_value);
        }
        if (publisher_secret_key_cvt != NULL) {
            config_value_destroy(publisher_secret_key_cvt);
        }
        return ret_val;
}

bool add_keys_to_config(config_t* sub_topic, char* app_name, kv_store_client_t* kv_store_client, void* handle, config_value_t* publisher_appname, config_value_t* sub_config) {
    bool ret_val = false;
    char* s_sub_pri_key = NULL;
    char* s_sub_public_key = NULL;
    char* sub_public_key = NULL;
    char* sub_pri_key = NULL;
    char* pub_public_key = NULL;
    bool config_set_result = false;
    config_value_t* sub_pri_key_cvt = NULL;
    config_value_t* pub_public_key_cvt = NULL;
    config_value_t* sub_public_key_cvt = NULL;

    size_t init_len = strlen(PUBLIC_KEYS) + strlen(publisher_appname->body.string) + 2;
    char* grab_public_key = concat_s(init_len, 2, PUBLIC_KEYS, publisher_appname->body.string);
    if (grab_public_key == NULL){
        LOG_ERROR_0("Failed to conact PUBLIC_KEYS and PublisherAppName value");
        goto err;
    }

    pub_public_key = kv_store_client->get(handle, grab_public_key);
    if(pub_public_key == NULL){
        LOG_DEBUG("Value is not found for the key: %s", grab_public_key);
    }

    if (pub_public_key != NULL) {
        // Adding Publisher public key to config
        pub_public_key_cvt = config_value_new_string(pub_public_key);
        if (pub_public_key_cvt == NULL) {
            LOG_ERROR_0("Get pub_public_key_cvt failed");
            goto err;
        }
        config_set_result = config_set(sub_topic, "server_public_key", pub_public_key_cvt);
        if (!config_set_result) {
            LOG_ERROR("Unable to set config value");
            goto err;
        }
    }

    // Adding Subscriber public key to config
    init_len = strlen(PUBLIC_KEYS) + strlen(app_name) + 2;
    s_sub_public_key = concat_s(init_len, 2, PUBLIC_KEYS, app_name);
    if (s_sub_public_key == NULL){
        LOG_ERROR_0("Failed to conact PUBLIC_KEYS and AppName");
        goto err;
    }
    sub_public_key = kv_store_client->get(handle, s_sub_public_key);
    if(sub_public_key == NULL){
        LOG_ERROR("Value is not found for applications own public key: %s", s_sub_public_key);
        ret_val=false;
        goto err;
    }

    sub_public_key_cvt = config_value_new_string(sub_public_key);
    if (sub_public_key_cvt == NULL) {
        LOG_ERROR_0("Get sub_public_key_cvt failed");
        goto err;
    }
    config_set_result = config_set(sub_topic, "client_public_key", sub_public_key_cvt);
    if (!config_set_result) {
        LOG_ERROR("Unable to set config value");
        goto err;
    }

    // Adding Subscriber private key to config
    init_len = strlen("/") + strlen(app_name) + strlen(PRIVATE_KEY) + 2;
    s_sub_pri_key = concat_s(init_len, 3, "/", app_name, PRIVATE_KEY);
    if (s_sub_pri_key == NULL){
        LOG_ERROR_0("Failed to conact /AppName and PRIVATE_KEY");
        goto err;
    }

    sub_pri_key = kv_store_client->get(handle, s_sub_pri_key);
    if(sub_pri_key == NULL){
        LOG_ERROR("Value is not found for applications own private key: %s", s_sub_pri_key);
        goto err;
    }

    sub_pri_key_cvt = config_value_new_string(sub_pri_key);
    if (sub_pri_key_cvt == NULL) {
        LOG_ERROR_0("Get sub_pri_key_cvt failed");
        goto err;
    }
    config_set_result = config_set(sub_topic, "client_secret_key", sub_pri_key_cvt);
    if (!config_set_result) {
        LOG_ERROR_0("Unable to set config value");
        goto err;
    }

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