/*
Copyright (c) 2018 Intel Corporation.

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "DataBus.h"

static char gDirection[4];

/* ContextCreate function creates the opcua server/client (pub/sub) context based on `contextConfig.direction` field
 *
 * @param  contextConfig(struct)     ContextConfig structure for opcua publisher/subscriber
 *                                   publisher (server) - If all certs/keys are set to empty string, the opcua server starts in insecure mode. If
                                                          not, it starts in secure mode.
                                     subscriber (client)- If all certs/keys are set to empty string, the opcua client tries to establishes insecure connection.
                                                          If not, it tries to establish secure connection with the opcua server
 * @return Return a string "0" for success and other string for failure of the function
*/
char*
ContextCreate(struct ContextConfig contextConfig) {
    char *hostname;
    char *errorMsg = "0";
    int port;
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
            port = atol(hostNamePort[2]);
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

/* Publish function for publishing the data by opcua server process
 *
 * @param  topicConfig(struct)       opcua `struct TopicConfig` structure
 * @param  data(string)              data to be written to opcua variable
 * @return Return a string "0" for success and other string for failure of the function */
char*
Publish(struct TopicConfig topicConfig, char *data) {
    return serverPublish(topicConfig, data);
}

/* Subscribe function makes the subscription to the opcua variable named topic
 * @param  topicConfigs(array)       array of `struct TopicConfig` structure instances
 * @param  topicConfigCount(int)     length of topicConfigs array
 * @param  trig(string)              opcua trigger ex: START | STOP
 * @param  cb(c_callback)            callback that sends out the subscribed data back to the caller
 * @param  pyxFunc                   needed to callback pyx callback function to call the original python callback.
 *                                   For c and go callbacks, just pass NULL and nil respectively.
 *
 * @return Return a string "0" for success and other string for failure of the function */
char*
Subscribe(struct TopicConfig topicConfigs[], int topicConfigCount, char *trig, c_callback cb, void* pyxFunc) {
    return clientSubscribe(topicConfigs, topicConfigCount, cb, pyxFunc);
}

//ContextDestroy function destroys the opcua server/client context
void ContextDestroy() {
    if (!strcmp(gDirection, "PUB")) {
        serverContextDestroy();
    } else if (!strcmp(gDirection, "SUB")) {
        clientContextDestroy();
    }
}
