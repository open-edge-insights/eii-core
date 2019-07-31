/*
Copyright (c) 2018 Intel Corporation.

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

package databus

import (
	"errors"
	"strings"
	"sync"

	"github.com/golang/glog"
)

type dataBusContext interface {
	createContext(map[string]string) error
	startTopic(map[string]string) error
	send(map[string]string, interface{}) error
	receive([]map[string]string, int, string, chan interface{}) error
	stopTopic(string) error
	destroyContext() error
}

type DataBus interface {
	ContextCreate(map[string]string) error
	Publish(map[string]string, interface{}) error
	Subscribe([]map[string]string, int, string, CbType) error
	ContextDestroy() error
}

type topicMeta struct {
	topicType string
	data      chan interface{}
	ev        chan string
}

type dataBus struct {
	busType   string
	direction string
	pubTopics map[string]*topicMeta
	subTopics map[string]*topicMeta
	bus       dataBusContext
	mutex     *sync.Mutex
}

// TODO: A dynamic importing?
var dataBusTypes = map[string]string{"OPCUA": "opcua:"}

func errHandler(errMsg string, err *error) {
	if r := recover(); r != nil {
		glog.Errorln(r)
		glog.Errorln(errMsg)
		*err = errors.New(errMsg)
	}
}

// NewDataBus function to create an instance for DataBus
func NewDataBus() (db *dataBus, err error) {
	defer errHandler("Couldnt create new databus!!!", &err)
	db = &dataBus{}
	db.pubTopics = map[string]*topicMeta{}
	db.subTopics = map[string]*topicMeta{}
	db.mutex = &sync.Mutex{}
	return
}

func (dbus *dataBus) ContextCreate(contextConfig map[string]string) (err error) {
	defer errHandler("DataBus Context Creation Failed!!!", &err)
	dbus.mutex.Lock()
	defer dbus.mutex.Unlock()
	endPoint := contextConfig["endpoint"]

	switch strings.Split(endPoint, "//")[0] {
	case dataBusTypes["OPCUA"]:
		dbus.busType = dataBusTypes["OPCUA"]
		//TODO: Error check
		dbus.bus, err = newOpcuaInstance()
		if err != nil {
			panic("newOpcuaInstance() Failed!!!")
		}
	default:
		panic("Unsupported DataBus Type!!!")
	}
	//TODO: Error check
	err = dbus.bus.createContext(contextConfig)
	if err != nil {
		panic("createContext() Failed!!!")
	}
	if contextConfig["direction"] == "PUB" {
		dbus.direction = "PUB"
	}
	if contextConfig["direction"] == "SUB" {
		dbus.direction = "SUB"
	}
	glog.Infoln("DataBus Context Created Successfully")
	return
}

func (dbus *dataBus) Publish(topicConfig map[string]string, msgData interface{}) (err error) {
	defer errHandler("DataBus Publish Failed!!!", &err)
	if strings.Contains(dbus.busType, "opcua") {
		err = dbus.bus.send(topicConfig, msgData)
		if err != nil {
			panic("send() Failed!!!")
		}
	}
	return
}

// CbType interface to the user callback function
type CbType func(topic string, msg interface{})

func worker(topic string, dch chan interface{}, ech chan string, cb CbType) {
	for {
		select {
		case data := <-dch:
			glog.V(1).Infoln("Worker receiving...")
			cb(topic, data)
		case <-ech:
			glog.V(1).Infoln("Worker terminating...")
			return
		}
	}
}

func (dbus *dataBus) Subscribe(topicConfigs []map[string]string, totalConfigs int, trig string, cb CbType) (err error) {
	defer errHandler("DataBus Subscription Failed!!!", &err)

	//TODO: Fix the opcua subscriber logic
	if strings.Contains(dbus.busType, "opcua") {
		if dbus.direction == "SUB" && trig == "START" && cb != nil {
			//dch := make(chan interface{})
			//ech := make(chan string)
			// go worker(topicConfig["name"], dch, ech, cb)
			// err = dbus.bus.receive(topicConfigs, totalConfigs, "START", dch)
			// if err != nil {
			// 	panic("receive() Failed!!!")
			// }
		}
	}
	return
}

func (dbus *dataBus) ContextDestroy() (err error) {
	defer errHandler("DataBus Context Termination Failed!!!", &err)
	dbus.bus.destroyContext()
	dbus.direction = ""
	glog.Infoln("DataBus Context Terminated")
	return
}

func (dbus *dataBus) checkMsgType(topicType string, msg interface{}) (ret bool) {
	switch msg.(type) {
	case string:
		if topicType == "string" {
			ret = true
		}
	}
	return
}
