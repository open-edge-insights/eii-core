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

func newMqttInstance() (db *dataBusMqtt, err error) {
	defer errHandler("MQTT New Instance Creation Failed!!!", &err)
	db = &dataBusMqtt{}
	return
}

func (dbMqtt *dataBusMqtt) createContext(contextConfig map[string]string) (err error) {
	defer errHandler("MQTT Context Creation Failed!!!", &err)
	dbMqtt.direction = contextConfig["direction"]
	if dbMqtt.direction == "PUB" || dbMqtt.direction == "SUB" {
		brokerUrl := "tcp://" + strings.Split(contextConfig["endpoint"], "//")[1]
		//TODO: set keepalive too?
		//opts := mqtt.NewClientOptions().AddBroker(brokerUrl).SetClientID(contextConfig["name"])
		opts := mqtt.NewClientOptions().AddBroker(brokerUrl)
		dbMqtt.client = mqtt.NewClient(opts)
		if token := dbMqtt.client.Connect(); token.Wait() && token.Error() != nil {
			panic(token.Error())
		}
	}
	return
}

func (dbMqtt *dataBusMqtt) startTopic(topicConfig map[string]string) (err error) {
	defer errHandler("MQTT Topic Start Failed!!!", &err)
	if dbMqtt.direction == "PUB" {
	}
	if dbMqtt.direction == "SUB" {
	}
	return
}

func (dbMqtt *dataBusMqtt) send(topic map[string]string, msgData interface{}) (err error) {
	defer errHandler("MQTT Send Failed!!!", &err)
	if topic["type"] == "string" {
		if token := dbMqtt.client.Publish(topic["name"], 1, false, msgData.(string)); token.Wait() && token.Error() != nil {
			panic(token.Error())
		}
	}
	return
}

func (dbMqtt *dataBusMqtt) receive(topic map[string]string, trig string, ch chan interface{}) (err error) {
	defer errHandler("MQTT Receive Failed!!!", &err)
	if dbMqtt.direction == "SUB" && trig == "START" && ch != nil {
		var f mqtt.MessageHandler = func(client mqtt.Client, msg mqtt.Message) {
			//TODO: Type check
			glog.Infoln("Receiving on topic: " + msg.Topic())
			if topic["type"] == "string" {
				ch <- string(msg.Payload())
			}
		}
		if token := dbMqtt.client.Subscribe(topic["name"], 1, f); token.Wait() && token.Error() != nil {
			panic(token.Error())
		}
		glog.Infoln("Sub started for: " + topic["name"])
	} else if dbMqtt.direction == "SUB" && trig == "STOP" {
		if token := dbMqtt.client.Unsubscribe(topic["name"]); token.Wait() && token.Error() != nil {
			panic(token.Error())
		}
		glog.Infoln("Sub stopped for: " + topic["name"])
	} else {
		panic("Wrong parameters passed for receive()")
	}
	return
}

func (dbMqtt *dataBusMqtt) stopTopic(topic string) (err error) {
	defer errHandler("MQTT Topic Stop Failed!!!", &err)
	return
}

func (dbMqtt *dataBusMqtt) destroyContext() (err error) {
	defer errHandler("MQTT Context Termination Failed!!!", &err)
	glog.Infoln("MQTT Client Disconnecting...")
	dbMqtt.client.Disconnect(100)
	glog.Infoln("MQTT Client Disconnected")
	return
}
