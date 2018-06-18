package databus

import (
	"github.com/golang/glog"
	"strings"
)

type dataBusContext interface {
	//getDataBusType() string
	createContext(map[string]string) int
	startTopic(map[string]string) int
	send(map[string]string, interface{}) int
	receive(string) int
	stopTopic(string) int
	destroyContext() int
}

type DataBus interface {
	ContextCreate(map[string]string) int
	Publish(map[string]string, interface{}) int
	Subscribe(map[string]string) int
	ContextDestroy() int
}

type dataBus struct {
	databType      string
	databDirection string
	databTopics    map[string]string
	databContext   dataBusContext
}

func NewDataBus() DataBus {
	db := &dataBus{}
	db.databTopics = map[string]string{}
	return db
}

// TODO: A dynamic importing?
var dataBusTypes = map[string]string{"OPCUA": "opcua:", "MQTT": "mqtt:", "NATS": "nats:"}

// TODO: NewDataBus API?
func (dbus *dataBus) ContextCreate(contextConfig map[string]string) (ret int) {
	defer func() {
		if r := recover(); r != nil {
			glog.Errorln(r)
			glog.Errorln("DataBus Context Creation Failed!!!")
			ret = -1
		}
	}()
	endPoint := contextConfig["endpoint"]
	switch strings.Split(endPoint, "//")[0] {
	case dataBusTypes["OPCUA"]:
		dbus.databType = dataBusTypes["OPCUA"]
		//TODO: Error check
		dbus.databContext = newOpcuaInstance()
	case dataBusTypes["MQTT"]:
		dbus.databType = dataBusTypes["MQTT"]
		//TODO: Error check
		dbus.databContext = newMqttInstance()
	case dataBusTypes["NATS"]:
		dbus.databType = dataBusTypes["NATS"]
		panic("TBD!!!")
	default:
		panic("Unsupported DataBus Type!!!")
	}
	//TODO: Error check
	if 0 != dbus.databContext.createContext(contextConfig) {
		panic("createContext() Failed!!!")
	}
	if contextConfig["direction"] == "PUB" {
		dbus.databDirection = "PUB"
	}
	if contextConfig["direction"] == "SUB" {
		dbus.databDirection = "SUB"
	}
	glog.Infoln("DataBus Context Created Successfully")
	return
}

func (dbus *dataBus) Publish(topicConfig map[string]string, msgData interface{}) (ret int) {
	defer func() {
		if r := recover(); r != nil {
			glog.Errorln(r)
			glog.Errorln("DataBus Publish Failed!!!")
			ret = -1
		}
	}()
	if dbus.databDirection == "PUB" {
		if !dbus.checkMsgType(topicConfig["type"], msgData) {
			panic("Message Type Not supported")
		}
		if _, ok := dbus.databTopics[topicConfig["name"]]; !ok {
			dbus.databContext.startTopic(topicConfig)
			dbus.databTopics[topicConfig["name"]] = topicConfig["type"]
		}
		if dbus.databTopics[topicConfig["name"]] != topicConfig["type"] {
			panic("Topic name & type NOT matching")
		}
		if 0 != dbus.databContext.send(topicConfig, msgData) {
			panic("send Failed!!!")
		}
	}
	return
}

//TODO
func (dbus *dataBus) Subscribe(topicConfig map[string]string) (ret int) {
	defer func() {
		if r := recover(); r != nil {
			glog.Errorln(r)
			glog.Errorln("DataBus Subscription Failed!!!")
			ret = -1
		}
	}()
	return
}

func (dbus *dataBus) ContextDestroy() (ret int) {
	defer func() {
		if r := recover(); r != nil {
			glog.Errorln(r)
			glog.Errorln("DataBus Context Termination Failed!!!")
			ret = -1
		}
	}()
	//TODO: Error check
	dbus.databContext.destroyContext()
	dbus.databDirection = ""
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
