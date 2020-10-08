/*
Copyright (c) 2020 Intel Corporation.

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

package main

import (
	eiscfgmgr "ConfigMgr/eisconfigmgr"
	eismsgbus "EISMessageBus/eismsgbus"
	"fmt"
	"os"
	"time"
)

func main() {
	os.Setenv("AppName", "GoPublisher")
	os.Setenv("DEV_MODE", "True")

	os.Setenv("CONFIGMGR_CERT", "")
	os.Setenv("CONFIGMGR_KEY", "")
	os.Setenv("CONFIGMGR_CACERT", "")

	// os.Setenv("DEV_MODE", "False")
	// // Give the approptiate certificates path for prod mode
	// os.Setenv("CONFIGMGR_CERT", "")
	// os.Setenv("CONFIGMGR_KEY", "")
	// os.Setenv("CONFIGMGR_CACERT", "")

	configMgr, _ := eiscfgmgr.ConfigManager()

	devMode, _ := configMgr.IsDevMode()
	if devMode {
		fmt.Printf("Running in DEV mode\n")
	} else {
		fmt.Printf("Running in PROD mode\n")
	}

	appName, _ := configMgr.GetAppName()
	fmt.Printf("AppName : %v\n", appName)

	numOfPublishers, _ := configMgr.GetNumPublishers()
	fmt.Printf("Publishers : %v\n", numOfPublishers)

	numOfSubscribers, _ := configMgr.GetNumSubscribers()
	fmt.Printf("Subscribers : %v\n", numOfSubscribers)

	numOfServers, _ := configMgr.GetNumServers()
	fmt.Printf("Servers : %v\n", numOfServers)

	numOfClients, _ := configMgr.GetNumClients()
	fmt.Printf("Clients : %v\n", numOfClients)

	// pubCtx, _ := configMgr.GetPublisherByName("sample_pub")

	pubCtx, _ := configMgr.GetPublisherByIndex(0)

	endpoint := pubCtx.GetEndPoints()
	fmt.Println("endpoint:", endpoint)

	config, err := pubCtx.GetMsgbusConfig()
	fmt.Println("err in main:", err)
	if err != nil {
		fmt.Printf("Error occured with error:%v", err)
		return
	}

	fmt.Println("GetMsgbusConfig:", config)

	topics := pubCtx.GetTopics()
	fmt.Println("Publisher Topics")
	for _, s := range topics {
		fmt.Println(s)
	}

	allowed_clients := pubCtx.GetAllowedClients()
	fmt.Println("Publiser Allowed Clients")
	for _, s := range allowed_clients {
		fmt.Println(s)
	}

	topicsList := []string{"topic1", "topic2"}
	topicsSet := pubCtx.SetTopics(topicsList)
	if topicsSet == true {
		fmt.Println("Pub topics are set succesfully")
	} else {
		fmt.Println("Failed to set pub topics")
	}

	topics2 := pubCtx.GetTopics()
	fmt.Println("Publisher Topics....")
	for _, s := range topics2 {
		fmt.Println(s)
	}
	
	interfaceStrVal, err := pubCtx.GetInterfaceValue("Name")
	if(err != nil){
		fmt.Printf("Error to GetInterfaceValue: %v\n", err)
		return
	}

	fmt.Println("Interface str Value:", interfaceStrVal.Value)
	
	strVal, err := interfaceStrVal.GetString()
	if(err != nil) {
		fmt.Printf("Error to GetString value %v\n", err)
		return
	}

	stringVal := "Hello"
	stringCat := stringVal + strVal
	fmt.Println("str concatinated value", stringCat)

	interfaceIntVal, err := pubCtx.GetInterfaceValue("TestInt")
	if(err != nil) {
		fmt.Printf("Error to GetInterfaceValue %v\n", err)
		return
	}

	intVal, err := interfaceIntVal.GetInteger()
	if(err != nil) {
		fmt.Printf("Error to GetInteger value %v\n", err)
		return
	}
	addInt := intVal + 100
	fmt.Println("Interface int  Value:", addInt)

	interfaceFltVal, _ := pubCtx.GetInterfaceValue("Testfloat")
	if(err != nil) {
		fmt.Printf("Error to GetInterfaceValue %v\n", err)
		return
	}

	floatVal, err := interfaceFltVal.GetFloat()
	if(err != nil) {
		fmt.Printf("Error to GetFloat value:%v\n", err)
		return
	}

	addFloat := floatVal + 100
	fmt.Println("Interface float Value:", addFloat)

	interfaceBoolVal, err := pubCtx.GetInterfaceValue("Testbool")
	if(err != nil) {
		fmt.Printf("Error to GetInterfaceValue %v\n", err)
		return
	}

	boolVal , err := interfaceBoolVal.GetBool()
	if(err != nil) {
		fmt.Printf("Error to GetBool value %v\n", err)
		return
	}

	if(boolVal) {
		fmt.Println("Interface bool Value....:", boolVal)
	}

	interfaceObjVal, err := pubCtx.GetInterfaceValue("Testobj")
	if(err != nil) {
		fmt.Printf("Error to GetInterfaceValue %v\n", err)
		return
	}

	fmt.Println("Interface obj Value:", interfaceObjVal.Value)

	jsonVal, err := interfaceObjVal.GetJson()
	
	fmt.Println("JSON val", jsonVal["Testobj1"])
	if(err != nil) {
		fmt.Printf("Error to GetBool value %v\n", err)
		return
	}

	interfaceArrVal, err := pubCtx.GetInterfaceValue("TestArray")
	if(err != nil) {
		fmt.Printf("Error to GetInterfaceValue %v\n", err)
		return
	}
	
	fmt.Println("IInterface array Value:", interfaceArrVal.Value)
	
	fmt.Println("Array elements.....")
	arr, err := interfaceArrVal.GetArray()
	if(err != nil) {
		fmt.Printf("Error to GetBool value %v\n", err)
		return
	}
	for _, s := range arr {
		fmt.Println(s)
	}

	client, err := eismsgbus.NewMsgbusClient(config)
	if err != nil {
		fmt.Printf("-- Error initializing message bus context: %v\n", err)
		return
	}

	defer client.Close()

	fmt.Printf("-- Creating publisher for topic")
	publisher, err := client.NewPublisher(topics[0])
	if err != nil {
		fmt.Printf("-- Error creating publisher: %v\n", err)
		return
	}
	defer publisher.Close()

	fmt.Println("-- Running...")
	msg := map[string]interface{}{
		"str":   "hello",
		"int":   2.0,
		"float": 55.5,
		"bool":  true,
		"obj": map[string]interface{}{
			"nest": map[string]interface{}{
				"test": "hello",
			},
			"hello": "world",
		},
		"arr":   []interface{}{"test", 123.0},
		"empty": nil,
	}

	for {
		err = publisher.Publish(msg)
		if err != nil {
			fmt.Printf("-- Failed to publish message: %v\n", err)
			return
		}
		time.Sleep(1 * time.Second)
	}

}
