/*
Copyright (c) 2018 Intel Corporation.

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "DataBus.h"

static UA_Logger logger = UA_Log_Stdout;
#define NUM_OF_TOPICS 50
// opcua binding global variables
static char *gPubTopics[NUM_OF_TOPICS];
static char *gSubTopics[NUM_OF_TOPICS];
static int gPubIndex = 0;
static int gSubIndex = 0;
struct ContextConfig gContextConfig;
struct TopicConfig gTopicConfig;

static bool checkTopicExistence(char *topic);

char*
ContextCreate(struct ContextConfig contextConfig) {
    char *hostname;
    int port;
    gContextConfig = contextConfig;
    char *hostNamePort[3];
    char *delimeter = "://";
    char *endpointStr = gContextConfig.endpoint;
    char *endpointArr = strtok(endpointStr, delimeter);
    for(int i=0; endpointArr != NULL; i++ ){   
        hostNamePort[i] = endpointArr;
        endpointArr = strtok(NULL, delimeter);  
    }
   
    hostname = hostNamePort[1];
    port = atol(hostNamePort[2]);

    if (!strcmp(gContextConfig.direction, "PUB")){
        serverContextCreate(hostname, port, gContextConfig.certificatePath, 
                            gContextConfig.privateKey, gContextConfig.trustList, 
                            gContextConfig.trustedListSize);
    }
    else if(!strcmp(gContextConfig.direction, "SUB")){
        clientContextCreate(hostname, port, gContextConfig.certificatePath, 
                            gContextConfig.privateKey, gContextConfig.trustList, 
                            gContextConfig.trustedListSize);
    }
}

static bool
checkTopicExistence(char *topic){
    for(int j = 0; j < NUM_OF_TOPICS; j++){
        if (!strcmp(gContextConfig.direction, "PUB")){
            if(gPubTopics[j] != NULL){
                if(!strcmp(topic, gPubTopics[j])){
                    return true;
                }
            }
        }
        else if(!strcmp(gContextConfig.direction, "SUB")){
            if(gSubTopics[j] != NULL){
                if(!strcmp(topic, gSubTopics[j])){
                    return true;
                }
            }

        }
    }
    return false;
}

char*
Publish(struct TopicConfig topicConfig, char *data){

    if (!checkTopicExistence(topicConfig.name)){
        if(gPubIndex > (NUM_OF_TOPICS-1)){
            static char errStr[] = "Exceeded the limit of pub topics";
            UA_LOG_ERROR(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "%s", errStr);
            return errStr;
        }
        gPubTopics[gPubIndex]=topicConfig.name;
        gPubIndex++;

        int nsIndex = serverStartTopic(gContextConfig.ns, topicConfig.name);
        topicConfig.nsIndex = nsIndex;
        if(topicConfig.nsIndex == 100) {
            static char errStr[] = "serverStartTopic() API failed";
            UA_LOG_ERROR(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "%s", errStr);
            return errStr;
        }
        gTopicConfig = topicConfig;
        UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "serverStartTopic() API successfully executed!, nsIndex: %d\n", 
                    gTopicConfig.nsIndex);
    }
    return serverPublish(gTopicConfig.nsIndex, gTopicConfig.name, data);
}

char*
Subscribe(struct TopicConfig topicConfig, char *trig, c_callback cb){

    if (!checkTopicExistence(topicConfig.name)){
        if(gSubIndex > (NUM_OF_TOPICS-1)){
            static char errStr[] = "Exceeded the limit of sub topics";
            UA_LOG_ERROR(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "%s", errStr);
            return errStr;
        }
        gSubTopics[gSubIndex]=topicConfig.name;
        gSubIndex++;

        topicConfig.nsIndex = clientStartTopic(gContextConfig.ns, topicConfig.name);
        if (topicConfig.nsIndex == 100) {
            static char errStr[] = "clientStartTopic() API failed";
            UA_LOG_ERROR(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "%s", errStr);
            return errStr;
        }
        UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "clientStartTopic() API successfully executed!, nsIndex: %d\n", 
                    topicConfig.nsIndex);

    }
    return clientSubscribe(topicConfig.nsIndex, topicConfig.name, cb, NULL);
}

void ContextDestroy()
{
    if (!strcmp(gContextConfig.direction, "PUB")){
        serverContextDestroy();
    }
    else if(!strcmp(gContextConfig.direction, "SUB")){
        clientContextDestroy();
    }
}
