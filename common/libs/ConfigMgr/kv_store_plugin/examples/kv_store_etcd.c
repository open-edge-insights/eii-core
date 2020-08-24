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
#include "kv_store_plugin.h"

#include <stdlib.h>
#include "eis/utils/json_config.h"

void watch_cb(char *key, char *value, void *user_data){
    printf("watch callback is called...\n");
    printf("key: %s and value: %s \n", key, value);
    char *data = user_data;
    printf("userdata: %s", data);
}

void watch_prefix_cb(char *key, char *value, void *user_data){
    printf("watch_prefix callback is called...\n");
    printf("key: %s and value: %s \n", key, value);
    char *data = user_data;
    printf("userdata: %s", data);
}

int main(int argc, char** argv) { 
    // Running in dev mode, provide 3r param: cert_file, 4th param: key_file and 5th param: root_file to run in prod mode
    config_t* config = json_config_new(argv[1]);
    kv_store_client_t* kv_store_client = create_kv_client(config);

    void *handle = kv_store_client->init(kv_store_client);

    char* val = kv_store_client->get(handle, "/VideoIngestion/config");
    printf("Value of key: /VideoIngestion/config is %s\n", val);

    int status = kv_store_client->put(handle, "/test", "hello world.....");

    if(status == 0){
        printf("put is successful\n");
    }

    kv_store_client->watch(handle, "/test", watch_cb, NULL);
    kv_store_client->watch_prefix(handle, "/te", watch_prefix_cb, (void*)"userdatawatchprefix");
    sleep(2);

    kv_client_free(kv_store_client);
    return 0;
}

