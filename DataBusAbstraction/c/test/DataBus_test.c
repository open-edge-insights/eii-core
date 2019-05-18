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

static char* printTime() {

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

    if (argc < 7) {
        printf("Usage: <program> <direction> <endpoint> <ns> <topics> <certificate> <private_key> [list_of_trusted_certs] where \n \
                direction: PUB or SUB and \n \
                endpoint: opcua://localhost:65003 \n \
                ns: streammanager \n \
                topics: list of topics separated by comma (Eg: cam_serial1_results, cam_serial2_results) \n \
                certificate: server or client certificate in der format \n \
                private_key: server or client private key in der format \n \
                trusted_certs: single or list of trusted_certs in .der format");
        exit(-1);
    }

    char *errorMsg;
    contextConfig.direction = argv[1];
    contextConfig.endpoint = argv[2];
    char *namespace = argv[3];
    char *topics = argv[4];
    contextConfig.certFile = argv[5];
    contextConfig.privateFile = argv[6];
    contextConfig.trustFile = NULL;

    size_t trustFileSize = 0;
    if (argc >= 8) {
        trustFileSize = (size_t)argc - 7;
    }

    contextConfig.trustFile = (char **) malloc(trustFileSize * sizeof(char *));
    contextConfig.trustedListSize = trustFileSize;
    for(size_t i = 0; i < trustFileSize; i++)
        contextConfig.trustFile[i] = (char*)argv[i+7];

    /* First tokenization here is to get the number of topics */
    char delim[] = ",";
    char tempTopics[TOPIC_SIZE * 100];
    DBA_STRCPY(tempTopics, topics);
    printf("\ntempTopics: %s\n", tempTopics);
	char *token = strtok(topics, delim);
    int totalTopics = 0;
	while(token != NULL) {
        totalTopics++;
		token = strtok(NULL, delim);
	}
    struct TopicConfig *topicConfigs = (struct TopicConfig*)malloc(totalTopics * sizeof(struct TopicConfig));

    /* Second tokenization here is to populate the topicConfigs array */
    char *topic = strtok(tempTopics, delim);
    char *topicDataType = "string";
    int i = 0;
    int namespaceLen = strlen(namespace) + 1;
    int topicLen = 0;
    int dTypeLen = strlen(topicDataType) + 1;
	while(topic != NULL) {
        topicConfigs[i].namespace = (char*)malloc(namespaceLen);
        strcpy_s(topicConfigs[i].namespace, namespaceLen, namespace);

        topicLen = strlen(topic) + 1;
        topicConfigs[i].name = (char*)malloc(topicLen);
        strcpy_s(topicConfigs[i].name, topicLen, topic);

        topicConfigs[i].dType = (char*)malloc(dTypeLen);
        strcpy_s(topicConfigs[i].dType, dTypeLen, topicDataType);
        i++;
		topic = strtok(NULL, delim);
	}

    errorMsg = ContextCreate(contextConfig);
    if(strcmp(errorMsg, "0")) {
        printf("ContextCreate() API failed, error: %s\n", errorMsg);
        return -1;
    }
    printf("ContextCreate() for %s API successfully executed!\n", contextConfig.direction);

    if (!strcmp(contextConfig.direction, "PUB")) {
        char result[100];
        for (int i = 0; i < 100 ; i++) {
            for (int j = 0; j < totalTopics; j++) {
                if (topicConfigs[j].name != NULL) {
                    sprintf(result, "%s %d", topicConfigs[j].name, i);
                    errorMsg = Publish(topicConfigs[j], result);
                    if(strcmp(errorMsg, "0")) {
                        printf("serverPublish() API failed, error: %s\n", errorMsg);
                        return -1;
                    }
                    printf("%s %s Publishing [%s]\n", __DATE__,  printTime(), result);
                    sleep(1);
                }
            }
        }
    } else if (!strcmp(contextConfig.direction, "SUB")) {
        errorMsg = Subscribe(topicConfigs, totalTopics, "START", cb, NULL);
        if(strcmp(errorMsg, "0")) {
            printf("clientSubscribe() API failed, error: %s\n", errorMsg);
            return -1;
        }
        printf("clientSubscribe() API successfully executed!\n");
        sleep(60 * 2);
    }
    ContextDestroy();
    printf("ContextDestroy() for %s API successfully executed!\n", contextConfig.direction);

}
