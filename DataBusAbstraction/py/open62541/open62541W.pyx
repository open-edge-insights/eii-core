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

def py_serverContextCreate(host, port, certificateFile, privateKeyFile, trustList):
  cdef bytes hostname  = host.encode();
  cdef char *chostname = hostname;

  cdef int cport = port;

  cdef bytes certFile = certificateFile.encode();
  cdef char *ccertFile = certFile;

  cdef bytes keyFile = privateKeyFile.encode();
  cdef char *ckeyFile = keyFile;

  cdef int ctrustListSize = len(trustList);

  cdef char **ctrustList = to_cstring_array(trustList);

  cdef char *nil = NULL;

  val = copen62541W.serverContextCreate(chostname, cport, ccertFile,
                                       ckeyFile, ctrustList, ctrustListSize)
  free(ctrustList)
  return val

def py_serverStartTopic(ns, topic):
  cdef bytes namespace = ns.encode();
  cdef char *cnamespace = namespace;

  cdef bytes topic_bytes = topic.encode();
  cdef char *ctopic = topic_bytes;

  return copen62541W.serverStartTopic(cnamespace, ctopic)

def py_serverPublish(nsIndex, topic, data):
  cdef int cnsIndex = nsIndex;

  cdef bytes topic_bytes = topic.encode();
  cdef char *ctopic = topic_bytes;

  cdef bytes data_bytes = data.encode();
  cdef char *cdata = data_bytes;

  return copen62541W.serverPublish(cnsIndex, ctopic, cdata)

def py_serverContextDestroy():
  copen62541W.serverContextDestroy()


def py_clientContextCreate(host, port, certificateFile, privateKeyFile, trustList):
  cdef bytes host_bytes  = host.encode();
  cdef char *chostname = host_bytes;

  cdef int cport = port;

  cdef bytes certFile = certificateFile.encode();
  cdef char *ccertFile = certFile

  cdef bytes keyFile = privateKeyFile.encode();
  cdef char *ckeyFile = keyFile

  cdef int ctrustListSize = len(trustList);
  cdef char **ctrustList = to_cstring_array(trustList);
  
  val = copen62541W.clientContextCreate(chostname, cport, ccertFile, ckeyFile, ctrustList,
                                 ctrustListSize)
  free(ctrustList)
  return val

cdef void pyxCallback(char *topic, char *data, void *func) with gil:
  (<object>func)(topic.decode('utf-8'), data.decode('utf-8'))

def py_clientStartTopic(ns, topic):
  cdef bytes namespace = ns.encode();
  cdef char *cnamespace = namespace;

  cdef bytes topic_bytes = topic.encode();
  cdef char *ctopic = topic_bytes;

  return copen62541W.clientStartTopic(cnamespace, ctopic)

def py_clientSubscribe(nsIndex, topic, pyFunc):
  cdef int cnsIndex = nsIndex;

  cdef bytes topic_bytes = topic.encode();
  cdef char *ctopic = topic_bytes;

  val = copen62541W.clientSubscribe(cnsIndex, ctopic, pyxCallback, <void *> pyFunc)
  return val

def py_clientContextDestroy():
  copen62541W.clientContextDestroy()
