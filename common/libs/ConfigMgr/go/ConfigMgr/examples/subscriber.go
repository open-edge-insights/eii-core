/*
Copyright (c) 2021 Intel Corporation

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

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
	eiicfgmgr "ConfigMgr/eiiconfigmgr"
	eiimsgbus "EIIMessageBus/eiimsgbus"
	"fmt"
	"os"
)

func main() {
	os.Setenv("AppName", "VideoAnalytics")

	// Initialize ConfigManager
	configMgr, err := eiicfgmgr.ConfigManager()
	if err != nil {
		fmt.Printf("Error occured with error:%v", err)
		return
	}

	// Delete ConfigManager context
	defer configMgr.Destroy()

	// Get applictaion's AppName
	appName, err := configMgr.GetAppName()
	if err != nil {
		fmt.Printf("Error occured with error:%v", err)
		return
	}
	fmt.Printf("AppName : %v\n", appName)

	// Get number of subscribers in the Publisher interface
	numOfSubscribers, _ := configMgr.GetNumSubscribers()
	fmt.Printf("Subscribers : %v\n", numOfSubscribers)

	// Get the subscriber object where subscriber's interface 'Name' is 'default'
	// subCtx, err := configMgr.GetSubscriberByName("default")

	// Get 0th publisher interface object
	subCtx, err := configMgr.GetSubscriberByIndex(0)
	if err != nil {
		fmt.Printf("Error occured with error:%v", err)
		return
	}

	// Delete ConfigManager's subscriber context
	defer subCtx.Destroy()

	// Get Endpoint of a publisher interface
	endpoint, err := subCtx.GetEndPoints()
	if err != nil {
		fmt.Printf("Error occured with error:%v", err)
		return
	}
	fmt.Printf("Subscriber endpoint:%s", endpoint)

	// get subscriber msgbus config for application to communicate over EII message bus
	config, err := subCtx.GetMsgbusConfig()

	if err != nil {
		fmt.Printf("Error occured with error:%v", err)
		return
	}

	fmt.Printf("GetMsgbusConfig:%v", config)

	// Get 'Topics' from subscriber interface
	topics, err := subCtx.GetTopics()
	if err != nil {
		fmt.Printf("Error occured with error:%v", err)
		return
	}
	fmt.Println("Subscriber Topics:", topics)
	for _, s := range topics {
		fmt.Println(s)
	}

	topicsList := []string{"subtopic1", "subtopic2", "subtopic3"}
	// Update new set of topic for subscriber's interface
	topicsSet := subCtx.SetTopics(topicsList)
	if topicsSet == true {
		fmt.Println("Sub topics are set succesfully")
	} else {
		fmt.Println("Failed to set sub topics")
	}

	// Get updated topics, modified by SetTopics() API
	topics2, err := subCtx.GetTopics()
	if err != nil {
		fmt.Printf("Error occured with error:%v", err)
		return
	}
	fmt.Println("Publisher New Topics....")
	for _, s := range topics2 {
		fmt.Println(s)
	}

	// Get the object to get the value of server interface of key 'Name'
	interfaceStrVal, err := subCtx.GetInterfaceValue("Name")
	if err != nil {
		fmt.Printf("Error to GetInterfaceValue: %v\n", err)
		return
	}

	// Get the value from object interfaceVal
	fmt.Println("Interface Value:", interfaceStrVal.Value)

	// Initialize msgbus context by passing msgbus config
	client, err := eiimsgbus.NewMsgbusClient(config)
	if err != nil {
		fmt.Printf("-- Error initializing message bus context: %v\n", err)
		return
	}
	defer client.Close()

	fmt.Printf("-- Subscribing to topic %s\n", topics[0])
	subscriber, err := client.NewSubscriber(topics[0])
	if err != nil {
		fmt.Printf("-- Error subscribing to topic: %v\n", err)
		return
	}
	defer subscriber.Close()

	fmt.Println("-- Running...")
	for {
		select {
		case msg := <-subscriber.MessageChannel:
			fmt.Printf("-- Received Message: %v on topic: %s\n", msg.Data, msg.Name)
		case err := <-subscriber.ErrorChannel:
			fmt.Printf("-- Error receiving message: %v\n", err)
		}
	}
}
