/*
Copyright (c) 2018 Intel Corporation.

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "DataBus.h"

#define NUM_OF_TOPICS 50

// opcua binding global variables
static char *gPubTopics[NUM_OF_TOPICS];
static char *gSubTopics[NUM_OF_TOPICS];
static int gPubIndex = 0;
static int gSubIndex = 0;
struct ContextConfig gContextConfig;
struct TopicConfig gTopicConfig;
static int gNamespaceIndex = 0;

static bool checkTopicExistence(char *topic);

/*ContextCreate function for
publisher (server) - If all certs/keys are set to empty string, the opcua server starts in insecure mode. If
                     not, it starts in secure mode.
subscriber (client) - If all certs/keys are set to empty string, the opcua client tries to establishes insecure connection.
                      If not, it tries to establish secure connection with the opcua server*/
char*
ContextCreate(struct ContextConfig contextConfig) {
    char *hostname;
    char *errorMsg = "0";
    int port;
    bool devmode = false;
    gContextConfig = contextConfig;
    char *hostNamePort[3];
    char *delimeter = "://";
    char *endpointStr = gContextConfig.endpoint;
    char *endpointArr = strtok(endpointStr, delimeter);
    for (int i = 0; endpointArr != NULL; i++) {
        hostNamePort[i] = endpointArr;
        endpointArr = strtok(NULL, delimeter);
    }
    hostname = hostNamePort[1];
    port = atol(hostNamePort[2]);

    if((!strcmp(gContextConfig.certFile, "")) && (!strcmp(gContextConfig.privateFile, "")) \
        && (!strcmp(gContextConfig.trustFile[0], ""))){
        devmode = true;
    }

    if(devmode){
        if(!strcmp(gContextConfig.direction, "PUB")) {
            errorMsg = serverContextCreate(hostname, port);
        } else if(!strcmp(gContextConfig.direction, "SUB")) {
            errorMsg = clientContextCreate(hostname, port);
        }
    } else {
        if(!strcmp(gContextConfig.direction, "PUB")) {
            errorMsg = serverContextCreateSecured(hostname, port, gContextConfig.certFile,
                        gContextConfig.privateFile, gContextConfig.trustFile,
                        gContextConfig.trustedListSize);
        } else if(!strcmp(gContextConfig.direction, "SUB")) {
            errorMsg = clientContextCreateSecured(hostname, port, gContextConfig.certFile,
                        gContextConfig.privateFile, gContextConfig.trustFile,
                        gContextConfig.trustedListSize);
        }
    }
    return errorMsg;
}

static bool
checkTopicExistence(char *topic) {
    for (int j = 0; j < NUM_OF_TOPICS; j++) {
        if (!strcmp(gContextConfig.direction, "PUB")) {
            if (gPubTopics[j] != NULL) {
                if (!strcmp(topic, gPubTopics[j])) {
                    return true;
                }
            }
        } else if (!strcmp(gContextConfig.direction, "SUB")) {
            if (gSubTopics[j] != NULL) {
                if (!strcmp(topic, gSubTopics[j])) {
                    return true;
                }
            }
        }
    }
    return false;
}

/* Publish function for publishing the data by opcua server process
 *
 * @param  TopicConfig(struct)       opcua topic config
 * @param  data(string)              data to be written to opcua variable
 * @return Return a string "0" for success and other string for failure of the function */
char*
Publish(struct TopicConfig topicConfig, char *data) {

    if (!checkTopicExistence(topicConfig.name)) {
        if (gPubIndex > (NUM_OF_TOPICS - 1)) {
            static char errStr[] = "Exceeded the limit of pub topics";
            UA_LOG_ERROR(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "%s", errStr);
            return errStr;
        }
        gPubTopics[gPubIndex] = topicConfig.name;
        gPubIndex++;

        gNamespaceIndex = serverStartTopic(gContextConfig.name, topicConfig.name);
        if (gNamespaceIndex == FAILURE) {
            static char errStr[] = "serverStartTopic() API failed";
            UA_LOG_ERROR(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "%s", errStr);
            return errStr;
        }
        gTopicConfig = topicConfig;
        UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "serverStartTopic() API successfully executed!, nsIndex: %d\n",
                    gNamespaceIndex);
    }

    return serverPublish(gNamespaceIndex, topicConfig.name, data);
}

/* Subscribe function makes the subscription to the opcua variable named topic
 * @param  TopicConfig(struct)       opcua topic config
 * @param  trig(string)              opcua trigger ex: START | STOP
 * @param  cb(c_callback)            callback that sends out the subscribed data back to the caller
 * @param  pyxFunc                   needed to callback pyx callback function to call the original python callback.
 *                                   For c and go callbacks, just pass NULL and nil respectively.
 *
 * @return Return a string "0" for success and other string for failure of the function */
char*
Subscribe(struct TopicConfig topicConfig, char *trig, c_callback cb, void* pyxFunc) {
    if (!checkTopicExistence(topicConfig.name)) {
        if (gSubIndex > (NUM_OF_TOPICS - 1)) {
            static char errStr[] = "Exceeded the limit of sub topics";
            UA_LOG_ERROR(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "%s", errStr);
            return errStr;
        }
        gSubTopics[gSubIndex] = topicConfig.name;
        gSubIndex++;
        gNamespaceIndex = clientStartTopic(gContextConfig.name, topicConfig.name);
        if (gNamespaceIndex == FAILURE) {
            static char errStr[] = "clientStartTopic() API failed";
            UA_LOG_ERROR(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "%s", errStr);
            return errStr;
        }
        UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "clientStartTopic() API successfully executed!, nsIndex: %d\n",
                    gNamespaceIndex);

    }

    return clientSubscribe(gNamespaceIndex, topicConfig.name, cb, pyxFunc);
}

//ContextDestroy function destroys the opcua server/client context
void ContextDestroy() {
    if (!strcmp(gContextConfig.direction, "PUB")) {
        serverContextDestroy();
    } else if (!strcmp(gContextConfig.direction, "SUB")) {
        clientContextDestroy();
    }
}
