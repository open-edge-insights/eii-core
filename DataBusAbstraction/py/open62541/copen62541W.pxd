cdef extern from "DataBus.h":
    struct ContextConfig:
        char *endpoint;
        char *direction;
        char *name;
        char *certFile;
        char *privateFile;
        char **trustFile;
        size_t trustedListSize;

    struct TopicConfig:
        char *name;
        char *dType;

    ctypedef void (*c_callback)(char *topic, char *data, void *pyFunc)

    char * ContextCreate(ContextConfig);

    char *Publish(TopicConfig, char *);

    char * Subscribe(TopicConfig, char *, c_callback cb, void* pyxFunc);

    void ContextDestroy();