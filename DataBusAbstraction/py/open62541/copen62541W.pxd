cdef extern from "DataBus.h":
    struct ContextConfig:
        char *endpoint;
        char *direction;
        char *certFile;
        char *privateFile;
        char **trustFile;
        size_t trustedListSize;

    struct TopicConfig:
        char *namespace;
        char *name;
        char *dType;

    ctypedef void (*c_callback)(char *topic, char *data, void *pyFunc)

    char* ContextCreate(ContextConfig cxtConfig);

    char* Publish(TopicConfig topicCfg, char *data);

    char* Subscribe(TopicConfig[] topicCfg, int, char *, c_callback cb, void* pyxFunc);

    void ContextDestroy();