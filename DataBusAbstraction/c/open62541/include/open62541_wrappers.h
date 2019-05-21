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

// strcpy_s, strcat_s and strncpy_s extern declarations are required for safstringlib
extern int
strcpy_s(char *dest, unsigned int dmax, const char *src);

extern int
strcat_s(char *dest, unsigned int dmax, const char *src);

extern int
strncpy_s (char *dest, unsigned int dmax, const char *src, unsigned int slen);

#define FAILURE -1
#define SECURITY_POLICY_URI "http://opcfoundation.org/UA/SecurityPolicy#Basic256Sha256"
#define ENDPOINT_SIZE 100
#define NAMESPACE_SIZE 100
#define TOPIC_SIZE 100
// Setting this value to 60KB since influxdb supports a max size of 64KB
#define PUBLISH_DATA_SIZE 60*1024
#define DBA_STRCPY(dest, src) \
    { \
        unsigned int srcLength = (unsigned int)strlen(src) + 1; \
        unsigned int destSize = (unsigned int)sizeof(dest); \
        if (srcLength >= destSize) { \
	        strcpy_s(dest, destSize - 1, src); \
	    } else { \
	        strcpy_s(dest, srcLength, src); \
	    } \
    }

#define DBA_STRNCPY(dest, src, srclen) \
    { \
        unsigned int destSize = (unsigned int)sizeof(dest); \
        if (srclen >= destSize) { \
            strncpy_s(dest, destSize, src, destSize - 1); \
        } else { \
            strncpy_s(dest, srclen + 1 , src, srclen); \
        } \
    }


// opcua context config
struct ContextConfig {
    char *endpoint;         // opcua endpoint ex:opcua://localhost:65003
    char *direction;        // opcua direction ex: PUB|SUB
    char *certFile;         // opcua certificates path
    char *privateFile;      // opcua private key file
    char **trustFile;       // opcua trust files list
    size_t trustedListSize; // opcua trust files list size
};

// opcua topic config
struct TopicConfig {
    char *namespace; // opcua namespace name
    char *name;      // opcua topic name
    char *dType;     // type of topic, ex: string|int
};

//*************open62541 server wrappers**********************
char*
serverContextCreateSecured(char *hostname,
                    int port,
                    char *certificateFile,
                    char *privateKeyFile,
                    char **trustList,
                    size_t trustListSize);

char*
serverContextCreate(char *hostname,
                    int port);

char*
serverPublish(struct TopicConfig topicConfig,
              char *data);

void serverContextDestroy();

//*************open62541 client wrappers**********************

typedef void (*c_callback)(char *topic, char *data, void *pyxFunc);

char*
clientContextCreateSecured(char *hostname,
                           int port,
                           char *certificateFile,
                           char *privateKeyFile,
                           char **trustList,
                           size_t trustListSize);

char*
clientContextCreate(char *hostname,
                    int port);

char*
clientSubscribe(struct TopicConfig topicConfig[],
                int totalTopics,
                c_callback cb,
                void* pyxFunc);

void clientContextDestroy();
