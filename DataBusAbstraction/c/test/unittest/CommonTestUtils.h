#ifndef __CTU__
#define __CTU__
extern "C"
{
#include <DataBus.h>
}

void initContext(struct ContextConfig *contextConfig, char *certFile, char *privateFile,
                 char **trustFileArray, int numOfTrustFile, char *endPoint, char *opType);

void freeContext(struct ContextConfig *contextConfig);

void initTopic(struct TopicConfig *tempTopicConfig, char *topic, char *nmSpace, char *dataType);

void freeTopic(struct TopicConfig *tempTopicConfig);

#endif
