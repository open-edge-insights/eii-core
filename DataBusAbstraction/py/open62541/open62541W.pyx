cimport copen62541W
from libc.stdlib cimport malloc, free
from libc.string cimport strcpy

cdef char** to_cstring_array(list_str):
    cdef char **ret = <char **>malloc(len(list_str) * sizeof(char *))
    cdef bytes temp
    for i in xrange(len(list_str)):
        temp = list_str[i].encode()
        ret[i] = temp
    return ret

def ContextCreate(endpoint, direction, name, certFile, privateFile, trustFiles):
  cdef copen62541W.ContextConfig contextConfig
  cdef bytes endpoint_bytes = endpoint.encode();
  cdef char *cendpoint = endpoint_bytes;

  cdef bytes direction_bytes = direction.encode();
  cdef char *cdirection = direction_bytes;

  cdef bytes name_bytes = name.encode();
  cdef char *cname = name_bytes;

  cdef bytes certFile_bytes = certFile.encode();
  cdef char *ccertFile = certFile_bytes;

  cdef bytes keyFile = privateFile.encode();
  cdef char *ckeyFile = keyFile;

  contextConfig.endpoint = cendpoint
  contextConfig.direction = cdirection
  contextConfig.name = cname
  contextConfig.certFile = ccertFile
  contextConfig.privateFile = ckeyFile
  contextConfig.trustFile = to_cstring_array(trustFiles)
  contextConfig.trustedListSize = 1

  val = copen62541W.ContextCreate(contextConfig)
  free(contextConfig.trustFile)
  return val

def Publish(topicConf, data):
  cdef copen62541W.TopicConfig topicConfig

  cdef bytes topic_bytes = topicConf['name'].encode();
  cdef char *ctopic = topic_bytes;

  cdef bytes dtype_bytes = topicConf['dtype'].encode();
  cdef char *cdtype = dtype_bytes;

  cdef bytes data_bytes = data.encode();
  cdef char *cdata = data_bytes;

  topicConfig.name =  ctopic
  topicConfig.dType = cdtype
  return copen62541W.Publish(topicConfig, cdata)

cdef void pyxCallback(char *topic, char *data, void *func) with gil:
  (<object>func)(topic, data)

def Subscribe(topicConf, trig, pyFunc):
  cdef copen62541W.TopicConfig topicConfig

  cdef bytes topic_bytes = topicConf['name'].encode();
  cdef char *ctopic = topic_bytes;

  cdef bytes dtype_bytes = topicConf['dtype'].encode();
  cdef char *cdtype = dtype_bytes;

  cdef bytes trig_bytes = trig.encode();
  cdef char *ctrig = trig_bytes;

  topicConfig.name =  ctopic
  topicConfig.dType = cdtype

  val = copen62541W.Subscribe(topicConfig, ctrig, pyxCallback, <void *> pyFunc)
  return val

def ContextDestroy():
  copen62541W.ContextDestroy()

