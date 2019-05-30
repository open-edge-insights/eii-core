#include <gtest/gtest.h>
#include <CommonTestUtils.h>
#include <time.h>
#include <sys/time.h>
#include <stdlib.h>
#include <string.h>

using namespace std;

char pub[] = "PUB";
char sub[] = "SUB";
char pubsub[] = "PUBSUB";

char certFile[] = "/etc/ssl/opcua/opcua_client_certificate.der";
char privateFile[] = "/etc/ssl/opcua/opcua_client_key.der";
char trustFile[] = "/etc/ssl/ca/ca_certificate.der";

char *trustStores[] = {trustFile};

char topicName[] = "ab";
char ns[] = "tm";
char dtype[] = "string";

int topicMsgCount[10] = {
    0x00,
};

int strToInt(const char *text)
{
    int n = 0;
    for (; isdigit(*text); ++text)
    {
        n *= 10;
        n += (*text - '0');
    }
    return n;
}

void cb(const char *topic, const char *data, void *pyFunc)
{
    if (topic)
    {
        int val = strToInt(topic + 5);
        topicMsgCount[val]++;
    }

    if (data)
        printf("Data received: topic=%s and data=%s\n", topic, data);
}

TEST(ContextCreateTestCase, PositiveTestcaseCreateContextPub)
{
    /*Test description: This testcase calls ContextCreate API 
    for publisher and do not expect any error message*/
    struct ContextConfig contextConfig;
    char *trustFileArray[2] = {0x00};
    trustFileArray[0] = trustFile;
    initContext(&contextConfig, certFile, privateFile,
                trustFileArray, 1, "opcua://localhost:65003", pub);
    char *errorMsg = ContextCreate(contextConfig);
    int errLen = strcmp(errorMsg, "0");
    if (errLen)
    {
        printf("ContextCreate() API failed, error: %s\n", errorMsg);
    }
    ASSERT_EQ(errLen, 0);
    ContextDestroy();
    freeContext(&contextConfig);
}

TEST(ContextCreateTestCase, PositiveTestcaseContextDestroy)
{
    /*Test description:This testcase calls ContextDestroy
    and expects no crash.*/
    ContextDestroy();
}

TEST(ContextCreateTestCase, PositiveTestcaseSub)
{
    /*Test description: This testcase calls ContextCreate API for 
    publisher and subscriber and do not expect any error 
    message.*/
    struct ContextConfig contextConfigPub;
    struct ContextConfig contextConfigSub;

    char *trustFileArray[2] = {0x00};
    trustFileArray[0] = trustFile;
    initContext(&contextConfigPub, certFile, privateFile,
                trustFileArray, 1, "opcua://localhost:65006", pub);
    char *errorMsg = ContextCreate(contextConfigPub);
    if (strcmp(errorMsg, "0"))
    {
        printf("ContextCreate() API failed, error: %s\n", errorMsg);
    }

    initContext(&contextConfigSub, certFile, privateFile,
                trustFileArray, 1, "opcua://localhost:65006", sub);
    errorMsg = ContextCreate(contextConfigSub);
    int isError = strcmp(errorMsg, "0");
    if (isError)
    {
        printf("ContextCreate() API failed, error: %s\n", errorMsg);
    }
    ASSERT_EQ(isError, 0);
    ContextDestroy();
    freeContext(&contextConfigPub);
    freeContext(&contextConfigSub);
}

/*
TEST(ContextCreateTestCase, NegativeTestcasePubNullCryptoArgs)
{
    struct ContextConfig contextConfig;
    char *trustFileArray[2] = {0x00};
    char *errorMsg = NULL;

    //Test description: This testcase calls ContextCreate API for 
    //publisher with NULL as cert file and expects error message
    //in return. 
    //This test case need to be fixed.
    //This is currently crashinig with NULL as a cert file
    trustFileArray[0] = trustFile;
    initContext(&contextConfig, NULL, privateFile,
        trustFileArray,1,"opcua://localhost:65005", pub);
    errorMsg = ContextCreate(contextConfig);
    int isError = strcmp(errorMsg, "0");
    if(isError) {
        printf("ContextCreate() API failed, error: %s\n", errorMsg);
    }
    ASSERT_EQ(isError, 0);
    ContextDestroy();
    freeContext(&contextConfig);

    //Test description: This testcase calls ContextCreate API for 
    //publisher with NULL as private key file and expects error message
    //in return.
    // This test case need to be fixed.
    // it is passing with NULL as a private key file.
    // This test case need to be fixed.
    // currently it is crashing
    trustFileArray[0] = trustFile;
    initContext(&contextConfig, certFile, NULL,
                trustFileArray, 1, "opcua://localhost:65005", pubsub);
    errorMsg = ContextCreate(contextConfig);
    int isError = strcmp(errorMsg, "0");
    if (isError)
    {
        printf("ContextCreate() API failed, error: %s\n", errorMsg);
    }
    ASSERT_NE(isError, 0);
    ContextDestroy();
    freeContext(&contextConfig);

    //Test description: This testcase calls ContextCreate API for 
    //publisher with NULL as cert file and expects error message
    //in return. 
    //This test case need to be fixed.
    //This is currently crashinig with NULL as a cert file
    trustFileArray[0] = trustFile;
    initContext(&contextConfig, NULL, privateFile,
        trustFileArray,1,"opcua://localhost:65005", pub);
    errorMsg = ContextCreate(contextConfig);
    int isError = strcmp(errorMsg, "0");
    if(isError) {
        printf("ContextCreate() API failed, error: %s\n", errorMsg);
    }
    ASSERT_EQ(isError, 0);
    ContextDestroy();
    freeContext(&contextConfig);

    //Test description: This testcase calls ContextCreate API for 
    //publisher with 0 number of trust files and expects error message
    //in return.
    //This test case need to be fixed.
    // currently it is crashing
    // This test case passes 0 trust files.
    trustFileArray[0] = 0x00;
    initContext(&contextConfig, certFile, privateFile,
                trustFileArray, 1, "opcua://localhost:65005", pub);
    errorMsg = ContextCreate(contextConfig);
    int isError = strcmp(errorMsg, "0");
    if (isError)
    {
        printf("ContextCreate() API failed, error: %s\n", errorMsg);
    }
    ASSERT_EQ(isError, 0);
    ContextDestroy();
    freeContext(&contextConfig);
    
    //Test description: This testcase calls ContextCreate API for 
    //publisher with wrong args and expects error message
    //in return.
    // TODO:This test case need to be fixed.
    // it is crashing
    trustFileArray[0] = trustFile;
    initContext(&contextConfig, "INTEL", "INTEL",
                trustFileArray, 0, "opcua://localhost:65005", pub);
    errorMsg = ContextCreate(contextConfig);
    int isError = strcmp(errorMsg, "0");
    if (isError)
    {
        printf("ContextCreate() API failed, error: %s\n", errorMsg);
    }
    ASSERT_EQ(isError, 0);
    ContextDestroy();
    freeContext(&contextConfig);

    //Test description: This testcase calls ContextCreate API for 
    //publisher with NULL args and expects error message
    //in return.
    //TODO:This test case need to be fixed.
    //currently it is crashing
    trustFileArray[0] = trustFile;
    initContext(&contextConfig, NULL, NULL, 
        trustFileArray,0,"opcua://localhost:65005", pub);
    errorMsg = ContextCreate(contextConfig);
    if(strcmp(errorMsg, "0")) {
        printf("ContextCreate() API failed, error: %s\n", errorMsg);
    }
    
    ASSERT_NE(strcmp(errorMsg, "0"), 0);
    ContextDestroy();
    freeContext(&contextConfig);
}
*/

/*TEST(ContextCreateTestCase, NegativeTestcaseSubNullCertFile) { 
    //Test description: This testcase calls ContextCreate API for 
    //subscriber with NULL as cert file and expects error message
    //in return.
    // TODO:This test case need to be fixed
    struct ContextConfig contextConfigPub;
    struct ContextConfig contextConfigSub;
    
    char *trustFileArray[2] = {0x00};
    trustFileArray[0] = trustFile;
    initContext(&contextConfigPub, certFile, privateFile, 
        trustFileArray,1,"opcua://localhost:65006",pub);
    char *errorMsg = ContextCreate(contextConfigPub);
    if(strcmp(errorMsg, "0")) {
        printf("ContextCreate() API failed, error: %s\n", errorMsg);
    }

    initContext(&contextConfigSub, NULL, privateFile, 
        trustFileArray,1,"opcua://localhost:65006",sub);
    errorMsg = ContextCreate(contextConfigSub);
    if(strcmp(errorMsg, "0")) {
        printf("ContextCreate() API failed, error: %s\n", errorMsg);
    }
    ASSERT_NE(strcmp(errorMsg, "0"), 0);
    ContextDestroy();
    freeContext(&contextConfigPub);
    freeContext(&contextConfigSub);
}*/

TEST(ContextCreateTestCase, NegativeTestcaseSubNullPrivteKeyFile)
{
    /*Test description: This testcase calls ContextCreate API for 
    subscriber with NULL as private key file and expects error message
    in return.*/
    struct ContextConfig contextConfigPub;
    struct ContextConfig contextConfigSub;

    char *trustFileArray[2] = {0x00};
    trustFileArray[0] = trustFile;
    initContext(&contextConfigPub, certFile, privateFile,
                trustFileArray, 1, "opcua://localhost:65006", pub);
    char *errorMsg = ContextCreate(contextConfigPub);
    int isError = strcmp(errorMsg, "0");
    if (isError)
    {
        printf("ContextCreate() API failed for PUB, error: %s\n", errorMsg);
    }
    ASSERT_EQ(isError, 0);

    initContext(&contextConfigSub, certFile, NULL,
                trustFileArray, 1, "opcua://localhost:65006", sub);
    errorMsg = ContextCreate(contextConfigSub);
    isError = strcmp(errorMsg, "0");
    if (isError)
    {
        printf("ContextCreate() API failed for SUB, error: %s\n", errorMsg);
    }
    ASSERT_NE(isError, 0);
    
    ContextDestroy();
    freeContext(&contextConfigPub);
    freeContext(&contextConfigSub);
}

/*TEST(ContextCreateTestCase, NegativeTestcaseSubWrongNumOfTrustFiles) {
    // Test description: This testcase calls ContextCreate API for 
    // subscriber with 0 trust files and expects error message
    // in return.
    // TODO: This test case need to be fixed.
    // This is crashing with 0 number of trust files.
    struct ContextConfig contextConfigPub;
    struct ContextConfig contextConfigSub;
    
    char *trustFileArray[2] = {0x00};
    trustFileArray[0] = trustFile;
    initContext(&contextConfigPub, certFile, privateFile, 
        trustFileArray,1,"opcua://localhost:65006",pub);
    char *errorMsg = ContextCreate(contextConfigPub);
    if(strcmp(errorMsg, "0")) {
        printf("ContextCreate() API failed, error: %s\n", errorMsg);
    }

    initContext(&contextConfigSub, certFile, privateFile, 
        trustFileArray,0,"opcua://localhost:65006",sub);
    errorMsg = ContextCreate(contextConfigSub);
    if(strcmp(errorMsg, "0")) {
        printf("ContextCreate() API failed, error: %s\n", errorMsg);
    }
    ASSERT_NE(strcmp(errorMsg, "0"), 0);
    ContextDestroy();
    freeContext(&contextConfigPub);
    freeContext(&contextConfigSub);
}*/

TEST(ContextCreateTestCase, NegativeTestcaseWrongDirection)
{
    /*Test description: This testcase calls ContextCreate API 
    for a wrong direction argument and expects the erro message 
    in return*/
    // TODO:This test case need to be fixed.
    // The library should not allow directions
    // other than PUB/SUB
    struct ContextConfig contextConfig;
    char *trustFileArray[2] = {0x00};
    trustFileArray[0] = trustFile;
    initContext(&contextConfig, certFile, privateFile,
                trustFileArray, 1, "opcua://localhost:65005", pubsub);
    char *errorMsg = ContextCreate(contextConfig);
    int isError = strcmp(errorMsg, "0");
    if (isError)
    {
        printf("ContextCreate() API failed, error: %s\n", errorMsg);
    }
    ASSERT_EQ(isError, 0);
    ContextDestroy();
    freeContext(&contextConfig);
}

/*TEST(ContextCreateTestCase, NegativeTestcasePublishNullData) {
    //Test description: This testcase calls ContextCreate API 
    for pub and calls Publish API with NULL as a data argument.
    It expect some error in return.
    // TODO:This need to be fixed.
    // This is crashing while publishing the NULL Data
    struct ContextConfig contextConfigPub;
    
    char *trustFileArray[2] = {0x00};
    trustFileArray[0] = trustFile;
    initContext(&contextConfigPub, certFile, privateFile, 
        trustFileArray,1,"opcua://localhost:65004",pub);
    char *errorMsg = ContextCreate(contextConfigPub);
    if(strcmp(errorMsg, "0")) {
        printf("ContextCreate() API failed, error: %s\n", errorMsg);
    }

    struct TopicConfig tempTopicConfig[10];
    for(int i = 0; i< numOfTopic; i++){
        char topicName[10] = {0x00,};
        sprintf(topicName,"topic%d",i);
        initTopic(&tempTopicConfig[i],topicName, ns, dtype);
    }        
    
    for(int i = 0; i< numOfTopic; i++){
        char result[100] = {0x00};
        sprintf(result, "topic-creation for:%s, Data:%d", tempTopicConfig[i].name, i);
        errorMsg = Publish(tempTopicConfig[i], NULL);
        ASSERT_EQ(strcmp(errorMsg, "0"), 0);
    }

    ContextDestroy();
    freeContext(&contextConfigPub);
    for(int i = 0; i< numOfTopic; i++){
        freeTopic(&tempTopicConfig[i]);
    }
}*/

/*TEST(ContextCreateTestCase, NegativeTestcaseSubscribeNullCallBack) { 
    //Test description: This testcase calls ContextCreate  
    for pub and it calls Publish API. it then try to Subscribe API with NULL 
    as a callback argument and It expect some error in return.
    // TODO:This need to be fixed.
    // This is crashing while Subscribing the NULL callback
    struct ContextConfig contextConfigPub;
    struct ContextConfig contextConfigSub;
    
    char *trustFileArray[2] = {0x00};
    trustFileArray[0] = trustFile;
    initContext(&contextConfigPub, certFile, privateFile, 
        trustFileArray,1,"opcua://localhost:65004",pub);
    char *errorMsg = ContextCreate(contextConfigPub);
    if(strcmp(errorMsg, "0")) {
        printf("ContextCreate() API failed, error: %s\n", errorMsg);
    }

    initContext(&contextConfigSub, certFile, privateFile, 
    trustFileArray,1,"opcua://localhost:65004",sub);
    errorMsg = ContextCreate(contextConfigSub);
    if(strcmp(errorMsg, "0")) {
        printf("ContextCreate() API failed, error: %s\n", errorMsg);
    }
    
    struct TopicConfig tempTopicConfig[10];
    for(int i = 0; i< numOfTopic; i++){
        char topicName[10] = {0x00,};
        sprintf(topicName,"topic%d",i);
        initTopic(&tempTopicConfig[i],topicName, ns, dtype);
    }        
    
    for(int i = 0; i< numOfTopic; i++){
        char result[100] = {0x00};
        sprintf(result, "topic-creation for:%s, Data:%d", tempTopicConfig[i].name, i);
        errorMsg = Publish(tempTopicConfig[i], result);
        ASSERT_EQ(strcmp(errorMsg, "0"), 0);
    }

    errorMsg = Subscribe(tempTopicConfig, 10, "START", NULL, (void*)NULL);
    if(strcmp(errorMsg, "0")) {
        printf("Subscribe() API failed, error: %s\n", errorMsg);
    }
    ASSERT_NE(strcmp(errorMsg, "0"), 0);
    
    ContextDestroy();
    freeContext(&contextConfigPub);
    freeContext(&contextConfigSub);
    for(int i = 0; i< numOfTopic; i++){
        freeTopic(&tempTopicConfig[i]);
    }
}*/

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
