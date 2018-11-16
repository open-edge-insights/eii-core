#include "open62541_wrappers.h"

void cb(char *data) {
    printf("Data received: %s\n", data);
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
    UA_Int16 port = 4840;
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
     
        
        errorMsg = serverContextCreate(hostname, port, ns, topic, certificatePath, privateKey, trustList, trustListSize);
        if(strcmp(errorMsg, "0")) {
            printf("serverContextCreate() API failed, error: %s", errorMsg);
            return -1;
        }
        printf("serverContextCreate() API successfully executed!");
        char result[100];
        for (int i = 0; i < 100; i++) {
            sleep(5);
            sprintf(result, "Hello %d", i);
            errorMsg = serverPublish(ns, topic, result);
            if(strcmp(errorMsg, "0")) {
                printf("serverPublish() API failed, error: %s", errorMsg);
                return -1;
            }
            printf("serverPublish() API successfully executed!");
        } 
        serverContextDestroy();
    } else if (!strcmp(argv[1], "client")) {
        errorMsg = clientContextCreate(hostname, port, certificatePath, privateKey, trustList, trustListSize, NULL, NULL);
        if(strcmp(errorMsg, "0")) {
            printf("clientContextCreate() API failed, error: %s", errorMsg);
            return -1;
        }
        printf("clientContextCreate() API successfully executed!");
        errorMsg = clientSubscribe(ns, topic, cb);
        if(strcmp(errorMsg, "0")) {
            printf("clientSubscribe() API failed, error: %s", errorMsg);
            return -1;
        }
        printf("clientSubscribe() API successfully executed!");
        sleep(120); /* sleep for 2 mins */
        clientContextDestroy();
    }
}
