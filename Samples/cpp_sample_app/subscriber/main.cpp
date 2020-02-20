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
 * @brief subscriber main program
 */

#include <signal.h>
#include <unistd.h>
#include "eis/msgbus/msgbus.h"
#include <eis/config_manager/env_config.h>
#include "subscriber.h"
#include "client.h"
#include <pthread.h>
#include "eis/utils/logger.h"

using namespace std;

pthread_t sub_thread ;
pthread_t client_thread ;
std::atomic<bool> *loop;
int start_subscriber(char *topic_name);
int start_client(char *service_name);

void signal_handler(int signo) 
{    loop->store(false,std::memory_order_relaxed);}

int main()
{
	signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

	loop = new std::atomic<bool>;
    *loop = true;
	if( (start_subscriber(getenv("SubTopics")) == 0) && (start_client(getenv("RequestEP")) == 0)) {
        pthread_join(sub_thread,NULL);
		pthread_join(client_thread,NULL);
    }

	delete loop;
	return 0;
}

int start_subscriber(char *topic_name)
{
	Subscriber *sub = new Subscriber(loop);
    int ret_val = -1;

	if(sub->init(topic_name)){
        if((ret_val = pthread_create(&sub_thread, NULL,Subscriber::start,sub)) != 0){
            LOG_ERROR("Unable to initialize subscriber thread, error code : %d", ret_val);
        }
        }else{
            delete sub;
            LOG_ERROR_0("Initializing subscriber object failed");
        }

        return ret_val;
}

int start_client(char *service_name)
{
	Client *cli = new Client(loop);
	int ret_val = -1;

	if(cli->init(service_name)){
		if((ret_val = pthread_create(&client_thread,NULL,Client::start,cli)) !=0){
			LOG_ERROR("Unable to initialize client thread, error code : %d", ret_val);
		}
	}else{
        delete cli;
		LOG_ERROR_0("Initializing client object failed");
		
	}
	return ret_val;
}
