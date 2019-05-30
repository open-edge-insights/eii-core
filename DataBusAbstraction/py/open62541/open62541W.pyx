cimport copen62541W
from libc.stdlib cimport malloc, free
from libc.string cimport strcpy, strlen

cdef copen62541W.TopicConfig *cTopicConfig
gTopicConfigCount = 0

cdef char** to_cstring_array(list_str):
    cdef char **ret = <char **>malloc(len(list_str) * sizeof(char *))
    cdef bytes temp
    for i in xrange(len(list_str)):
        temp = list_str[i].encode()
        ret[i] = temp
    return ret

def ContextCreate(endpoint, direction, certFile, privateFile, trustFiles):
  cdef copen62541W.ContextConfig contextConfig
  cdef bytes endpoint_bytes = endpoint.encode();
  cdef char *cendpoint = endpoint_bytes;

  cdef bytes direction_bytes = direction.encode();
  cdef char *cdirection = direction_bytes;

  cdef bytes certFile_bytes = certFile.encode();
  cdef char *ccertFile = certFile_bytes;

  cdef bytes keyFile = privateFile.encode();
  cdef char *ckeyFile = keyFile;

  contextConfig.endpoint = cendpoint
  contextConfig.direction = cdirection
  contextConfig.certFile = ccertFile
  contextConfig.privateFile = ckeyFile
  contextConfig.trustFile = to_cstring_array(trustFiles)
  contextConfig.trustedListSize = len(trustFiles)

  val = copen62541W.ContextCreate(contextConfig)
  free(contextConfig.trustFile)
  return val

def Publish(topicConf, data):
  cdef copen62541W.TopicConfig topicConfig

  cdef bytes namespace_bytes = topicConf['ns'].encode();
  cdef char *cnamespace = namespace_bytes;

  cdef bytes topic_bytes = topicConf['name'].encode();
  cdef char *ctopic = topic_bytes;

  cdef bytes dtype_bytes = topicConf['dType'].encode();
  cdef char *cdtype = dtype_bytes;

  cdef bytes data_bytes = data.encode();
  cdef char *cdata = data_bytes;

  topicConfig.ns = cnamespace
  topicConfig.name =  ctopic
  topicConfig.dType = cdtype
  return copen62541W.Publish(topicConfig, cdata)

cdef void pyxCallback(char *topic, char *data, void *func) with gil:
  (<object>func)(topic, data)

def Subscribe(topicConfigs, topicConfigCount, trig, pyFunc):
  cTopicConfig = <copen62541W.TopicConfig *>malloc(topicConfigCount * sizeof(copen62541W.TopicConfig))
  cdef bytes topic_bytes
  cdef char *ctopic
  cdef bytes namespace_bytes
  cdef char *cnamespace
  cdef bytes dtype_bytes
  cdef char *cdtype

  for i in range(topicConfigCount):
    namespace_bytes = topicConfigs[i]["ns"].encode();
    cnamespace = namespace_bytes;

    topic_bytes = topicConfigs[i]["name"].encode();
    ctopic = topic_bytes;

    dtype_bytes = topicConfigs[i]["dType"].encode();
    cdtype = dtype_bytes;

    cTopicConfig[i].ns = <char *>malloc(strlen(cnamespace) + 1)
    strcpy(cTopicConfig[i].ns, cnamespace)
    cTopicConfig[i].name = <char *>malloc(strlen(ctopic) + 1)
    strcpy(cTopicConfig[i].name, ctopic)
    cTopicConfig[i].dType = <char *>malloc(strlen(cdtype) + 1)
    strcpy(cTopicConfig[i].dType, cdtype)

  cdef bytes trig_bytes = trig.encode();
  cdef char *ctrig = trig_bytes;

  val = copen62541W.Subscribe(cTopicConfig, topicConfigCount, ctrig, pyxCallback, <void *> pyFunc)
  gTopicConfigCount = topicConfigCount

  return val

def ContextDestroy():
  copen62541W.ContextDestroy()
  for i in range(gTopicConfigCount):
    if cTopicConfig[i].name is not NULL:
      free(cTopicConfig[i].name)
    if cTopicConfig[i].dType is not NULL:
      free(cTopicConfig[i].dType)

  if cTopicConfig is not NULL:
    free(cTopicConfig)

