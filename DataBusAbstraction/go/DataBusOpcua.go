/*
Copyright (c) 2018 Intel Corporation.

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

package databus

/*
#cgo CFLAGS: -I ../c/open62541/include
#cgo LDFLAGS: -L ../c/open62541/src -lsafestring -lopen62541W -lmbedtls -lmbedx509 -lmbedcrypto -pthread
void cgoFunc(char *topic, char *data);
#include <stdlib.h>
#include "open62541_wrappers.h"
#include <stdio.h>
*/
import "C"

import (
	"strconv"
	"strings"
	"unsafe"

	"github.com/golang/glog"
)

type dataBusOpcua struct {
	namespace string
	direction string
	nsIndex   int
}

func newOpcuaInstance() (db *dataBusOpcua, err error) {
	defer errHandler("OPCUA New Instance Creation Failed!!!", &err)
	db = &dataBusOpcua{}
	return
}

var gCh chan interface{}

//export goCallback
func goCallback(topic *C.char, data *C.char) {
	glog.Infoln("In goCallback() fucntion...")
	dataGoStr := C.GoString(data)
	gCh <- dataGoStr
}

//TODO: Debug the crash seen when this is called
//frees up all CString() allocated memory
func free(cStrs []*C.char) {
	for _, str := range cStrs {
		C.free(unsafe.Pointer(str))
	}
}

func (dbOpcua *dataBusOpcua) createContext(contextConfig map[string]string) (err error) {
	defer errHandler("OPCUA Context Creation Failed!!!", &err)
	dbOpcua.direction = contextConfig["direction"]
	dbOpcua.namespace = contextConfig["name"]
	endpoint := contextConfig["endpoint"]

	hostPort := strings.Split(endpoint, "://")[1]
	host := strings.Split(hostPort, ":")[0]
	port, err := strconv.Atoi(strings.Split(hostPort, ":")[1])
	if err != nil {
		glog.Errorf("port int conversion failed, error: %v", err)
		panic(err)
	}

	cHostname := C.CString(host)
	cCertFile := C.CString(contextConfig["certFile"])
	cPrivateFile := C.CString(contextConfig["privateFile"])

	//TODO - Make contextConfig["trustFile"] an array
	trustFiles := [1]string{contextConfig["trustFile"]}
	cArray := C.malloc(C.size_t(len(trustFiles)) * C.size_t(unsafe.Sizeof(uintptr(0))))
	a := (*[1<<30 - 1]*C.char)(cArray)
	for idx, substring := range trustFiles {
		a[idx] = C.CString(substring)
	}

	if dbOpcua.direction == "PUB" {
		cResp := C.serverContextCreate(cHostname, C.int(port), cCertFile, cPrivateFile, (**C.char)(cArray), 1)

		goResp := C.GoString(cResp)
		if goResp != "0" {
			glog.Errorln("Response: ", goResp)
			panic(goResp)
		}
	}
	if dbOpcua.direction == "SUB" {
		cResp := C.clientContextCreate(cHostname, C.int(port), cCertFile, cPrivateFile, (**C.char)(cArray), 1)
		goResp := C.GoString(cResp)
		if goResp != "0" {
			glog.Errorln("Response: ", goResp)
			panic(goResp)
		}
	}

	return
}

func (dbOpcua *dataBusOpcua) startTopic(topicConfig map[string]string) (err error) {
	defer errHandler("OPCUA Topic Start Failed!!!", &err)
	if dbOpcua.direction == "PUB" {
		cNamespace := C.CString(dbOpcua.namespace)
		cTopic := C.CString(topicConfig["name"])

		nsIndex := C.serverStartTopic(cNamespace, cTopic)
		dbOpcua.nsIndex = int(nsIndex)
		if dbOpcua.nsIndex == 100 {
			panic("Failed to add opcua variable for the topic!!")
		}
	} else if dbOpcua.direction == "SUB" {
		//TODO: opcua integration needs to be done
		cNamespace := C.CString(dbOpcua.namespace)
		cTopic := C.CString(topicConfig["name"])

		nsIndex := C.clientStartTopic(cNamespace, cTopic)
		dbOpcua.nsIndex = int(nsIndex)
		if dbOpcua.nsIndex == 100 {
			panic("opcua variable for the topic doesn't exist!!")
		}
	}
	return
}

func (dbOpcua *dataBusOpcua) send(topic map[string]string, msgData interface{}) (err error) {
	defer errHandler("OPCUA Send Failed!!!", &err)
	if dbOpcua.direction == "PUB" {
		cTopic := C.CString(topic["name"])
		cMsgData := C.CString(msgData.(string))
		cResp := C.serverPublish(C.int(dbOpcua.nsIndex), cTopic, cMsgData)
		goResp := C.GoString(cResp)
		if goResp != "0" {
			glog.Errorln("Response: ", goResp)
			panic(goResp)
		}
	}
	return
}

func (dbOpcua *dataBusOpcua) receive(topic map[string]string, trig string, ch chan interface{}) (err error) {
	defer errHandler("OPCUA Receive Failed!!!", &err)
	//TODO: opcua integration needs to be done
	if dbOpcua.direction == "SUB" {
		cTopic := C.CString(topic["name"])
		gCh = ch
		cResp := C.clientSubscribe(C.int(dbOpcua.nsIndex), cTopic, (C.c_callback)(unsafe.Pointer(C.cgoFunc)), nil)
		goResp := C.GoString(cResp)
		if goResp != "0" {
			glog.Errorln("Response: ", goResp)
			panic(goResp)
		}
	}
	return
}

func (dbOpcua *dataBusOpcua) stopTopic(topic string) (err error) {
	defer errHandler("OPCUA Topic Stop Failed!!!", &err)
	return
}

func (dbOpcua *dataBusOpcua) destroyContext() (err error) {
	defer errHandler("OPCUA Context Termination Failed!!!", &err)
	if dbOpcua.direction == "PUB" {
		C.serverContextDestroy()
	} else if dbOpcua.direction == "SUB" {
		C.clientContextDestroy()
	}
	return
}
