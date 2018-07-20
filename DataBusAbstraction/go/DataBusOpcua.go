package databus

import (
	"github.com/golang/glog"
	"github.com/sbinet/go-python"
	"strconv"
	"strings"
)

type dataBusOpcua struct {
	pyThread  *python.PyThreadState
	pyServer  *python.PyObject
	pyNs      *python.PyObject
	direction string
}

func newOpcuaInstance() (db *dataBusOpcua, err error) {
	defer errHandler("OPCUA New Instance Creation Failed!!!", &err)
	db = &dataBusOpcua{}
	err = python.Initialize()
	if err != nil {
		panic(err.Error())
	}
	db.pyThread = python.PyEval_SaveThread()
	return
}

func (dbOpcua *dataBusOpcua) createContext(contextConfig map[string]string) (err error) {
	defer errHandler("OPCUA Context Creation Failed!!!", &err)
	dbOpcua.direction = contextConfig["direction"]
	if dbOpcua.direction == "PUB" {
		serverUrl := "opc.tcp://" + strings.Split(contextConfig["endpoint"], "//")[1]
		glog.Infoln("serverURL: ", serverUrl)
		//TODO: set keepalive too?
		python.PyEval_RestoreThread(dbOpcua.pyThread)
		defer func() {
			dbOpcua.pyThread = python.PyEval_SaveThread()
		}()
		glog.Infoln("Python Thread Restored")
		pyModule := python.PyImport_ImportModule("opcua.server.server")
		if pyModule == nil {
			panic("Module load Failed")
		}
		pyClass := pyModule.GetAttrString("Server")
		if pyClass == nil {
			panic("No such class!!!")
		}
		pyModule.Clear()
		pyArgs := python.PyTuple_New(0)
		if pyArgs == nil {
			panic("Can't build argument list")
		}
		dbOpcua.pyServer = pyClass.CallObject(pyArgs)
		if dbOpcua.pyServer == nil {
			panic("Object creation failed!!!")
		}
		pyArgs.Clear()
		pyRet := dbOpcua.pyServer.CallMethodObjArgs("set_endpoint", python.PyString_FromString(serverUrl))
		if pyRet == nil {
			panic("Endpoint setting failed!!!!")
		}
		dbOpcua.pyNs = dbOpcua.pyServer.CallMethodObjArgs("register_namespace", python.PyString_FromString(contextConfig["name"]))
		if dbOpcua.pyNs == nil {
			panic("Namespace registration failed!!!!")
		} else {
			glog.Infoln(strconv.Itoa(python.PyInt_AsLong(dbOpcua.pyNs)))
		}
		dbOpcua.pyServer.CallMethodObjArgs("start")
	}
	return
}

func (dbOpcua *dataBusOpcua) startTopic(topicConfig map[string]string) (err error) {
	defer errHandler("OPCUA Topic Start Failed!!!", &err)
	if dbOpcua.direction == "PUB" {
		topicSlice := strings.Split(topicConfig["name"], "/")
		// Restore the python thread state
		python.PyEval_RestoreThread(dbOpcua.pyThread)
		defer func() {
			dbOpcua.pyThread = python.PyEval_SaveThread()
		}()
		pyObjsRoot := dbOpcua.pyServer.CallMethodObjArgs("get_objects_node")
		if pyObjsRoot == nil {
			panic("No Object root node found")
		}
		pyObj := pyObjsRoot
		for idx := range topicSlice {
			pyChild := pyObj.CallMethodObjArgs("get_child", python.PyString_FromString(strconv.Itoa(python.PyInt_AsLong(dbOpcua.pyNs))+":"+topicSlice[idx]))
			if pyChild == nil {
				//This is twisted; but for now it has to do...
				pyTypeO, pyValO, pyTraceO := python.PyErr_Fetch()
				_, pyVal, _ := python.PyErr_NormalizeException(pyTypeO, pyValO, pyTraceO)
				glog.Infoln(pyVal)
				excString := python.PyString_AsString(pyVal.Str())
				if strings.Contains(excString, "(BadNoMatch)") {
					pyChild = pyObj.CallMethodObjArgs("add_object", dbOpcua.pyNs, python.PyString_FromString(topicSlice[idx]))
				} else {
					panic("Unknown Python Exception")
				}
			} else {
				glog.Infoln(pyChild)
			}
			pyObj = pyChild
		}
		//TODO: Support other types too
		pyVar := pyObj.CallMethodObjArgs("add_variable", dbOpcua.pyNs, python.PyString_FromString(topicSlice[len(topicSlice)-1]+"_var"), python.PyString_FromString("NONE"))
		pyVar.CallMethodObjArgs("set_writable")
	}
	return
}

func (dbOpcua *dataBusOpcua) send(topic map[string]string, msgData interface{}) (err error) {
	defer errHandler("OPCUA Send Failed!!!", &err)
	if dbOpcua.direction == "PUB" {
		topicSlice := strings.Split(topic["name"], "/")
		// Restore the python thread state
		python.PyEval_RestoreThread(dbOpcua.pyThread)
		defer func() {
			dbOpcua.pyThread = python.PyEval_SaveThread()
		}()
		pyObjsRoot := dbOpcua.pyServer.CallMethodObjArgs("get_objects_node")
		if pyObjsRoot == nil {
			panic("No Object root node found")
		}
		pyObj := pyObjsRoot
		for idx := range topicSlice {
			pyChild := pyObj.CallMethodObjArgs("get_child", python.PyString_FromString(strconv.Itoa(python.PyInt_AsLong(dbOpcua.pyNs))+":"+topicSlice[idx]))
			if pyChild == nil {
				//python.PyErr_Print()
				panic("No child object found!!!")
			}
			pyObj = pyChild
		}
		pyVar := pyObj.CallMethodObjArgs("get_child", python.PyString_FromString(strconv.Itoa(python.PyInt_AsLong(dbOpcua.pyNs))+":"+topicSlice[len(topicSlice)-1]+"_var"))
		if topic["type"] == "string" {
			pyVar.CallMethodObjArgs("set_value", python.PyString_FromString(msgData.(string)))
		}
	}
	return
}

func (dbOpcua *dataBusOpcua) receive(topic map[string]string, trig string, ch chan interface{}) (err error) {
	defer errHandler("OPCUA Receive Failed!!!", &err)
	return
}

func (dbOpcua *dataBusOpcua) stopTopic(topic string) (err error) {
	defer errHandler("OPCUA Topic Stop Failed!!!", &err)
	return
}

func (dbOpcua *dataBusOpcua) destroyContext() (err error) {
	defer errHandler("OPCUA Context Termination Failed!!!", &err)
	if dbOpcua.direction == "PUB" {
		glog.Infoln("OPCUA Server Stopping....")
		python.PyEval_RestoreThread(dbOpcua.pyThread)
		defer func() {
			dbOpcua.pyThread = python.PyEval_SaveThread()
		}()
		dbOpcua.pyServer.CallMethodObjArgs("stop")
		glog.Infoln("OPCUA Server Stopped")
	}
	return
}
