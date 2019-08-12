cdef extern from "DataBus.h":
    struct ContextConfig:
        char *endpoint;
        char *direction;
        char *certFile;
        char *privateFile;
        char **trustFile;
        size_t trustedListSize;

    struct TopicConfig:
        char *ns;
        char *name;
        char *dType;

    ctypedef void (*c_callback)(const char *topic, const char *data, void *pyFunc)

    char* ContextCreate(ContextConfig cxtConfig);

    char* Publish(TopicConfig topicCfg, const char *data);

    char* Subscribe(TopicConfig[] topicConfigs, unsigned int topicConfigCount, const char *trig, c_callback cb, void* pyxFunc);

    void ContextDestroy();