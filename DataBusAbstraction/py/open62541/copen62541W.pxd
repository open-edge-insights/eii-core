cdef extern from "open62541_wrappers.h":
    ctypedef void (*c_callback)(char *topic, char *data, void *pyFunc)
    char* serverContextCreate(char *hostname,
                              int port,
                              char *certificateFile, 
                              char *privateKeyFile,
                              char **trustList,
                              size_t trustListSize);

    int serverStartTopic(char *ns, 
                        char *topic);

    char* serverPublish(int nsIndex, 
                        char *topic,
                        char *data);

    void serverContextDestroy();

    char* clientContextCreate(char *hostname,
                              int port,
                              char *certificateFile,
                              char *privateKeyFile,
                              char **trustList,
                              size_t trustListSize);

    int clientStartTopic(char *ns, 
                         char *topic);

    char* clientSubscribe(int nsIndex, 
                          char* topic,
                          c_callback cb, void* userFunc);
                                
    void clientContextDestroy();
