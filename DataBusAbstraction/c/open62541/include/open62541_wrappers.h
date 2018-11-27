/*
Copyright (c) 2018 Intel Corporation.

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <stdio.h>
#include "open62541.h"
#include "common.h"
#include <pthread.h>

//*************open62541 server wrappers**********************
char*
serverContextCreate(char *hostname,
                    int port,
                    char *certificateFile, 
                    char *privateKeyFile,
                    char **trustList,
                    size_t trustListSize);

int
serverStartTopic(char *nsName, 
                 char *topic);

char*
serverPublish(int nsIndex, 
              char *topic,
              char *data);

void serverContextDestroy();

//*************open62541 client wrappers**********************

typedef void (*c_callback)(char *topic, char *data, void *pyxFunc);

char*
clientContextCreate(char *hostname,
                    int port,
                    char *certificateFile,
                    char *privateKeyFile,
                    char **trustList,
                    size_t trustListSize);

int
clientStartTopic(char *nsName, 
                 char *topic);

char*
clientSubscribe(int nsIndex, 
                char* topic,
                c_callback cb,
                void* pyFunc);
                            
void clientContextDestroy();