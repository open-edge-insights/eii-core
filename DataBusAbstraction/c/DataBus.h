/*
Copyright (c) 2018 Intel Corporation.

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "open62541_wrappers.h"

// opcua context config
struct ContextConfig {
    char *endpoint;         // opcua endpoint ex:opcua://localhost:65003
    char *direction;        // opcua direction ex: PUB|SUB
    char *name;             // opcua name space
    char *certFile;         // opcua certificates path
    char *privateFile;      // opcua private key file
    char **trustFile;       //opcua trust files list
    size_t trustedListSize; //opcua trust files list size
};

//opcua topic config
struct TopicConfig {
    char *name;   // opcua topic name
    char *dType;  //type of topic, ex: string|int
};

//*************C bindings**********************
char*
ContextCreate(struct ContextConfig contextConfig);

char*
Publish(struct TopicConfig topicConfig,
        char *data);

char*
Subscribe(struct TopicConfig topicConfig,
          char *trig,
          c_callback cb,
          void* pyxFunc);

void ContextDestroy();
