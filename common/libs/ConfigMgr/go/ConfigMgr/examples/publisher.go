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
	os.Setenv("AppName", "VideoIngestion")

	configMgr, err := eiscfgmgr.ConfigManager()
	if err != nil {
		fmt.Printf("Error occured with error:%v", err)
		return
	}

	defer configMgr.Destroy()

	devMode, _ := configMgr.IsDevMode()
	if devMode {
		fmt.Printf("Running in DEV mode\n")
	} else {
		fmt.Printf("Running in PROD mode\n")
	}

	appName, err := configMgr.GetAppName()
	if err != nil {
		fmt.Printf("Error occured with error:%v", err)
		return
	}
	fmt.Printf("AppName : %v\n", appName)

	numOfPublishers, _ := configMgr.GetNumPublishers()
	fmt.Printf("Publishers : %v\n", numOfPublishers)

	pubCtx, err := configMgr.GetPublisherByName("default")
	// pubCtx, err := configMgr.GetPublisherByIndex(0)
	if err != nil {
		fmt.Printf("Error occured with error:%v", err)
		return
	}

	defer pubCtx.Destroy()

	endpoint, err:= pubCtx.GetEndPoints()
	if err != nil {
		fmt.Printf("Error occured with error:%v", err)
		return
	}
	fmt.Println("endpoint:", endpoint)

	config, err := pubCtx.GetMsgbusConfig()
	fmt.Println("err in main:", err)
	if err != nil {
		fmt.Printf("Error occured with error:%v", err)
		return
	}

	fmt.Println("GetMsgbusConfig:", config)

	topics, err := pubCtx.GetTopics()
	if err != nil {
		fmt.Printf("Error occured with error:%v", err)
		return
	}

	fmt.Println("Publisher Topics")
	for _, s := range topics {
		fmt.Println(s)
	}

	allowedClients, err := pubCtx.GetAllowedClients()
	if err != nil {
		fmt.Printf("Error occured with error:%v", err)
		return
	}
	fmt.Println("Publiser Allowed Clients")
	for _, s := range allowedClients {
		fmt.Println(s)
	}

	topicsList := []string{"topic1", "topic2"}
	topicsSet := pubCtx.SetTopics(topicsList)
	if topicsSet == true {
		fmt.Println("Pub topics are set succesfully")
	} else {
		fmt.Println("Failed to set pub topics")
	}

	topics2, err:= pubCtx.GetTopics()
	if err != nil {
		fmt.Printf("Error occured with error:%v", err)
		return
	}
	fmt.Println("Publisher Topics....")
	for _, s := range topics2 {
		fmt.Println(s)
	}

	interfaceStrVal, err := pubCtx.GetInterfaceValue("Name")
	if err != nil {
		fmt.Printf("Error to GetInterfaceValue: %v\n", err)
		return
	}

	fmt.Println("Interface str Value:", interfaceStrVal.Value)

	strVal, err := interfaceStrVal.GetString()
	if err != nil {
		fmt.Printf("Error to GetString value %v\n", err)
		return
	}

	stringVal := "Hello"
	stringCat := stringVal + strVal
	fmt.Println("str concatinated value", stringCat)

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

	for i := 1 ; i <= 10; i++ {
		err = publisher.Publish(msg)
		if err != nil {
			fmt.Printf("-- Failed to publish message: %v\n", err)
			return
		}
		time.Sleep(1 * time.Second)
	}
}
