#include <stdio.h>
#include "open62541.h"
#include "common.h"
#include <pthread.h>

//*************open62541 server wrappers**********************
char*
serverContextCreate(char *hostname,
                    UA_Int16 port,
                    char *ns, 
                    char *topic,
                    char *certificateFile, 
                    char *privateKeyFile,
                    char **trustList,
                    size_t trustListSize);

char*
serverPublish(char *ns, 
              char *topic,
              char *data);

void serverContextDestroy();

//*************open62541 client wrappers**********************

typedef void (*callback)(char *user_data);

char*
clientContextCreate(char *hostname,
                    UA_Int16 port,
                    char *certificateFile,
                    char *privateKeyFile,
                    char **trustList,
                    size_t trustListSize,
                    char *username,
                    char *password);

char*
clientSubscribe(char* ns, 
                char* topic,
                callback cb);
                            
void clientContextDestroy();