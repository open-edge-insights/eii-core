package databus

import (
	"github.com/golang/glog"
	"strings"

	mqtt "github.com/eclipse/paho.mqtt.golang"
)

type dataBusMqtt struct {
	client    mqtt.Client
	direction string
}

func newMqttInstance() (db *dataBusMqtt) {
	defer func() {
		if r := recover(); r != nil {
			glog.Errorln(r)
			glog.Errorln("MQTT New Instance Creation Failed!!!")
			db = nil
		}
	}()
	db = &dataBusMqtt{}
	return
}

func (dbMqtt *dataBusMqtt) createContext(contextConfig map[string]string) (ret int) {
	defer func() {
		if r := recover(); r != nil {
			glog.Errorln(r)
			glog.Errorln("MQTT Context Creation Failed!!!")
			ret = -1
		}
	}()
	dbMqtt.direction = contextConfig["direction"]
	if dbMqtt.direction == "PUB" {
		brokerUrl := "tcp://" + strings.Split(contextConfig["endpoint"], "//")[1]
		//TODO: set keepalive too?
		opts := mqtt.NewClientOptions().AddBroker(brokerUrl).SetClientID(contextConfig["name"])
		dbMqtt.client = mqtt.NewClient(opts)
		if token := dbMqtt.client.Connect(); token.Wait() && token.Error() != nil {
			panic(token.Error())
		}
	}
	return
}

func (dbMqtt *dataBusMqtt) startTopic(topicConfig map[string]string) (ret int) {
	defer func() {
		if r := recover(); r != nil {
			glog.Errorln(r)
			glog.Errorln("MQTT Topic Start Failed!!!")
			ret = -1
		}
	}()
	if dbMqtt.direction == "PUB" {
	}
	if dbMqtt.direction == "SUB" {
	}
	return
}

func (dbMqtt *dataBusMqtt) send(topic map[string]string, msgData interface{}) (ret int) {
	defer func() {
		if r := recover(); r != nil {
			glog.Errorln(r)
			glog.Errorln("MQTT Send Failed!!!")
			ret = -1
		}
	}()
	if topic["type"] == "string" {
		if token := dbMqtt.client.Publish(topic["name"], 1, false, msgData.(string)); token.Wait() && token.Error() != nil {
			panic(token.Error())
		}
	}
	return
}

func (dbMqtt *dataBusMqtt) receive(topic string) (ret int) {
	defer func() {
		if r := recover(); r != nil {
			glog.Errorln(r)
			glog.Errorln("MQTT Receive Failed!!!")
			ret = -1
		}
	}()
	return
}

func (dbMqtt *dataBusMqtt) stopTopic(topic string) (ret int) {
	defer func() {
		if r := recover(); r != nil {
			glog.Errorln(r)
			glog.Errorln("MQTT Topic Stop Failed!!!")
			ret = -1
		}
	}()
	return
}

func (dbMqtt *dataBusMqtt) destroyContext() (ret int) {
	defer func() {
		if r := recover(); r != nil {
			glog.Errorln(r)
			glog.Errorln("MQTT Context Termination Failed!!!")
			ret = -1
		}
	}()
	glog.Infoln("MQTT Client Disconnecting...")
	dbMqtt.client.Disconnect(100)
	glog.Infoln("MQTT Client Disconnected")
	return
}
