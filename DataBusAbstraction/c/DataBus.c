/*
Copyright (c) 2018 Intel Corporation.

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "DataBus.h"

static char gDirection[4];

char*
ContextCreate(struct ContextConfig contextConfig) {
    char *hostname;
    char *errorMsg = "0";
    unsigned int port;
    bool devmode = false;
    DBA_STRCPY(gDirection, contextConfig.direction);
    char *hostNamePort[3];
    char *delimeter = "://";
    char *endpointStr = contextConfig.endpoint;
    char *endpointArr = strtok(endpointStr, delimeter);
    if(endpointArr != NULL)
    {
        for (int i = 0; endpointArr != NULL; i++) {
            hostNamePort[i] = endpointArr;
            endpointArr = strtok(NULL, delimeter);
        }
        if(hostNamePort[1] != NULL && hostNamePort[2] != NULL) {
            hostname = hostNamePort[1];
            port = atoi(hostNamePort[2]);
            port = abs(port);
            if((!strcmp(contextConfig.certFile, "")) && (!strcmp(contextConfig.privateFile, "")) \
            && (!strcmp(contextConfig.trustFile[0], ""))){
                devmode = true;
            }
            if (hostname != NULL) {
                if(devmode){
                    if(!strcmp(contextConfig.direction, "PUB")) {
                        errorMsg = serverContextCreate(hostname, port);
                    } else if(!strcmp(contextConfig.direction, "SUB")) {
                        errorMsg = clientContextCreate(hostname, port);
                    }
                } else {
                    if(!strcmp(contextConfig.direction, "PUB")) {
                        errorMsg = serverContextCreateSecured(hostname, port, contextConfig.certFile,
                                                              contextConfig.privateFile, contextConfig.trustFile,
                                                              contextConfig.trustedListSize);
                    } else if(!strcmp(contextConfig.direction, "SUB")) {
                        errorMsg = clientContextCreateSecured(hostname, port, contextConfig.certFile,
                                                              contextConfig.privateFile, contextConfig.trustFile,
                                                              contextConfig.trustedListSize);
                    }
                }
            }
        }
    }
    return errorMsg;
}

char*
Publish(struct TopicConfig topicConfig, const char *data) {
    return serverPublish(topicConfig, data);
}

char*
Subscribe(struct TopicConfig topicConfigs[], unsigned int topicConfigCount, const char *trig, c_callback cb, void* pyxFunc) {
    return clientSubscribe(topicConfigs, topicConfigCount, cb, pyxFunc);
}

void ContextDestroy() {
    if (!strcmp(gDirection, "PUB")) {
        serverContextDestroy();
    } else if (!strcmp(gDirection, "SUB")) {
        clientContextDestroy();
    }
}
