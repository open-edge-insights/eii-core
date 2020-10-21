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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <eis/utils/json_config.h>
#include <eis/utils/config.h>
#include <eis/config_manager/kv_store_plugin/kv_store_plugin.h>

void watch_cb(char *key, char *value, void *user_data){
    printf("watch callback is called...\n");
    printf("key: %s and value: %s \n", key, value);
}

void watch_prefix_cb(char *key, char *value, void *user_data){
    printf("watch_prefix callback is called...\n");
    printf("key: %s and value: %s \n", key, value);
    char *data = user_data;
    printf("userdata: %s", data);
}

int main(int argc, char** argv) { 
    set_log_level(LOG_LVL_INFO);
    
    config_t* config = json_config_new(argv[1]);
    kv_store_client_t* kv_store_client = NULL;
    void *handle = NULL;

    kv_store_client = create_kv_client(config);

    if (kv_store_client != NULL){
        handle = kv_store_client->init(kv_store_client);
    }

    if(handle == NULL)
        return -1;
    char *key_prefix = "/Video";

    config_value_t* values = kv_store_client->get_prefix(handle, key_prefix);

    if(values == NULL) {
        printf("No value found on prfix of key:%s\n", key_prefix);
    } else {
        config_value_t* value;
        for (int i = 0; i < config_value_array_len(values); i++) {
            value = config_value_array_get(values, i);
            printf("%s\n", value->body.string);
        }
        config_value_destroy(values);
    }
    char* val = kv_store_client->get(handle, "/VideoIngestion/config");

    if (val != NULL)
        printf("Value of key: /VideoIngestion/config is %s\n", val);

    int status1 = kv_store_client->put(handle, "/test", "hello world.....");

    int status2 = kv_store_client->put(handle, "/prefix_test", "Test prefix: hello world.....");

    if(status1 == 0 && status2 == 0){
        printf("put is successful\n");
    }

    kv_store_client->watch(handle, "/test", watch_cb, NULL);
    kv_store_client->watch_prefix(handle, "/prefix", watch_prefix_cb, (void*)"hello_user_data");
    sleep(10);

    kv_client_free(kv_store_client);
    return 0;
}

