#include <stdlib.h>
#include <CommonTestUtils.h>

void initContext(struct ContextConfig *contextConfig, char *certFile, char *privateFile,
                 char **trustFileArray, int numOfTrustFile, char *endPoint, char *opType)
{
    int len = 0;
    if (certFile)
    {
        len = strlen(certFile);
        contextConfig->certFile = (char *)calloc(len + 1, sizeof(char));
        strcpy_s(contextConfig->certFile, len + 1, certFile);
    }
    else
    {
        contextConfig->certFile = NULL;
    }

    if (privateFile)
    {
        len = strlen(privateFile);
        contextConfig->privateFile = (char *)calloc(len + 1, sizeof(char));
        strcpy_s(contextConfig->privateFile, len + 1, privateFile);
    }
    else
    {
        contextConfig->privateFile = NULL;
    }

    if (numOfTrustFile > 0)
    {
        contextConfig->trustFile = (char **)calloc(numOfTrustFile, sizeof(char *));
        contextConfig->trustedListSize = numOfTrustFile;

        for (int i = 0; i < numOfTrustFile; i++)
        {
            len = strlen(trustFileArray[i]);
            contextConfig->trustFile[i] = (char *)calloc(len + 1, sizeof(char));
            strcpy_s(contextConfig->trustFile[i], len + 1, trustFileArray[i]);
        }
    }
    else
    {
        contextConfig->trustFile = NULL;
        contextConfig->trustedListSize = 0;
    }

    if (endPoint)
    {
        len = strlen(endPoint);
        contextConfig->endpoint = (char *)calloc(len + 1, sizeof(char));
        strcpy_s(contextConfig->endpoint, len + 1, endPoint);
    }
    else
    {
        contextConfig->endpoint = NULL;
    }

    if (opType)
    {
        len = strlen(opType);
        contextConfig->direction = (char *)calloc(len + 1, sizeof(char));
        strcpy_s(contextConfig->direction, len + 1, opType);
    }
    else
    {
        contextConfig->direction = NULL;
    }
}

void freeContext(struct ContextConfig *contextConfig)
{
    if (contextConfig)
    {
        if (contextConfig->certFile)
            free(contextConfig->certFile);

        if (contextConfig->privateFile)
            free(contextConfig->privateFile);

        for (int i = 0; i < contextConfig->trustedListSize; i++)
            free(contextConfig->trustFile[i]);

	free(contextConfig->trustFile);

        if (contextConfig->endpoint)
            free(contextConfig->endpoint);

        if (contextConfig->direction)
            free(contextConfig->direction);
    }
}

void initTopic(struct TopicConfig *tempTopicConfig, char *topic, char *nmSpace, char *dataType)
{
    int len = 0;
    len = strlen(dataType) + 1;
    tempTopicConfig->dType = (char *)calloc(len, sizeof(char));
    strcpy_s(tempTopicConfig->dType, len, dataType);

    len = strlen(topic) + 1;
    tempTopicConfig->name = (char *)calloc(len, sizeof(char));
    strcpy_s(tempTopicConfig->name, len, topic);

    len = strlen(nmSpace) + 1;
    tempTopicConfig->ns = (char *)calloc(len, sizeof(char));
    strcpy_s(tempTopicConfig->ns, len, nmSpace);
}

void freeTopic(struct TopicConfig *tempTopicConfig)
{
    if (tempTopicConfig->dType)
        free(tempTopicConfig->dType);

    if (tempTopicConfig->name)
        free(tempTopicConfig->name);

    if (tempTopicConfig->ns)
        free(tempTopicConfig->ns);
}
