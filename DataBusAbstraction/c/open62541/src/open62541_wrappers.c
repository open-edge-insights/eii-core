/*
Copyright (c) 2018 Intel Corporation.

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <unistd.h>
#include "open62541_wrappers.h"

#define FAILURE 100
#define SECURITY_POLICY_URI "http://opcfoundation.org/UA/SecurityPolicy#Basic256Sha256"
#define ENDPOINT_SIZE 100
#define NAMESPACE_SIZE 100
#define TOPIC_SIZE 100
// TODO: we need to see what's the optimum size for this or have a better logic to handle this regardless of any size limits
// imposed by safestringlib strcpy_s API
#define PUBLISH_DATA_SIZE 4*1024
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

// opcua server global variables
static UA_Server *server = NULL;
static UA_ServerConfig *serverConfig;
static UA_ByteString* remoteCertificate = NULL;
static UA_Boolean serverRunning = true;
static pthread_mutex_t *serverLock = NULL;

// opcua client global variables
static UA_Client *client = NULL;
static UA_ClientConfig* clientConfig;
static char lastSubscribedTopic[TOPIC_SIZE] = "";
static int lastNamespaceIndex = FAILURE;
static char endpoint[ENDPOINT_SIZE];
static c_callback userCallback;
static char dataToPublish[PUBLISH_DATA_SIZE];
static void *userFunc = NULL;
static bool clientExited = false;

//*************open62541 common wrappers**********************

// TODO: this seems to not work especially when there are
// format specifiers, needs to be debugged
/*
static void
logMsg(const char *msg, ...) {
    va_list args; va_start(args, msg);
    UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, msg, args);
    va_end(args);
}
*/

// Converts the passed int to string
static char* convertToString(char str[], int num) {
    int i, rem, len = 0, n;

    n = num;
    while (n != 0) {
        len++;
        n /= 10;
    }
    for (i = 0; i < len; i++) {
        rem = num % 10;
        num = num / 10;
        str[len - (i + 1)] = rem + '0';
    }
    str[len] = '\0';
    return str;
}

static UA_Int16
getNamespaceIndex(char *ns, char *topic) {
    UA_LOG_DEBUG(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "Browsing nodes in objects folder:\n");
    UA_BrowseRequest bReq;
    UA_BrowseRequest_init(&bReq);
    bReq.requestedMaxReferencesPerNode = 0;
    bReq.nodesToBrowse = UA_BrowseDescription_new();
    bReq.nodesToBrowseSize = 1;
    bReq.nodesToBrowse[0].nodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER); /* browse objects folder */
    bReq.nodesToBrowse[0].resultMask = UA_BROWSERESULTMASK_ALL; /* return everything */
    UA_BrowseResponse bResp = UA_Client_Service_browse(client, bReq);
    UA_LOG_DEBUG(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "%-9s %-16s %-16s %-16s\n", "NAMESPACE", "NODEID", "BROWSE NAME", "DISPLAY NAME");
    for(size_t i = 0; i < bResp.resultsSize; ++i) {
        for(size_t j = 0; j < bResp.results[i].referencesSize; ++j) {
            UA_ReferenceDescription *ref = &(bResp.results[i].references[j]);
            if(ref->nodeId.nodeId.identifierType == UA_NODEIDTYPE_STRING) {
                UA_LOG_DEBUG(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "%-9d %-16.*s %-16.*s %-16.*s\n", ref->nodeId.nodeId.namespaceIndex,
                       (int)ref->nodeId.nodeId.identifier.string.length,
                       ref->nodeId.nodeId.identifier.string.data,
                       (int)ref->browseName.name.length, ref->browseName.name.data,
                       (int)ref->displayName.text.length, ref->displayName.text.data);
                char nodeIdentifier[NAMESPACE_SIZE];
                DBA_STRCPY(nodeIdentifier, ref->nodeId.nodeId.identifier.string.data);
                UA_Int16 nodeNs = ref->nodeId.nodeId.namespaceIndex;
                if (!strcmp(topic, nodeIdentifier)) {
                    UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "Node Id exists !!!\n");
                    UA_BrowseRequest_deleteMembers(&bReq);
                    UA_BrowseResponse_deleteMembers(&bResp);
                    return nodeNs;
                }
            }
        }
    }
    UA_BrowseRequest_deleteMembers(&bReq);
    UA_BrowseResponse_deleteMembers(&bResp);
    return FAILURE;
}

//*************open62541 server wrappers**********************
/* This function provides data to the subscriber */
static UA_StatusCode
readPublishedData(UA_Server *server,
                const UA_NodeId *sessionId,
                void *sessionContext,
                const UA_NodeId *nodeId, void *nodeContext,
                UA_Boolean sourceTimeStamp, const UA_NumericRange *range,
                UA_DataValue *data) {
    UA_LOG_DEBUG(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,
                     "In readPublishedData() function");
    data->hasValue = true;
    UA_String str = UA_STRING(dataToPublish);
    UA_Variant_setScalarCopy(&data->value, &str, &UA_TYPES[UA_TYPES_STRING]);
	return UA_STATUSCODE_GOOD;
}

static UA_StatusCode
writePublishedData(UA_Server *server,
                 const UA_NodeId *sessionId, void *sessionContext,
                 const UA_NodeId *nodeId, void *nodeContext,
                 const UA_NumericRange *range, const UA_DataValue *data) {

    return UA_STATUSCODE_GOOD;
}

static UA_UInt16
addTopicDataSourceVariable(UA_Server *server, char *namespace, char *topic) {

    UA_UInt16 namespaceIndex = UA_Server_addNamespace(server, namespace);
    if (namespaceIndex == 0) {
        UA_LOG_FATAL(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "UA_Server_addNamespace has failed");
        return FAILURE;
    }
    UA_LOG_DEBUG(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "%u:namespaceIndex created for namespace: %s\n", namespaceIndex, namespace);

    UA_VariableAttributes attr = UA_VariableAttributes_default;
    attr.description = UA_LOCALIZEDTEXT("en-US", topic);
    attr.displayName = UA_LOCALIZEDTEXT("en-US", topic);
    attr.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;

    /* Add the variable node to the information model */
    UA_NodeId currentNodeId = UA_NODEID_STRING(namespaceIndex, topic);
    UA_QualifiedName currentName = UA_QUALIFIEDNAME(namespaceIndex, topic);
    UA_NodeId parentNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER);
    UA_NodeId parentReferenceNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES);
    UA_NodeId variableTypeNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE);

    UA_DataSource topicDataSource;
    topicDataSource.read = readPublishedData;
    topicDataSource.write = writePublishedData;
    UA_Server_addDataSourceVariableNode(server, currentNodeId, parentNodeId,
                                        parentReferenceNodeId, currentName,
                                        variableTypeNodeId, attr,
                                        topicDataSource, NULL, NULL);
    return namespaceIndex;
}

/* cleanupServer deletes the memory allocated for server configuration */
static void
cleanupServer() {
    if (server) {
        UA_Server_delete(server);
    }
    if (serverConfig) {
        UA_ServerConfig_delete(serverConfig);
    }
    if (serverLock) {
        pthread_mutex_destroy(serverLock);
        free(serverLock);
    }
}

static void*
startServer(void *ptr) {

    /* run server */
    UA_StatusCode retval = UA_Server_run_startup(server);
    if (retval != UA_STATUSCODE_GOOD) {
        UA_LOG_FATAL(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "\nServer failed to start, error: %s", UA_StatusCode_name(retval));
        return NULL;
    }

    UA_UInt16 timeout;
    while (serverRunning) {
        pthread_mutex_lock(serverLock);
        /* timeout is the maximum possible delay (in millisec) until the next
        _iterate call. Otherwise, the server might miss an internal timeout
        or cannot react to messages with the promised responsiveness. */
        timeout = UA_Server_run_iterate(server, false);
        pthread_mutex_unlock(serverLock);

        /* Now we can use the max timeout to do something else. In this case, we
        just sleep. (select is used as a platform-independent sleep
        function.) */
        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = timeout * 1000;
        select(0, NULL, NULL, NULL, &tv);
    }
    UA_Server_run_shutdown(server);
    return NULL;
}

/* serverContextCreateSecured function creates the opcua namespace, opcua variable and starts the server
 *
 * @param  hostname(string)           hostname of the system where opcua server should run
 * @param  port(uint)                 opcua port
 * @param  certificateFile(string)    server certificate file in .der format
 * @param  privateKeyFile(string)     server private key file in .der format
 * @param  trustedCerts(string array) list of trusted certs
 * @param  trustedListSize(int)       count of trusted certs
 *
 * @return Return a string "0" for success and other string for failure of the function */
char*
serverContextCreateSecured(char *hostname, int port, char *certificateFile, char *privateKeyFile, char **trustedCerts, size_t trustedListSize) {

    /* Load certificate and private key */
    UA_ByteString certificate = loadFile(certificateFile);
    if(certificate.length == 0) {
        static char str[] = "Unable to load certificate file";
        UA_LOG_FATAL(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,
            "%s", str);
        return str;
    }
    UA_ByteString privateKey = loadFile(privateKeyFile);
    if(privateKey.length == 0) {
        static char str[] = "Unable to load private key file";
        UA_LOG_FATAL(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,
            "%s", str);
        return str;
    }

    /* Load the trustlist */
    UA_STACKARRAY(UA_ByteString, trustList, trustedListSize);

    for(size_t i = 0; i < trustedListSize; i++) {
        trustList[i] = loadFile(trustedCerts[i]);
        if(trustList[i].length == 0) {
            static char str[] = "Unable to load trusted cert file";
            UA_LOG_FATAL(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,
                "%s", str);
            return str;
        }
    }

    /* Loading of a revocation list currently unsupported */
    UA_ByteString *revocationList = NULL;
    size_t revocationListSize = 0;

    /* Initiate server config */
    serverConfig =
        UA_ServerConfig_new_basic256sha256(port, &certificate, &privateKey,
                                          trustList, trustedListSize,
                                          revocationList, revocationListSize);
    if(!serverConfig) {
        static char str[] = "Could not create the server config";
        UA_LOG_FATAL(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "%s",
            str);
        return str;
    }
    UA_ServerConfig_set_customHostname(serverConfig, UA_STRING(hostname));

    UA_DurationRange range = {5.0, 5.0};
    serverConfig->publishingIntervalLimits = range;
    serverConfig->samplingIntervalLimits = range;

    /* Initiate server instance */
    server = UA_Server_new(serverConfig);
    if(server == NULL) {
        static char str[] = "UA_Server_new() API failed";
        UA_LOG_FATAL(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "%s",
            str);
        return str;
    }

    /* Creation of mutex for server instance */
    serverLock = malloc(sizeof(pthread_mutex_t));

    if (!serverLock || pthread_mutex_init(serverLock, NULL) != 0) {
        static char str[] = "server lock mutex init has failed!";
        UA_LOG_FATAL(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "%s", str);
        return str;
    }

    pthread_t serverThread;
    if (pthread_create(&serverThread, NULL, startServer, NULL)) {
        static char str[] = "server pthread creation to start server failed";
        UA_LOG_FATAL(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "%s",
            str);
        return str;
    }
    return "0";
}
/* serverContextCreate function creates the opcua namespace, opcua variable and starts the server
 * in insecure mode
 *
 * @param  hostname(string)           hostname of the system where opcua server should run
 * @param  port(uint)                 opcua port
 *
 * @return Return a string "0" for success and other string for failure of the function */
char*
serverContextCreate(char *hostname, int port) {

    /* Initiate server config */
    serverConfig = UA_ServerConfig_new_minimal(port, NULL);;
    if(!serverConfig) {
        static char str[] = "Could not create the server config";
        UA_LOG_FATAL(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "%s",
            str);
        return str;
    }
    UA_ServerConfig_set_customHostname(serverConfig, UA_STRING(hostname));

    UA_DurationRange range = {5.0, 10.0};
    serverConfig->publishingIntervalLimits = range;
    serverConfig->samplingIntervalLimits = range;

    /* Initiate server instance */
    server = UA_Server_new(serverConfig);
    if(server == NULL) {
        static char str[] = "UA_Server_new() API failed";
        UA_LOG_FATAL(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "%s",
            str);
        return str;
    }

    /* Creation of mutex for server instance */
    serverLock = malloc(sizeof(pthread_mutex_t));

    if (!serverLock || pthread_mutex_init(serverLock, NULL) != 0) {
        static char str[] = "server lock mutex init has failed!";
        UA_LOG_FATAL(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "%s", str);
        return str;
    }

    pthread_t serverThread;
    if (pthread_create(&serverThread, NULL, startServer, NULL)) {
        static char str[] = "server pthread creation to start server failed";
        UA_LOG_FATAL(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "%s",
            str);
        return str;
    }
    return "0";
}

/* serverStartTopic function creates the opcua variable for the topic
 *
 * @param  namespace(string)         opcua namespace name
 * @param  topic(string)             name of the opcua variable (aka topic name)
 *
 * @return Return namespace index for success and 100 for failure */
int
serverStartTopic(char *namespace, char *topic) {

    /* check if server is started or not */
    if (server == NULL) {
        static char str[] = "UA_Server instance is not instantiated";
        UA_LOG_FATAL(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "%s",
            str);
        return FAILURE;
    }

    //TODO: check if node already exists or not before trying to add

    /* add datasource variable */
    UA_Int16 nsIndex = addTopicDataSourceVariable(server, namespace, topic);
    if (nsIndex == FAILURE) {
        static char str[] = "Failed to add topic data source variable node";
        UA_LOG_FATAL(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "%s",
            str);
        return FAILURE;
    }
    return nsIndex;
}

/* serverPublish function for publishing the data by opcua server process
 *
 * @param  nsIndex(int)              opcua namespace index of the variable node
 * @param  topic(string)             name of the opcua variable (aka topic name)
 * @param  data(string)              data to be written to opcua variable
 *
 * @return Return a string "0" for success and other string for failure of the function */
char*
serverPublish(int nsIndex, char *topic, char* data) {

    /* check if server is started or not */
    if (server == NULL) {
        static char str[] = "UA_Server instance is not instantiated";
        UA_LOG_FATAL(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "%s",
            str);
        return str;
    }

    /* writing the data to the opcua variable */
    UA_Variant *val = UA_Variant_new();
    DBA_STRCPY(dataToPublish, data);
    UA_String str = UA_STRING(data);
    UA_Variant_setScalarCopy(val, &str, &UA_TYPES[UA_TYPES_STRING]);
    UA_LOG_DEBUG(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "nsIndex: %d, topic:%s\n", nsIndex, topic);

    /*sleep for mininum publishing interval in ms*/
    UA_sleep_ms((int)serverConfig->publishingIntervalLimits.min);

    pthread_mutex_lock(serverLock);
    UA_StatusCode retval = UA_Server_writeValue(server, UA_NODEID_STRING(nsIndex, topic), *val);
    pthread_mutex_unlock(serverLock);

    if (retval == UA_STATUSCODE_GOOD) {
        UA_Variant_delete(val);
        return "0";
    }
    UA_LOG_FATAL(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "%s", UA_StatusCode_name(retval));
    UA_Variant_delete(val);
    return UA_StatusCode_name(retval);
}

/* serverContextDestroy function destroys the opcua server context */
void serverContextDestroy() {
    cleanupServer();
}

//*************open62541 client wrappers**********************

/* cleanupClient deletes the memory allocated for client configuration */
static void
cleanupClient() {
    if (remoteCertificate) {
        UA_ByteString_delete(remoteCertificate);
    }
    if (client) {
        UA_Client_delete(client);
    }
}

static void
deleteSubscriptionCallback(UA_Client *client, UA_UInt32 subscriptionId, void *subscriptionContext) {
    UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "Subscription Id %u was deleted", subscriptionId);
}

static void
subscriptionInactivityCallback (UA_Client *client, UA_UInt32 subId, void *subContext) {
    UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "Inactivity for subscription %u", subId);
}

static void
subscriptionCallback(UA_Client *client, UA_UInt32 subId, void *subContext,
                        UA_UInt32 monId, void *monContext, UA_DataValue *data) {
    UA_Variant *value = &data->value;
    if(UA_Variant_isScalar(value)) {
        if (value->type == &UA_TYPES[UA_TYPES_STRING]) {
            UA_String str = *(UA_String*)value->data;
            if (userCallback) {
                static char subscribedData[PUBLISH_DATA_SIZE];
                DBA_STRCPY(subscribedData, str.data);
                // TODO: Have better logic here to check the mapping between topics and
                // their corresponding published data. Better to use DataBus wrappers in C
                // and call the same in py and go to support multi pub/sub feature (needs some work)
                if (strstr(subscribedData, lastSubscribedTopic) != NULL)
                    userCallback(lastSubscribedTopic, subscribedData, userFunc);
            } else {
                UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "userCallback is NULL");
            }
        }
    }
}

/* creates the subscription for the opcua variable with topic name */
static UA_Int16
createSubscription(int nsIndex, char *topic, c_callback cb) {

    UA_CreateSubscriptionRequest request = UA_CreateSubscriptionRequest_default();
    request.requestedPublishingInterval = 0;

    UA_CreateSubscriptionResponse response = UA_Client_Subscriptions_create(client, request,
                                                                            NULL, NULL, deleteSubscriptionCallback);

    if(response.responseHeader.serviceResult == UA_STATUSCODE_GOOD) {
        UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "Create subscription succeeded, id %u", response.subscriptionId);
        UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "Subscription's revisedPublishingInterval: %u ms", response.revisedPublishingInterval);
    } else {
        UA_LOG_FATAL(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "Create subscription failed");
        return 1;
    }

    if (topic == NULL) {
        UA_LOG_FATAL(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "topic is NULL!");
        return 1;
    }

    UA_NodeId nodeId = UA_NODEID_STRING(nsIndex, topic);

    UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "ns: %d, identifier: %.*s\n", nodeId.namespaceIndex,
        (int)nodeId.identifier.string.length,
        nodeId.identifier.string.data);

    /* Add a MonitoredItem */
    UA_MonitoredItemCreateRequest monRequest =
        UA_MonitoredItemCreateRequest_default(nodeId);

    UA_MonitoredItemCreateResult monResponse =
        UA_Client_MonitoredItems_createDataChange(client, response.subscriptionId,
                                                    UA_TIMESTAMPSTORETURN_BOTH,
                                                    monRequest, NULL, subscriptionCallback, NULL);
    if(monResponse.statusCode == UA_STATUSCODE_GOOD) {
        /* house keeping of last subscribed topic */
        if (cb != NULL) {
            UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "callback(cb) is not NULL\n");
            userCallback = cb;
        }
        UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,
            "Monitoring NODEID(%d, %.*s),id %u\n",
            nodeId.namespaceIndex,
            (int)nodeId.identifier.string.length,
            nodeId.identifier.string.data, monResponse.monitoredItemId);
        if (topic != NULL)
            DBA_STRCPY(lastSubscribedTopic, topic);
        if (nsIndex != FAILURE)
            lastNamespaceIndex = nsIndex;
        return 0;
    }
    return 1;
}

static void stateCallback(UA_Client *client, UA_ClientState clientState) {
    switch(clientState) {
        case UA_CLIENTSTATE_WAITING_FOR_ACK:
            UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "The client is waiting for ACK");
            break;
        case UA_CLIENTSTATE_DISCONNECTED:
            UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "The client is disconnected");
            break;
        case UA_CLIENTSTATE_CONNECTED:
            UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "A TCP connection to the server is open");
            break;
        case UA_CLIENTSTATE_SECURECHANNEL:
            UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "A SecureChannel to the server is open");
            break;
        case UA_CLIENTSTATE_SESSION:
            UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "A session with the server is open");
            /* recreating the subscription upon opcua server connect */
            if (lastNamespaceIndex != FAILURE) {
                if (createSubscription(lastNamespaceIndex, lastSubscribedTopic, NULL) == 1)
                    UA_LOG_FATAL(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "Subscription failed!");
            }
            break;
        case UA_CLIENTSTATE_SESSION_RENEWED:
            /* The session was renewed. We don't need to recreate the subscription. */
            UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "A session with the server is open (renewed)");
            break;
        case UA_CLIENTSTATE_SESSION_DISCONNECTED:
            UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "Session disconnected");
            break;
    }
    return;
}

/* clientContextCreate function establishes secure connection with the opcua server
 *
 * @param  hostname(string)           hostname of the system where opcua server is running
 * @param  port(uint)                 opcua port
 * @param  certificateFile(string)    client certificate file in .der format
 * @param  privateKeyFile(string)     client private key file in .der format
 * @param  trustedCerts(string array) list of trusted certs
 * @param  trustedListSize(int)       count of trusted certs
 *
 * @return Return a string "0" for success and other string for failure of the function */
char*
clientContextCreateSecured(char *hostname, int port, char *certificateFile, char *privateKeyFile, char **trustedCerts, size_t trustedListSize) {

    UA_StatusCode retval = UA_STATUSCODE_GOOD;
    UA_ByteString *revocationList = NULL;
    size_t revocationListSize = 0;

    /* endpointArray holds the available endpoints in the server
     * and endpointArraySize holds the number of endpoints available */
    UA_EndpointDescription* endpointArray = NULL;
    size_t endpointArraySize = 0;

    /* Load certificate and private key */
    UA_ByteString certificate = loadFile(certificateFile);
    if(certificate.length == 0) {
        static char str[] = "Unable to load certificate file";
        UA_LOG_FATAL(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,
            "%s", str);
        return str;
    }
    UA_ByteString privateKey = loadFile(privateKeyFile);
    if(privateKey.length == 0) {
        static char str[] = "Unable to load private key file";
        UA_LOG_FATAL(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,
            "%s", str);
        return str;
    }

    client = UA_Client_new();
    if(client == NULL) {
        static char str[] = "UA_Client_new() API returned NULL";
        UA_LOG_FATAL(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,
            "%s", str);
        return str;
    }

    remoteCertificate = UA_ByteString_new();

    char portStr[10];
    DBA_STRCPY(endpoint, "opc.tcp://");
    strcat_s(endpoint, ENDPOINT_SIZE, hostname);
    strcat_s(endpoint, ENDPOINT_SIZE, ":");
    strcat_s(endpoint, ENDPOINT_SIZE, convertToString(portStr, port));

    UA_STACKARRAY(UA_ByteString, trustList, trustedListSize);
    for(size_t trustListCount = 0; trustListCount < trustedListSize; trustListCount++) {
        trustList[trustListCount] = loadFile(trustedCerts[trustListCount]);
          if(trustList[trustListCount].length == 0) {
            static char str[] = "Unable to load trusted cert file";
            UA_LOG_FATAL(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,
                "%s", str);
            return str;
        }
    }

    clientConfig = UA_Client_getConfig(client);
    clientConfig->securityMode = UA_MESSAGESECURITYMODE_SIGNANDENCRYPT;
    UA_ClientConfig_setDefaultEncryption(clientConfig, certificate, privateKey,
                                         trustList, trustedListSize,
                                         revocationList, revocationListSize);

    /* Set stateCallback */
    clientConfig->timeout = 1000;
    clientConfig->stateCallback = stateCallback;
    clientConfig->subscriptionInactivityCallback = subscriptionInactivityCallback;

    UA_ByteString_clear(&certificate);
    UA_ByteString_clear(&privateKey);
    for(size_t deleteCount = 0; deleteCount < trustedListSize; deleteCount++) {
        UA_ByteString_clear(&trustList[deleteCount]);
    }

    /* Secure client connect */
    clientConfig->securityMode = UA_MESSAGESECURITYMODE_SIGNANDENCRYPT; /* require encryption */
    retval = UA_Client_connect(client, endpoint);
    if(retval != UA_STATUSCODE_GOOD) {
        UA_LOG_ERROR(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "Error: %s", UA_StatusCode_name(retval));
        cleanupClient();
        return UA_StatusCode_name(retval);
    }

    return "0";
}

/* clientContextCreate function establishes unsecure connection with the opcua server
 *
 * @param  hostname(string)           hostname of the system where opcua server is running
 * @param  port(uint)                 opcua port
 *
 * @return Return a string "0" for success and other string for failure of the function */
char*
clientContextCreate(char *hostname, int port) {
    // TODO: fill
    UA_StatusCode retval = UA_STATUSCODE_GOOD;

    char portStr[10];
    DBA_STRCPY(endpoint, "opc.tcp://");
    strcat_s(endpoint, ENDPOINT_SIZE, hostname);
    strcat_s(endpoint, ENDPOINT_SIZE, ":");
    strcat_s(endpoint, ENDPOINT_SIZE, convertToString(portStr, port));

    /* client initialization */
    client = UA_Client_new();
    if(client == NULL) {
        static char str[] = "UA_Client_new() API returned NULL";
        UA_LOG_FATAL(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,
            "%s", str);
        return str;
    }
    clientConfig = UA_Client_getConfig(client);
    UA_ClientConfig_setDefault(clientConfig);

    /* Set stateCallback */
    clientConfig->timeout = 1000;
    clientConfig->stateCallback = stateCallback;
    clientConfig->subscriptionInactivityCallback = subscriptionInactivityCallback;

    retval = UA_Client_connect(client, endpoint);
    if(retval != UA_STATUSCODE_GOOD) {
        cleanupClient();
        return UA_StatusCode_name(retval);
    }
    return "0";
}

/* Runs iteratively the client to auto-reconnect and re-subscribe to the last subscribed topic of the client */
static void*
runClient(void *ptr) {

    UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "In runClient...\n");
    while(!clientExited) {
        /* if already connected, this will return GOOD and do nothing */
        /* if the connection is closed/errored, the connection will be reset and then reconnected */
        /* Alternatively you can also use UA_Client_getState to get the current state */
        UA_ClientState clientState = UA_Client_getState(client);
        if (clientState == UA_CLIENTSTATE_DISCONNECTED) {
            UA_StatusCode retval = UA_Client_connect(client, endpoint);
            if(retval != UA_STATUSCODE_GOOD) {
                UA_LOG_ERROR(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "Error: %s", UA_StatusCode_name(retval));
                UA_LOG_ERROR(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "Not connected. Retrying to connect in 1 second");
                /* The connect may timeout after 1 second (see above) or it may fail immediately on network errors */
                /* E.g. name resolution errors or unreachable network. Thus there should be a small sleep here */
                UA_sleep_ms(1000);
                continue;
            }
        }

        UA_Client_run_iterate(client, 1000);
    }
    return NULL;

}

/* clientStartTopic function checks for the existence of the opcua variable(topic)
 *
 * @param  namespace(string)         opcua namespace name
 * @param  topic(string)             name of the opcua variable (aka topic name)
 *
 * @return Return namespace index for success and 100 for failure */
int
clientStartTopic(char *namespace, char *topic) {
    if (client == NULL) {
        static char str[] = "UA_Client instance is not created";
        UA_LOG_FATAL(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "%s",
            str);
        return FAILURE;
    }
    return getNamespaceIndex(namespace, topic);
}


/* clientSubscribe function makes the subscription to the opcua varaible named topic for a given namespace
 *
 * @param  namespace(string)         opcua namespace
 * @param  topic(string)             opcua variable name
 * @param  cb(c_callback)            callback that sends out the subscribed data back to the caller
 * @param  pyxFunc                   needed to callback pyx callback function to call the original python callback.
 *                                   For c and go callbacks, just puss NULL and nil respectively.
 *
 * @return Return a string "0" for success and other string for failure of the function */
char*
clientSubscribe(int nsIndex, char* topic, c_callback cb, void* pyxFunc) {
    userFunc = pyxFunc;
    if (client == NULL) {
        static char str[] = "UA_Client instance is not created";
        UA_LOG_FATAL(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "%s",
            str);
        return str;
    }
    UA_ClientState clientState = UA_Client_getState(client);
    if ((clientState != UA_CLIENTSTATE_SESSION) && (clientState != UA_CLIENTSTATE_SESSION_RENEWED)) {
        static char str[] = "Not a valid client state: for subscription to occurinstance is not created";
        UA_LOG_FATAL(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "%s",
            str);
        return str;
    }

    if (createSubscription(nsIndex, topic, cb) == 1) {
        static char str[] = "Subscription failed!";
        UA_LOG_FATAL(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "%s",
            str);
        return str;
    }

    pthread_t clientThread;
    if (pthread_create(&clientThread, NULL, runClient, NULL)) {
        static char str[] = "pthread creation to run the client thread iteratively failed";
        UA_LOG_FATAL(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "%s",
            str);
        return str;
    }

    return "0";
}

/* clientContextDestroy function destroys the opcua client context */
void clientContextDestroy() {
    clientExited = true;
    cleanupClient();
}
