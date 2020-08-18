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
#include "db_client.h"
#include <stdlib.h>

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

int main(){ 
    // Running in dev mode, provide 3r param: cert_file, 4th param: key_file and 5th param: root_file to run in prod mode
    db_client_t* db_client = create_etcd_client("localhost", "2379", "", "", "");

    void *handle = db_client->init(db_client);

    char* val = db_client->get(handle, "/VideoIngestion/config");
    printf("In main..., value: %s and len:%ld \n", val, strlen(val));

    int status = db_client->put(handle, "/test", "C hello world.....");

    if(status == 0){
        printf("put is successful\n");
    }

    db_client->watch(handle, "/Global", watch_cb, NULL);
    db_client->watch_prefix(handle, "/Global", watch_prefix_cb, (void*)"userdatawatchprefix");
    sleep(2);

    // db_client->db_destroy(handle);
    db_client_free(db_client);
    return 0;
}

