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
// Setting this value to 61KB since influxdb supports a max size of 64KB
#define PUBLISH_DATA_SIZE 61*1024
#define DBA_STRCPY(dest, src) \
    { \
        unsigned int srcLength = (unsigned int)strlen(src) + 1; \
        unsigned int destSize = (unsigned int)sizeof(dest); \
        if (srcLength >= PUBLISH_DATA_SIZE) { \
            destSize =  PUBLISH_DATA_SIZE; \
        } \
        if (srcLength >= destSize) { \
	        strcpy_s(dest, destSize - 1, src); \
        } else { \
            strcpy_s(dest, srcLength, src); \
        } \
    }

#define DBA_STRNCPY(dest, src, srclen) \
    { \
        unsigned int destSize = (unsigned int)sizeof(dest); \
        if (srclen >= PUBLISH_DATA_SIZE) { \
            destSize =  PUBLISH_DATA_SIZE; \
        } \
        if (srclen >= destSize) { \
            strncpy_s(dest, destSize, src, destSize - 1); \
        } else { \
            strncpy_s(dest, srclen + 1 , src, srclen); \
        } \
    }


// opcua context config
struct ContextConfig {
    char *endpoint;         ///< opcua endpoint ex:opcua://localhost:65003
    char *direction;        ///< opcua direction ex: PUB|SUB
    char *certFile;         ///< opcua certificates path
    char *privateFile;      ///< opcua private key file
    char **trustFile;       ///< opcua trust files list
    size_t trustedListSize; ///< opcua trust files list size
};

// opcua topic config
struct TopicConfig {
    char *ns;           ///< opcua namespace name
    char *name;         ///< opcua topic name
    char *dType;        ///< type of topic, ex: string|int
};

//*************open62541 server wrappers**********************/
/**serverContextCreateSecured function builds the server context and starts the opcua server in secure mode
 * @param  hostname(string)           hostname of the system where opcua server should run
 * @param  port(int)                  opcua port
 * @param  certificateFile(string)    server certificate file in .der format
 * @param  privateKeyFile(string)     server private key file in .der format
 * @param  trustedCerts(string array) list of trusted certs
 * @param  trustedListSize(int)       count of trusted certs
 * @return string "0" for success and other string for failure of the function */
char*
serverContextCreateSecured(char *hostname,
                    int port,
                    char *certificateFile,
                    char *privateKeyFile,
                    char **trustedCerts,
                    size_t trustedListSize);

/**serverContextCreate function builds the server context and starts the opcua server in insecure mode
 * @param  hostname(string)           hostname of the system where opcua server should run
 * @param  port(int)                  opcua port
 * @return string "0" for success and other string for failure of the function */
char*
serverContextCreate(char *hostname,
                    int port);

/**serverPublish creates the namespace if it doesn't exist, adds the opcua variable node (topic) 
 * in that namespace and writes **data** to the node
 * @param  topicConfig(struct)       opcua `struct TopicConfig` structure
 * @param  data(string)              data to be written to opcua variable
 * @return string "0" for success and other string for failure of the function */
char*
serverPublish(struct TopicConfig topicConfig,
              char *data);

/** serverContextDestroy function destroys the opcua server context */
void serverContextDestroy();

//*************open62541 client wrappers**********************

typedef void (*c_callback)(char *topic, char *data, void *pyxFunc);

/**clientContextCreateSecured function establishes secure connection with the opcua server
 * @param  hostname(string)           hostname of the system where opcua server is running
 * @param  port(uint)                 opcua port
 * @param  certificateFile(string)    client certificate file in .der format
 * @param  privateKeyFile(string)     client private key file in .der format
 * @param  trustedCerts(string array) list of trusted certs
 * @param  trustedListSize(int)       count of trusted certs
 * @return string "0" for success and other string for failure of the function */
char*
clientContextCreateSecured(char *hostname,
                           int port,
                           char *certificateFile,
                           char *privateKeyFile,
                           char **trustedCerts,
                           size_t trustedListSize);

/**clientContextCreate function establishes unsecure connection with the opcua server
 * @param  hostname(string)           hostname of the system where opcua server is running
 * @param  port(int)                  opcua port
 * @return string "0" for success and other string for failure of the function */
char*
clientContextCreate(char *hostname,
                    int port);

/**clientSubscribe function makes the subscription to the list of opcua variables (topics) in topicConfig array
 * @param  topicConfigs(array)       array of `struct TopicConfig` instances
 * @param  topicConfigCount(int)     length of topicConfigs array
 * @param  cb(c_callback)            callback that sends out the subscribed data back to the caller
 * @param  pyxFunc                   needed to callback pyx callback function to call the original python callback.
 *                                   For c and go callbacks, just puss NULL and nil respectively.
 * @return string "0" for success and other string for failure of the function */
char*
clientSubscribe(struct TopicConfig topicConfigs[],
                int topicConfigCount,
                c_callback cb,
                void* pyxFunc);

/**clientContextDestroy function destroys the opcua client context */
void clientContextDestroy();
