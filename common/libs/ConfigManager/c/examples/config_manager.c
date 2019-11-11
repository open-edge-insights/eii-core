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
 * @brief Config Manager example
 */

#include<stdio.h>
#include <string.h>
#include<unistd.h>
#include "eis/config_manager/config_manager.h"

void callback(char* key, char * val){
   printf("callback is called\n");
   printf("key: %s and value: %s\n", key, val);
}

void test_callback(char* key, char * val){
   printf("test_callback callback is called\n");
   printf("key: %s and value: %s\n", key, val);
}

int main() {
   config_mgr_t *config_mgr_client = config_mgr_new("etcd", "", "", "");
   if (config_mgr_client == NULL){
      printf("Config manager client creation failed\n");
      return;
   }
   char *value = config_mgr_client->get_config("/GlobalEnv/");
   printf("get_config is called, value is: %s \n", value);
   config_mgr_client->register_watch_key("/GlobalEnv/", callback);
   config_mgr_client->register_watch_dir("/Kapacitor/", test_callback);
   config_mgr_config_destroy(config_mgr_client);
   sleep(35);
   return 0;
}
