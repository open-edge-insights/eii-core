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


#include <eis/config_manager/config_manager.h>
#include "eis/config_manager/env_config.h"

#define PUB "pub"
#define SUB "sub"

int main(){

    config_mgr_t* config_mgr_client = config_mgr_new("etcd", "", "", "");
   if (config_mgr_client == NULL){
      printf("Config manager client creation failed\n");
      return 0;
   }

   setenv("PubTopics","camera1_stream",1);
   setenv("SubTopics","Video/camera1_stream", 1);

    setenv("DEV_MODE", "true", 1);
    setenv("AppName", "publisher", 1);
    setenv("Clients", "publisher,VideoAnalytics", 1);
    setenv("camera1_stream_cfg", "zmq_tcp,127.0.0.1:65013", 1);

    env_config_t* env_config_client = env_config_new();

    char** pub_topics = env_config_client->get_topics_from_env(PUB);

    config_t* pub_config = env_config_client->get_messagebus_config(config_mgr_client, pub_topics[0], PUB);
    if(pub_config == NULL) {
        printf("Failed to get publisher message bus config\n");
    }
    else{
        printf("Getting Message bus publisher config is success !!\n");
    }

    char** sub_topics = env_config_client->get_topics_from_env(SUB);

    config_t* sub_config = env_config_client->get_messagebus_config(config_mgr_client, sub_topics[0], SUB);
    if(sub_config == NULL) {
        printf("Failed to get subscriber message bus config\n");
    }
    else{
        printf("Getting Message bus subscriber config is success !!\n");
    }

    env_config_destroy(env_config_client);

}