/*
Copyright (c) 2018 Intel Corporation.

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "DataBus.h"
#include <time.h>
#include <math.h>
#include <string.h>
#include <sys/time.h>

struct ContextConfig contextConfig;
struct TopicConfig topicConfig;

char* printTime() {

    int millisec;
    struct tm* tm_info;
    struct timeval tv;

    gettimeofday(&tv, NULL);

    millisec = tv.tv_usec / 1000.0;
    if (millisec >= 1000) {
        millisec -= 1000;
        tv.tv_sec++;
    }

    char buffer[26];
    static char timeStr[100];
    tm_info = localtime(&tv.tv_sec);
    strftime(buffer, 26, "%Y:%m:%d %H:%M:%S", tm_info);
    sprintf(timeStr, "%s.%03d", buffer, millisec);
    return timeStr;
}

void cb(char *topic, char* data, void *pyFunc) {
    printf("%s %s Data received: topic=%s and data=%s\n", __DATE__, printTime(), topic, data);
}

int main(int argc, char **argv) {

    if (argc < 8) {
        printf("Usage: <program> <cmd> <endpoint> <ns> <topic> <certificate> <private_key> [list_of_trusted_certs] where \n \
                cmd: PUB or SUB and \n \
                endpoint: opcua://localhost:65003 \n \
                ns: streammanager \n \
                topic: classifier_results \n \
                certificate: server or client certificate in der format \n \
                private_key: server or client private key\n \
                trusted_certs: list of trusted_certs");
        exit(-1);
    }
    
    char *errorMsg;
    contextConfig.direction = argv[1];
    contextConfig.endpoint = argv[2];
    contextConfig.ns = argv[3];
    topicConfig.name = argv[4];
    contextConfig.certificatePath = argv[5];
    contextConfig.privateKey = argv[6];
    contextConfig.trustList = NULL;

    size_t trustListSize = 0;
    
    if (argc > 7) {
        trustListSize = (size_t)argc-7;
    }

    contextConfig.trustList = (char **) malloc(trustListSize * sizeof(char *));
    // UA_STACKARRAY(UA_ByteString, trustList, trustListSize);

    for(size_t i = 0; i < trustListSize; i++)
        contextConfig.trustList[i] = (char*)argv[i+7];

    errorMsg = ContextCreate(contextConfig);
    if(strcmp(errorMsg, "0")) {
        printf("ContextCreate() API failed, error: %s\n", errorMsg);
        return -1;
    }
    printf("ContextCreate() for %s API successfully executed!\n", contextConfig.direction);

    if (!strcmp(contextConfig.direction, "PUB")) {
        char result[100];
        for (int i = 0; i < 10000; i++) {
            sprintf(result, "Hello %ld", i);
            errorMsg = Publish(topicConfig, result);
            if(strcmp(errorMsg, "0")) {
                printf("serverPublish() API failed, error: %s\n", errorMsg);
                return -1;
            }
            printf("%s %s Publishing [%s]\n", __DATE__,  printTime(), result);
        }
        ContextDestroy();
    } else if (!strcmp(contextConfig.direction, "SUB")) {

        errorMsg = Subscribe(topicConfig, "START", cb);
        if(strcmp(errorMsg, "0")) {
            printf("clientSubscribe() API failed, error: %s\n", errorMsg);
            return -1;
        }
        printf("clientSubscribe() API successfully executed!\n");
        sleep(120); /* sleep for 2 mins */
        ContextDestroy();
    }
}
