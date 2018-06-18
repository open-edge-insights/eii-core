package main

import (
	"bufio"
	"flag"
	"fmt"
	databus "iapoc_elephanttrunkarch/DataBusAbstraction/go"
	"os"
)

func main() {
	mqtt_context := map[string]string{
		"direction": "PUB",
		"name":      "streammanager",
		"endpoint":  "mqtt://localhost:1883/",
	}
	opcua_context := map[string]string{
		"direction": "PUB",
		"name":      "streammanager",
		"endpoint":  "opcua://0.0.0.0:4840/elephanttrunk/",
	}
	topic_config1 := map[string]string{
		"name": "streammanager/first/time/test",
		"type": "string",
	}
	topic_config2 := map[string]string{
		"name": "streammanager/second/test2",
		"type": "string",
	}
	topic_config3 := map[string]string{
		"name": "streammanager/second/time/test3",
		"type": "string",
	}
	//for glog
	flag.Parse()
	flag.Lookup("logtostderr").Value.Set("true")
	flag.Lookup("log_dir").Value.Set("/var/log")
	//now start the tests
	mqttDatab := databus.NewDataBus()
	if 0 != mqttDatab.ContextCreate(mqtt_context) {
		panic("MQTT Context Creation failure")
	}
	opcuaDatab := databus.NewDataBus()
	if 0 != opcuaDatab.ContextCreate(opcua_context) {
		panic("OPCUA Context Creation failure")
	}
	itr := 1
	text := ""
	scanner := bufio.NewScanner(os.Stdin)
	fmt.Println("Enter Msg: ")
	for scanner.Scan() {
		text = scanner.Text()
		if text == "Terminate" {
			mqttDatab.ContextDestroy()
			opcuaDatab.ContextDestroy()
			break
		}
		switch itr {
		case 1:
			fmt.Println("Publish '" + text + "' to topic '" + topic_config1["name"] + "'")
			mqttDatab.Publish(topic_config1, text)
			opcuaDatab.Publish(topic_config1, text)
			itr = 2
		case 2:
			fmt.Println("Publish '" + text + "' to topic '" + topic_config2["name"] + "'")
			mqttDatab.Publish(topic_config2, text)
			opcuaDatab.Publish(topic_config2, text)
			itr = 3
		case 3:
			fmt.Println("Publish '" + text + "' to topic '" + topic_config3["name"] + "'")
			mqttDatab.Publish(topic_config3, text)
			opcuaDatab.Publish(topic_config3, text)
			itr = 1
		}
		fmt.Println("Enter Msg: ")
	}
}
