#include <gtest/gtest.h>
#include <CommonTestUtils.h>
#include <time.h>
#include <sys/time.h>
#include <stdlib.h>
#include <string.h>

using namespace std;

#define NUM_OF_TOPICS 10
#define MSG_SIZE 100
#define TOPIC_NAME 32

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

TEST(ContextCreateTestCase, PositiveTestcasePubSub)
{
    /*Test description: This testcase creates the PUB abd SUB.
    PUB creates the 10 topics and publishes some data.
    SUB is getting the subscribed data. And test case
    verifies the message count at subscriber end. 
    */
    struct ContextConfig contextConfigPub;
    struct ContextConfig contextConfigSub;

    char *trustFileArray[2] = {0x00};
    trustFileArray[0] = trustFile;
    char *errorMsg = NULL;
    int isError = 0;

    initContext(&contextConfigPub, certFile, privateFile,
                trustFileArray, 1, "opcua://localhost:65004", pub);
    errorMsg = ContextCreate(contextConfigPub);
    isError = strcmp(errorMsg, "0");
    if (isError)
    {
        printf("ContextCreate() API failed, error: %s\n", errorMsg);
    }
    ASSERT_EQ(isError, 0);

    initContext(&contextConfigSub, certFile, privateFile,
                trustFileArray, 1, "opcua://localhost:65004", sub);
    errorMsg = ContextCreate(contextConfigSub);
    isError = strcmp(errorMsg, "0");
    if (isError)
    {
        printf("ContextCreate() API failed, error: %s\n", errorMsg);
    }
    ASSERT_EQ(isError, 0);

    struct TopicConfig tempTopicConfig[NUM_OF_TOPICS];
    for (int i = 0; i < NUM_OF_TOPICS; i++)
    {
        char topicName[TOPIC_NAME] = {
            0x00,
        };
        sprintf(topicName, "topic%d", i);
        initTopic(&tempTopicConfig[i], topicName, ns, dtype);
    }

    for (int i = 0; i < NUM_OF_TOPICS; i++)
    {
        char result[MSG_SIZE] = {0x00};
        sprintf(result, "topic-creation for:%s, Data:%d", tempTopicConfig[i].name, i);
        errorMsg = Publish(tempTopicConfig[i], result);
        isError = strcmp(errorMsg, "0");
        if (isError)
        {
            printf("Publish() API failed, error: %s\n", errorMsg);
        }
        ASSERT_EQ(isError, 0);
    }

    errorMsg = Subscribe(tempTopicConfig, NUM_OF_TOPICS, "START", cb, (void *)NULL);
    isError = strcmp(errorMsg, "0");
    if (isError)
    {
        printf("Subscribe() API failed, error: %s\n", errorMsg);
    }
    ASSERT_EQ(isError, 0);

    for (int i = 0; i < NUM_OF_TOPICS; i++)
    {
        char result[MSG_SIZE] = {0x00};
        sprintf(result, "Data-publishing for:%s, Data:%d", tempTopicConfig[i].name, i);
        errorMsg = Publish(tempTopicConfig[i], result);
        isError = strcmp(errorMsg, "0");
        if (isError)
        {
            printf("Publish() API failed, error: %s\n", errorMsg);
        }
        ASSERT_EQ(isError, 0);
    }

    sleep(5);

    for (int i = 0; i < NUM_OF_TOPICS; i++)
    {
        printf("topic%d got %d messages\n", i, topicMsgCount[i]);
        ASSERT_GT(topicMsgCount[i], 0);
    }

    ContextDestroy();
    freeContext(&contextConfigPub);
    freeContext(&contextConfigSub);
    for (int i = 0; i < NUM_OF_TOPICS; i++)
    {
        freeTopic(&tempTopicConfig[i]);
    }
}

TEST(ContextCreateTestCase, PositiveTestcasePubSubDevMode)
{
    /*Test description: This is test case for Developer mode.
    This testcase creates the PUB abd SUB.
    PUB creates the 10 topics and publishes some data.
    SUB is getting the subscribed data. And test case
    verifies the message count at subscriber end. 
    */
    struct ContextConfig contextConfigPub;
    struct ContextConfig contextConfigSub;

    char *trustFileArray[2] = {0x00};
    trustFileArray[0] = "";
    char *errorMsg = NULL;
    int isError = 0;

    initContext(&contextConfigPub, "", "",
                trustFileArray, 1, "opcua://localhost:65011", pub);
    errorMsg = ContextCreate(contextConfigPub);
    isError = strcmp(errorMsg, "0");
    if (isError)
    {
        printf("ContextCreate() API failed, error: %s\n", errorMsg);
    }
    ASSERT_EQ(isError, 0);

    initContext(&contextConfigSub, "", "",
                trustFileArray, 1, "opcua://localhost:65011", sub);
    errorMsg = ContextCreate(contextConfigSub);
    isError = strcmp(errorMsg, "0");
    if (isError)
    {
        printf("ContextCreate() API failed, error: %s\n", errorMsg);
    }
    ASSERT_EQ(isError, 0);

    struct TopicConfig tempTopicConfig[NUM_OF_TOPICS];
    for (int i = 0; i < NUM_OF_TOPICS; i++)
    {
        char topicName[TOPIC_NAME] = {
            0x00,
        };
        sprintf(topicName, "topic%d", i);
        initTopic(&tempTopicConfig[i], topicName, ns, dtype);
    }

    for (int i = 0; i < NUM_OF_TOPICS; i++)
    {
        char result[MSG_SIZE] = {0x00};
        sprintf(result, "topic-creation for:%s, Data:%d", tempTopicConfig[i].name, i);
        errorMsg = Publish(tempTopicConfig[i], result);
        isError = strcmp(errorMsg, "0");
        if (isError)
        {
            printf("Publish() API failed, error: %s\n", errorMsg);
        }
        ASSERT_EQ(isError, 0);
    }

    errorMsg = Subscribe(tempTopicConfig, NUM_OF_TOPICS, "START", cb, (void *)NULL);
    isError = strcmp(errorMsg, "0");
    if (isError)
    {
        printf("Subscribe() API failed, error: %s\n", errorMsg);
    }
    ASSERT_EQ(isError, 0);

    for (int i = 0; i < NUM_OF_TOPICS; i++)
    {
        char result[MSG_SIZE] = {0x00};
        sprintf(result, "Data-publishing for:%s, Data:%d", tempTopicConfig[i].name, i);
        errorMsg = Publish(tempTopicConfig[i], result);
        isError = strcmp(errorMsg, "0");
        if (isError)
        {
            printf("Publish() API failed, error: %s\n", errorMsg);
        }
        ASSERT_EQ(isError, 0);
    }

    sleep(5);

    for (int i = 0; i < NUM_OF_TOPICS; i++)
    {
        printf("topic%d got %d messages\n", i, topicMsgCount[i]);
        ASSERT_GT(topicMsgCount[i], 0);
    }

    ContextDestroy();
    freeContext(&contextConfigPub);
    freeContext(&contextConfigSub);
    for (int i = 0; i < NUM_OF_TOPICS; i++)
    {
        freeTopic(&tempTopicConfig[i]);
    }
}

TEST(ContextCreateTestCase, NegativeTestcaseSubWithoutPub)
{   
    /*Test description: This is test case which verifies
    the SUB do not get created before PUB.
    */
    struct ContextConfig contextConfigSub;
    char *trustFileArray[2] = {0x00};
    trustFileArray[0] = trustFile;
    char *errorMsg = NULL;
    int isError = 0;

    initContext(&contextConfigSub, certFile, privateFile,
                trustFileArray, 1, "opcua://localhost:65012", sub);
    errorMsg = ContextCreate(contextConfigSub);
    isError = strcmp(errorMsg, "0");
    if (isError)
    {
        printf("ContextCreate() API failed, error: %s\n", errorMsg);
    }
    ASSERT_NE(isError, 0);

    ContextDestroy();
    freeContext(&contextConfigSub);
}

/*TEST(ContextCreateTestCase, NegativeTestcasePubSubHugeData) { 
    // This test case need to be fixed
    // This is crashing.
    struct ContextConfig contextConfigPub;
    struct ContextConfig contextConfigSub;
    
    char *trustFileArray[2] = {0x00};
    trustFileArray[0] = trustFile;
    int numOfTopic = 1;
    initContext(&contextConfigPub, certFile, privateFile, 
        trustFileArray,1,"opcua://localhost:65010",pub);
    char *errorMsg = ContextCreate(contextConfigPub);
    if(strcmp(errorMsg, "0")) {
        printf("ContextCreate() API failed, error: %s\n", errorMsg);
    }

    initContext(&contextConfigSub, certFile, privateFile, 
        trustFileArray,1,"opcua://localhost:65010",sub);
    errorMsg = ContextCreate(contextConfigSub);
    if(strcmp(errorMsg, "0")) {
        printf("ContextCreate() API failed, error: %s\n", errorMsg);
    }
    
    struct TopicConfig tempTopicConfig;
    char topicName[10] = {0x00,};
    sprintf(topicName,"topic%d",1);
    initTopic(&tempTopicConfig,topicName, ns, dtype);
    
    int topicId = 1;
    topicMsgCount[topicId] = 0;
    char result[100] = {0x00};
    sprintf(result, "topic-creation for:%s, Data:%d", tempTopicConfig.name, topicId);
    errorMsg = Publish(tempTopicConfig, result);
    
    errorMsg = Subscribe(&tempTopicConfig, 1, "START", cb, (void*)NULL);
    if(strcmp(errorMsg, "0")) {
        printf("Subscribe() API failed, error: %s\n", errorMsg);
    }
    
    ASSERT_EQ(strcmp(errorMsg, "0"), 0);

    int numChars = (64*1024)+1;
    
    char *dataToBePublished = (char*)calloc(numChars, sizeof(char));
    memset(dataToBePublished,'A',numChars-1);
    Publish(tempTopicConfig, dataToBePublished);
    
    sleep(5);

    printf("topic:%d got %d messages\n",topicId,topicMsgCount[topicId]);
    ASSERT_GT(topicMsgCount[topicId], 0);

    ContextDestroy();
    freeContext(&contextConfigPub);
    freeContext(&contextConfigSub);
    freeTopic(&tempTopicConfig);
    free(dataToBePublished);
}*/

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
