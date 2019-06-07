/*
Copyright (c) 2018 Intel Corporation.

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#define _DEFAULT_SOURCE 1

#include <unistd.h>
#include "open62541_wrappers.h"

// opcua server global variables
// Structure for maintaining Server Context
typedef struct {
    UA_Server *server;
    UA_ServerConfig *serverConfig;
    UA_ByteString* remoteCertificate;
    UA_Boolean serverRunning;
    char *topicContext;
    pthread_mutex_t *serverLock;
} server_context_t;

// opcua client global variables
typedef struct {
    int namespaceIndex;
    char *topic;
    void *userFunc;
    c_callback userCallback;
} monitor_context_t;

typedef struct {
    struct TopicConfig *topicCfgArr;
    int topicCfgItems;
    void *userFunc;
    c_callback userCallback;
    monitor_context_t *monitorContext;
} subscribe_args_t;

// Structure for maintaining Client Context
typedef struct {
    UA_Client *client;
    UA_ClientConfig* clientConfig;
    char endpoint[ENDPOINT_SIZE];
    char dataToPublish[PUBLISH_DATA_SIZE];
    bool clientExited;
    subscribe_args_t *subArgs;
    UA_MonitoredItemCreateRequest *items;
    UA_Client_DataChangeNotificationCallback *subCallbacks;
    UA_Client_DeleteMonitoredItemCallback *deleteCallbacks;
    void **contexts;
} client_context_t;

static server_context_t gServerContext;
static client_context_t gClientContext;

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

/* freeMemory frees up heap allocated memory */
static void
freeMemory(void *ptr) {
    if (ptr != NULL)
        free(ptr);
}

/* Converts the passed int to string */
static char*
convertToString(char str[],
                             int num) {
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

/* Gets the namespace index for the given namespace and topic */
static UA_Int16
getNamespaceIndex(char *ns,
                  char *topic) {
    UA_LOG_DEBUG(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "Browsing nodes in objects folder:\n");
    char nodeIdentifier[NAMESPACE_SIZE];
    UA_BrowseRequest bReq;
    UA_BrowseRequest_init(&bReq);
    bReq.requestedMaxReferencesPerNode = 0;
    bReq.nodesToBrowse = UA_BrowseDescription_new();
    bReq.nodesToBrowseSize = 1;
    bReq.nodesToBrowse[0].nodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER); /* browse objects folder */
    bReq.nodesToBrowse[0].resultMask = UA_BROWSERESULTMASK_ALL; /* return everything */
    UA_BrowseResponse bResp = UA_Client_Service_browse(gClientContext.client, bReq);
    UA_LOG_DEBUG(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "%-9s %-16s %-16s %-16s\n", "NAMESPACE", "NODEID", "BROWSE NAME", "DISPLAY NAME");
    for(size_t i = 0; i < bResp.resultsSize; ++i) {
        for(size_t j = 0; j < bResp.results[i].referencesSize; ++j) {
            UA_ReferenceDescription *ref = &(bResp.results[i].references[j]);
            if(ref->nodeId.nodeId.identifierType == UA_NODEIDTYPE_STRING) {
                int len = (int)ref->nodeId.nodeId.identifier.string.length;
                UA_LOG_DEBUG(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "%-9d %-16.*s %-16.*s %-16.*s\n", ref->nodeId.nodeId.namespaceIndex,
                       (int)ref->nodeId.nodeId.identifier.string.length,
                       ref->nodeId.nodeId.identifier.string.data,
                       (int)ref->browseName.name.length, ref->browseName.name.data,
                       (int)ref->displayName.text.length, ref->displayName.text.data);
                DBA_STRNCPY(nodeIdentifier, (char*)ref->nodeId.nodeId.identifier.string.data, len);
                UA_Int16 nodeNs = ref->nodeId.nodeId.namespaceIndex;
                if (!strcmp(topic, nodeIdentifier)) {
                    UA_LOG_DEBUG(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "Node Id exists !!!\n");
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
                     "In %s function...", __FUNCTION__);
    data->hasValue = true;
    UA_String str = UA_STRING(gClientContext.dataToPublish);
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

static void
addTopicDataSourceVariable(char *namespace,
                           char *topic,
                           size_t* namespaceIndex) {

    UA_StatusCode ret = UA_Server_getNamespaceByName(gServerContext.server, UA_STRING(namespace), namespaceIndex);
    if (ret == UA_STATUSCODE_GOOD) {
        UA_LOG_DEBUG(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "Namespace: %s exist.",
                     namespace);
    } else {
        *namespaceIndex = UA_Server_addNamespace(gServerContext.server, namespace);
        if (*namespaceIndex == 0) {
            static char str[] = "UA_Server_addNamespace() has failed";
            UA_LOG_FATAL(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "%s for namespace: %s", str, namespace);
            return;
        }
    }
    UA_LOG_DEBUG(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "%lu:namespaceIndex created for namespace: %s\n", *namespaceIndex, namespace);

    UA_VariableAttributes attr = UA_VariableAttributes_default;
    attr.description = UA_LOCALIZEDTEXT("en-US", topic);
    attr.displayName = UA_LOCALIZEDTEXT("en-US", topic);
    attr.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;

    /* Add the variable node to the information model */
    UA_NodeId currentNodeId = UA_NODEID_STRING(*namespaceIndex, topic);
    ret = UA_Server_getNodeContext(gServerContext.server, currentNodeId, (void**) &gServerContext.topicContext);

    if (gServerContext.topicContext != NULL) {
        UA_LOG_DEBUG(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "Topic context: %s", gServerContext.topicContext);
        if (!strcmp(gServerContext.topicContext, topic)) {
            static char str[] = "Topic already exist for namespace";
            UA_LOG_DEBUG(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "%s", str);
            return;
        }
    }

    UA_QualifiedName currentName = UA_QUALIFIEDNAME(*namespaceIndex, topic);
    UA_NodeId parentNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER);
    UA_NodeId parentReferenceNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES);
    UA_NodeId variableTypeNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE);

    UA_DataSource topicDataSource;
    topicDataSource.read = readPublishedData;
    topicDataSource.write = writePublishedData;
    ret = UA_Server_addDataSourceVariableNode(gServerContext.server, currentNodeId, parentNodeId,
                                              parentReferenceNodeId, currentName,
                                              variableTypeNodeId, attr,
                                              topicDataSource, topic, NULL);
    if (ret != UA_STATUSCODE_GOOD) {
        static char str[] = "UA_Server_addDataSourceVariableNode() has failed";
        UA_LOG_ERROR(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "%s \
                    for namespace: %s and topic: %s. Error code: %s", str, namespace, topic, UA_StatusCode_name(ret));
        return;
    }
}

/* cleanupServer deletes the memory allocated for server configuration */
static void
cleanupServer() {
    if (gServerContext.server) {
        UA_Server_run_shutdown(gServerContext.server);
        UA_Server_delete(gServerContext.server);
    }
    if (gServerContext.serverConfig) {
        UA_ServerConfig_delete(gServerContext.serverConfig);
    }
    if (gServerContext.serverLock) {
        int rc = pthread_mutex_destroy(gServerContext.serverLock);
        assert(rc == 0);
        free(gServerContext.serverLock);
    }
    free(gServerContext.topicContext);
}

static void*
startServer(void *ptr) {

    /* run server */
    UA_StatusCode retval = UA_Server_run_startup(gServerContext.server);
    if (retval != UA_STATUSCODE_GOOD) {
        UA_LOG_FATAL(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "\nServer failed to start, error: %s", UA_StatusCode_name(retval));
        return NULL;
    }

    UA_UInt16 timeout;
    gServerContext.serverRunning = true;
    while (gServerContext.serverRunning) {
        int rc = pthread_mutex_lock(gServerContext.serverLock);
        assert(rc == 0);
        /* timeout is the maximum possible delay (in millisec) until the next
        _iterate call. Otherwise, the server might miss an internal timeout
        or cannot react to messages with the promised responsiveness. */
        timeout = UA_Server_run_iterate(gServerContext.server, false);
        rc = pthread_mutex_unlock(gServerContext.serverLock);
        assert(rc == 0);

        /* Now we can use the max timeout to do something else. In this case, we
        just sleep. (select is used as a platform-independent sleep
        function.) */
        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = timeout * 1000;
        select(0, NULL, NULL, NULL, &tv);
    }
    return NULL;
}

char*
serverContextCreateSecured(const char *hostname,
                           unsigned int port,
                           const char *certificateFile,
                           const char *privateKeyFile,
                           const char **trustedCerts,
                           size_t trustedListSize) {

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
    gServerContext.serverConfig =
        UA_ServerConfig_new_basic256sha256(port, &certificate, &privateKey,
                                          trustList, trustedListSize,
                                          revocationList, revocationListSize);
    if(!gServerContext.serverConfig) {
        static char str[] = "Could not create the server config";
        UA_LOG_FATAL(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "%s",
            str);
        return str;
    }

    for(int i = 0; i < gServerContext.serverConfig->endpointsSize; i++) {
        if(gServerContext.serverConfig->endpoints[i].securityMode != UA_MESSAGESECURITYMODE_SIGNANDENCRYPT) {
            gServerContext.serverConfig->endpoints[i].userIdentityTokens = NULL;
            gServerContext.serverConfig->endpoints[i].userIdentityTokensSize = 0;
        }
    }

    UA_ServerConfig_set_customHostname(gServerContext.serverConfig, UA_STRING(hostname));

    UA_DurationRange range = {5.0, 5.0};
    gServerContext.serverConfig->publishingIntervalLimits = range;
    gServerContext.serverConfig->samplingIntervalLimits = range;

    /* Initiate server instance */
    gServerContext.server = UA_Server_new(gServerContext.serverConfig);
    if(gServerContext.server == NULL) {
        static char str[] = "UA_Server_new() API failed";
        UA_LOG_FATAL(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "%s",
            str);
        return str;
    }

    /* Creation of mutex for server instance */
    gServerContext.serverLock = (pthread_mutex_t*) malloc(sizeof(pthread_mutex_t));

    if (!gServerContext.serverLock || pthread_mutex_init(gServerContext.serverLock, NULL) != 0) {
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
    gServerContext.topicContext = NULL;
    return "0";
}

char*
serverContextCreate(const char *hostname,
                    unsigned int port) {

    /* Initiate server config */
    gServerContext.serverConfig = UA_ServerConfig_new_minimal(port, NULL);;
    if(!gServerContext.serverConfig) {
        static char str[] = "Could not create the server config";
        UA_LOG_FATAL(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "%s",
            str);
        return str;
    }
    UA_ServerConfig_set_customHostname(gServerContext.serverConfig, UA_STRING(hostname));

    UA_DurationRange range = {5.0, 10.0};
    gServerContext.serverConfig->publishingIntervalLimits = range;
    gServerContext.serverConfig->samplingIntervalLimits = range;

    /* Initiate server instance */
    gServerContext.server = UA_Server_new(gServerContext.serverConfig);

    if(gServerContext.server == NULL) {
        static char str[] = "UA_Server_new() API failed";
        UA_LOG_FATAL(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "%s",
            str);
        return str;
    }

    /* Creation of mutex for server instance */
    gServerContext.serverLock = (pthread_mutex_t*) malloc(sizeof(pthread_mutex_t));

    if (!gServerContext.serverLock || pthread_mutex_init(gServerContext.serverLock, NULL) != 0) {
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
    gServerContext.topicContext = NULL;
    return "0";
}

char*
serverPublish(struct TopicConfig topicConfig,
              const char* data) {

    /* check if server is started or not */
    if (gServerContext.server == NULL) {
        static char str[] = "UA_Server instance is not instantiated";
        UA_LOG_FATAL(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "%s",
            str);
        return str;
    }

    size_t nsIndex;
    if(gServerContext.topicContext == NULL) {
        gServerContext.topicContext = (char*) malloc(strlen(topicConfig.name) + 1);
    }

    addTopicDataSourceVariable(topicConfig.ns,
                               topicConfig.name,
                               &nsIndex);

    /* writing the data to the opcua variable */
    UA_Variant *val = UA_Variant_new();
    DBA_STRCPY(gClientContext.dataToPublish, data);
    UA_String str = UA_STRING(data);
    UA_Variant_setScalarCopy(val, &str, &UA_TYPES[UA_TYPES_STRING]);
    UA_LOG_DEBUG(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "nsIndex: %lu, topic:%s\n", nsIndex, topicConfig.name);

    /*sleep for mininum publishing interval in ms*/
    UA_sleep_ms((int)gServerContext.serverConfig->publishingIntervalLimits.min);

    int rc = pthread_mutex_lock(gServerContext.serverLock);
    assert(rc == 0);
    UA_StatusCode retval = UA_Server_writeValue(gServerContext.server, UA_NODEID_STRING(nsIndex, topicConfig.name), *val);
    rc = pthread_mutex_unlock(gServerContext.serverLock);
    assert(rc == 0);

    if (retval == UA_STATUSCODE_GOOD) {
        UA_Variant_delete(val);
        return "0";
    }
    UA_LOG_FATAL(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "%s", UA_StatusCode_name(retval));
    UA_Variant_delete(val);
    return (char *)UA_StatusCode_name(retval);
}

void serverContextDestroy() {
    cleanupServer();
}

//*************open62541 client wrappers**********************

/* cleanupClient deletes the memory allocated for client configuration */
static void
cleanupClient() {
    if (gServerContext.remoteCertificate) {
        UA_ByteString_delete(gServerContext.remoteCertificate);
    }
    if (gClientContext.client) {
        UA_Client_delete(gClientContext.client);
    }
    
    freeMemory(gClientContext.subArgs->topicCfgArr);
    freeMemory(gClientContext.subArgs->monitorContext);
    freeMemory(gClientContext.subArgs);
    freeMemory(gClientContext.items);
    freeMemory(gClientContext.subCallbacks);
    freeMemory(gClientContext.deleteCallbacks);
    freeMemory(gClientContext.contexts);
}

static void
deleteSubscriptionCallback(UA_Client *client,
                           UA_UInt32 subscriptionId,
                           void *subscriptionContext) {
    UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "Subscription Id %u was deleted", subscriptionId);
}

static void
subscriptionInactivityCallback(UA_Client *client,
                               UA_UInt32 subId,
                               void *subContext) {
    UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "Inactivity for subscription %u", subId);
}

static void
subscriptionCallback(UA_Client *client,
                     UA_UInt32 subId,
                     void *subContext,
                     UA_UInt32 monId,
                     void *monContext,
                     UA_DataValue *data) {
    UA_LOG_DEBUG(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "In %s...", __FUNCTION__);
    UA_Variant *value = &data->value;
    monitor_context_t *args = (monitor_context_t*) monContext;
    if(args != NULL && UA_Variant_isScalar(value)) {
        if (value->type == &UA_TYPES[UA_TYPES_STRING]) {
            UA_String str = *(UA_String*)value->data;
            if (args->userCallback) {
                static char subscribedData[PUBLISH_DATA_SIZE];
                DBA_STRCPY(subscribedData, (char*)str.data);
                if (strstr(subscribedData, args->topic) != NULL)
                    args->userCallback(args->topic, subscribedData, args->userFunc);
            } else {
                UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "userCallback is NULL");
            }
        }
    }
}

/* creates the subscription for the opcua variable with topic name */
static UA_Int16
createSubscription() {

    UA_CreateSubscriptionRequest request = UA_CreateSubscriptionRequest_default();
    request.requestedPublishingInterval = 0;

    UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "In %s...!", __FUNCTION__);
    UA_CreateSubscriptionResponse response = UA_Client_Subscriptions_create(gClientContext.client, request,
                                                                            NULL, NULL, deleteSubscriptionCallback);

    if(response.responseHeader.serviceResult == UA_STATUSCODE_GOOD) {
        UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "Create subscription succeeded, id %u", response.subscriptionId);
    } else {
        UA_LOG_FATAL(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "Create subscription failed");
        return 1;
    }
    int subId = response.subscriptionId;

    if(gClientContext.items == NULL) {
        gClientContext.items = (UA_MonitoredItemCreateRequest*) malloc(gClientContext.subArgs->topicCfgItems * sizeof(UA_MonitoredItemCreateRequest));
    }
    if(gClientContext.subCallbacks == NULL) {
        gClientContext.subCallbacks = (UA_Client_DataChangeNotificationCallback*) malloc(gClientContext.subArgs->topicCfgItems * sizeof(UA_Client_DataChangeNotificationCallback));
    }
    if(gClientContext.deleteCallbacks == NULL) {
        gClientContext.deleteCallbacks = (UA_Client_DeleteMonitoredItemCallback*) malloc(gClientContext.subArgs->topicCfgItems * sizeof(UA_Client_DeleteMonitoredItemCallback));
    }
    if(gClientContext.contexts == NULL) {
        gClientContext.contexts = (void*) malloc(gClientContext.subArgs->topicCfgItems * sizeof(void*));;
    }

    char *topic;
    char *ns;
    size_t namespaceIndex;
    UA_NodeId nodeId;
    for(int i = 0; i < gClientContext.subArgs->topicCfgItems; i++) {
        topic = gClientContext.subArgs->topicCfgArr[i].name;
        ns = gClientContext.subArgs->topicCfgArr[i].ns;
        namespaceIndex = getNamespaceIndex(ns, topic);
        nodeId = UA_NODEID_STRING(namespaceIndex, topic);

        if(gClientContext.items != NULL) {
            gClientContext.items[i] = UA_MonitoredItemCreateRequest_default(nodeId);
        }
        if(gClientContext.subCallbacks != NULL) {
            gClientContext.subCallbacks[i] = subscriptionCallback;
        }

        gClientContext.subArgs->monitorContext[i].namespaceIndex = namespaceIndex;
        gClientContext.subArgs->monitorContext[i].topic = topic;
        gClientContext.subArgs->monitorContext[i].userCallback = gClientContext.subArgs->userCallback;
        gClientContext.subArgs->monitorContext[i].userFunc = gClientContext.subArgs->userFunc;
        if(gClientContext.contexts != NULL) {
            gClientContext.contexts[i] = &gClientContext.subArgs->monitorContext[i];
        }
        if(gClientContext.deleteCallbacks != NULL) {
            gClientContext.deleteCallbacks[i] = NULL;
        }
    }

    UA_CreateMonitoredItemsRequest createRequest;
    UA_CreateMonitoredItemsRequest_init(&createRequest);
    createRequest.subscriptionId = subId;
    createRequest.timestampsToReturn = UA_TIMESTAMPSTORETURN_BOTH;
    createRequest.itemsToCreate = gClientContext.items;
    createRequest.itemsToCreateSize = gClientContext.subArgs->topicCfgItems;
    if (gClientContext.deleteCallbacks != NULL && gClientContext.contexts != NULL && gClientContext.subCallbacks != NULL) {
       UA_CreateMonitoredItemsResponse createResponse =
       UA_Client_MonitoredItems_createDataChanges(gClientContext.client, createRequest, gClientContext.contexts,
                                                   gClientContext.subCallbacks, gClientContext.deleteCallbacks);

        for(int i = 0; i < gClientContext.subArgs->topicCfgItems; i++) {
            if (createResponse.results[0].statusCode == UA_STATUSCODE_GOOD) {
                UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,"MonitorItemId: %d created successfully for topic: %s\n",
                            createResponse.results[0].monitoredItemId, gClientContext.subArgs->topicCfgArr[i].name);
            }
        }
    }

    return 0;
}

static void stateCallback(UA_Client *client,
                          UA_ClientState clientState) {
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


/* Runs iteratively the client to auto-reconnect and re-subscribe to the last subscribed topic of the client */
static void*
runClient(void *tArgs) {
    UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "In %s...Thread ID: %lu", __FUNCTION__, pthread_self());

    while(!gClientContext.clientExited) {
        /* if already connected, this will return GOOD and do nothing */
        /* if the connection is closed/errored, the connection will be reset and then reconnected */
        /* Alternatively you can also use UA_Client_getState to get the current state */
        UA_ClientState clientState = UA_Client_getState(gClientContext.client);
        if (clientState == UA_CLIENTSTATE_DISCONNECTED) {
            UA_StatusCode retval = UA_Client_connect(gClientContext.client, gClientContext.endpoint);
            if(retval != UA_STATUSCODE_GOOD) {
                UA_LOG_ERROR(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "Error: %s", UA_StatusCode_name(retval));
                UA_LOG_ERROR(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "Not connected. Retrying to connect in 1 second");
                /* The connect may timeout after 1 second (see above) or it may fail immediately on network errors */
                /* E.g. name resolution errors or unreachable network. Thus there should be a small sleep here */
                UA_sleep_ms(1000);
                continue;
            }
            clientState = UA_Client_getState(gClientContext.client);
            if (clientState == UA_CLIENTSTATE_SESSION) {
                /* recreating the subscription upon opcua server connect */
                if (createSubscription() == 1)
                    UA_LOG_FATAL(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "Subscription failed!");
            }
        }

        UA_Client_run_iterate(gClientContext.client, 1000);
    }
    return NULL;

}

char*
clientContextCreateSecured(const char *hostname,
                           unsigned int port,
                           const char *certificateFile,
                           const char *privateKeyFile,
                           const char **trustedCerts,
                           size_t trustedListSize) {

    UA_StatusCode retval = UA_STATUSCODE_GOOD;
    UA_ByteString *revocationList = NULL;
    size_t revocationListSize = 0;

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

    gServerContext.remoteCertificate = UA_ByteString_new();

    char portStr[10];
    const char* opcuaProto = "opc.tcp://";
    strcpy_s(gClientContext.endpoint, strlen(opcuaProto), opcuaProto);
    strcat_s(gClientContext.endpoint, ENDPOINT_SIZE, hostname);
    strcat_s(gClientContext.endpoint, ENDPOINT_SIZE, ":");
    strcat_s(gClientContext.endpoint, ENDPOINT_SIZE, convertToString(portStr, port));

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

    gClientContext.client = UA_Client_new();
    if(gClientContext.client == NULL) {
        static char str[] = "UA_Client_new() API returned NULL";
        UA_LOG_FATAL(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,
            "%s", str);
        return str;
    }

    gClientContext.clientConfig = UA_Client_getConfig(gClientContext.client);
    gClientContext.clientConfig->securityMode = UA_MESSAGESECURITYMODE_SIGNANDENCRYPT;
    UA_ClientConfig_setDefaultEncryption(gClientContext.clientConfig, certificate, privateKey,
                                         trustList, trustedListSize,
                                         revocationList, revocationListSize);

    /* Set stateCallback */
    gClientContext.clientConfig->timeout = 1000;
    gClientContext.clientConfig->stateCallback = stateCallback;
    gClientContext.clientConfig->subscriptionInactivityCallback = subscriptionInactivityCallback;

    UA_ByteString_clear(&certificate);
    UA_ByteString_clear(&privateKey);
    for(size_t deleteCount = 0; deleteCount < trustedListSize; deleteCount++) {
        UA_ByteString_clear(&trustList[deleteCount]);
    }

    /* Secure client connect */
    gClientContext.clientConfig->securityMode = UA_MESSAGESECURITYMODE_SIGNANDENCRYPT; /* require encryption */
    retval = UA_Client_connect(gClientContext.client, gClientContext.endpoint);
    if(retval != UA_STATUSCODE_GOOD) {
        UA_LOG_ERROR(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "Error: %s", UA_StatusCode_name(retval));
        cleanupClient();
        return (char *)UA_StatusCode_name(retval);
    }

    return "0";
}

char*
clientContextCreate(const char *hostname,
                    unsigned int port) {
    UA_StatusCode retval = UA_STATUSCODE_GOOD;

    char portStr[10];
    const char* opcuaProto = "opc.tcp://";
    strcpy_s(gClientContext.endpoint, strlen(opcuaProto), opcuaProto);
    strcat_s(gClientContext.endpoint, ENDPOINT_SIZE, hostname);
    strcat_s(gClientContext.endpoint, ENDPOINT_SIZE, ":");
    strcat_s(gClientContext.endpoint, ENDPOINT_SIZE, convertToString(portStr, port));

    /* client initialization */
    gClientContext.client = UA_Client_new();
    if(gClientContext.client == NULL) {
        static char str[] = "UA_Client_new() API returned NULL";
        UA_LOG_FATAL(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,
            "%s", str);
        return str;
    }
    gClientContext.clientConfig = UA_Client_getConfig(gClientContext.client);
    UA_ClientConfig_setDefault(gClientContext.clientConfig);

    /* Set stateCallback */
    gClientContext.clientConfig->timeout = 1000;
    gClientContext.clientConfig->stateCallback = stateCallback;
    gClientContext.clientConfig->subscriptionInactivityCallback = subscriptionInactivityCallback;

    retval = UA_Client_connect(gClientContext.client, gClientContext.endpoint);
    if(retval != UA_STATUSCODE_GOOD) {
        cleanupClient();
        return (char *)UA_StatusCode_name(retval);
    }
    return "0";
}

char*
clientSubscribe(struct TopicConfig topicConfigs[],
                unsigned int topicConfigCount,
                c_callback cb,
                void* pyxFunc) {

    if (gClientContext.client == NULL) {
        static char str[] = "UA_Client instance is not created";
        UA_LOG_FATAL(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "%s",
            str);
        return str;
    }

    gClientContext.subArgs = (subscribe_args_t*) malloc(sizeof(subscribe_args_t));
    if(gClientContext.subArgs != NULL) {
        gClientContext.subArgs->userFunc = pyxFunc;
        gClientContext.subArgs->userCallback = cb;
        gClientContext.subArgs->topicCfgItems = topicConfigCount;
        gClientContext.subArgs->topicCfgArr = (struct TopicConfig*) malloc(topicConfigCount * sizeof(struct TopicConfig));
        gClientContext.subArgs->monitorContext = (monitor_context_t*) malloc(gClientContext.subArgs->topicCfgItems * sizeof(monitor_context_t));
        for(int i = 0; i < topicConfigCount; i++) {
            if (gClientContext.subArgs->topicCfgArr != NULL) {
                gClientContext.subArgs->topicCfgArr[i] = topicConfigs[i];
            }
        }
    }

    if (createSubscription() == 1) {
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

void clientContextDestroy() {
    gClientContext.clientExited = true;
    cleanupClient();
}
