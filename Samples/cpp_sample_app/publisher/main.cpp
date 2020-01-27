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
 * @brief publisher main program
 */

#include <signal.h>
#include <unistd.h>
#include <eis/config_manager/env_config.h>
#include "eis/msgbus/msgbus.h"
#include "publisher.h"
#include "server.h"
#include <pthread.h>
#include "eis/utils/logger.h"
#include <usage.h>

using namespace std;

pthread_t pub_thread ;
pthread_t server_thread ;
std::atomic<bool> *loop;
int start_publisher(char *topic_name);
int start_server(char *service_name);

void signal_handler(int signo) 
{    *loop = false;}

int main(int argc, char** argv)
{
    if(argc == 1) {
        LOG_ERROR_0("Too few arguments");
        return -1;
    } else if(argc > 3) {
        LOG_ERROR_0("Too many arguments");
        return -1;
    }

    if(strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
        usage(argv[0]);
        return 0;
    }

    set_log_level(LOG_LVL_DEBUG);
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    loop = new std::atomic<bool>;
    *loop = true;

    if( (start_publisher("publish_test") == 0) && (start_server("cppPublisher") == 0) ) {
        pthread_join(pub_thread,NULL);
        pthread_join(server_thread,NULL);
    }

    delete loop;
    return 0;
}

int start_publisher(char *topic_name)
{
    publisher *pub = new publisher(loop);
    int ret_val = -1;

    if(pub->init(topic_name)){
        if((ret_val = pthread_create(&pub_thread, NULL,publisher::start,pub)) != 0){
            LOG_ERROR("Unable to initialize publisher thread, error code : %d", ret_val);
        }
    }else{
        delete pub;
        LOG_ERROR_0("Initializing publisher object failed");
    }

    return ret_val;
}

int start_server(char *service_name)
{
    server *ser = new server(loop);
	int ret_val = -1;

    if(ser->init(service_name)){
        if((ret_val = pthread_create(&server_thread, NULL,server::start,ser)) != 0){
            LOG_ERROR("Unable to initialize server thread, error code : %d", ret_val);
        }
    }else{
            delete ser;
            LOG_ERROR_0("Initializing server object failed");
    }

    return ret_val;
}


