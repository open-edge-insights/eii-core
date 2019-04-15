/*
Copyright (c) 2018 Intel Corporation.

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "open62541_wrappers.h"
#include <time.h>
#include <math.h>
#include <sys/time.h>


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

    if (argc < 5) {
        printf("Usage: <program> <cmd> <certificate> <private_key> [list_of_trusted_certs] where \n \
                cmd: server or client and \n \
                certificate: server or client certificate in der format \n \
                private_key: server or client private key\n \
                trusted_certs: list of trusted_certs");
        exit(-1);
    }

    char *ns = "streammanager";
    char *hostname = "localhost";
    char *topic = "classifier_results";
    int port = 65003;
    char *errorMsg;
    char *certificatePath = argv[2];
    char *privateKey = argv[3];
    size_t trustListSize = 0;
    char **trustList = NULL;

    if (argc > 4) {
        trustListSize = (size_t)argc-4;
    }

    trustList = (char **) malloc(trustListSize * sizeof(char *));
    // UA_STACKARRAY(UA_ByteString, trustList, trustListSize);

    for(size_t i = 0; i < trustListSize; i++)
        trustList[i] = (char*)argv[i+4];

    if (!strcmp(argv[1], "server")) {
        if (!strcmp(certificatePath, "") && !strcmp(privateKey, "") && !strcmp(trustList[0], "")) {
            errorMsg = serverContextCreate(hostname, port);
            if(strcmp(errorMsg, "0")) {
                printf("serverContextCreate() API failed, error: %s\n", errorMsg);
                return -1;
            }
            printf("serverContextCreate() API successfully executed!\n");
        } else {
            errorMsg = serverContextCreateSecured(hostname, port, certificatePath, privateKey, trustList, trustListSize);
            if(strcmp(errorMsg, "0")) {
                printf("serverContextCreateSecured() API failed, error: %s\n", errorMsg);
                return -1;
            }
            printf("serverContextCreateSecured() API successfully executed!\n");
        }

        int nsIndex = serverStartTopic(ns, topic);
        if(nsIndex == 100) {
            printf("serverStartTopic() API failed\n");
            return -1;
        }
        printf("serverStartTopic() API successfully executed!, nsIndex: %d\n", nsIndex);

        char result[100];

        for (int i = 0; ; i++) {
            sprintf(result, "classifier_results %ld", i);
            errorMsg = serverPublish(nsIndex, topic, result);
            if(strcmp(errorMsg, "0")) {
                printf("serverPublish() API failed, error: %s\n", errorMsg);
                return -1;
            }
            printf("%s %s Publishing [%s]\n", __DATE__,  printTime(), result);
        }
        serverContextDestroy();
    } else if (!strcmp(argv[1], "client")) {
        if (!strcmp(certificatePath, "") && !strcmp(privateKey, "") && !strcmp(trustList[0], "")) {
            errorMsg = clientContextCreate(hostname, port);
            if(strcmp(errorMsg, "0")) {
                printf("clientContextCreate() API failed, error: %s\n", errorMsg);
                return -1;
            }
            printf("clientContextCreate() API successfully executed!\n");
        } else {
            errorMsg = clientContextCreateSecured(hostname, port, certificatePath, privateKey, trustList, trustListSize);
            if(strcmp(errorMsg, "0")) {
                printf("clientContextCreateSecured() API failed, error: %s\n", errorMsg);
                return -1;
            }
            printf("clientContextCreateSecured() API successfully executed!\n");
        }

        int nsIndex = clientStartTopic(ns, topic);
        if (nsIndex == 100) {
            printf("clienStartTopic() API failed!\n");
            return -1;
        }
        printf("clienStartTopic() API successfully executed!, nsIndex: %d\n", nsIndex);
        errorMsg = clientSubscribe(nsIndex, topic, cb, NULL);
        if(strcmp(errorMsg, "0")) {
            printf("clientSubscribe() API failed, error: %s\n", errorMsg);
            return -1;
        }
        printf("clientSubscribe() API successfully executed!\n");
        while (1) {
            sleep(60);
        }
        clientContextDestroy();
    }
}
