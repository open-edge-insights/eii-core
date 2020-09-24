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
)

func main() {
	os.Setenv("AppName", "GoSubscriber")
	os.Setenv("DEV_MODE", "True")

	configMgr, _ := eiscfgmgr.ConfigManager()

	// subCtx, _ := configMgr.GetSubscriberByName("sample_sub")
	subCtx, _ := configMgr.GetSubscriberByIndex(0)

	endpoint := subCtx.GetEndPoints()
	fmt.Printf("Subscriber endpoint:%s", endpoint)

	config, err := subCtx.GetMsgbusConfig()

	if(err != nil) {
		fmt.Printf("Error occured with error:%v", err)
		return
	}

	fmt.Printf("GetMsgbusConfig:%v", config)

	topics := subCtx.GetTopics()
	fmt.Println("Subscriber Topics:", config)
	for _, s := range topics {
		fmt.Println(s)
	}

	topicsList := []string{"subtopic1", "subtopic2", "subtopic3"}
	topicsSet := subCtx.SetTopics(topicsList);
	if(topicsSet == true) {
		fmt.Println("Sub topics are set succesfully")
	} else {
		fmt.Println("Failed to set sub topics")
	}

	topics2 := subCtx.GetTopics()
	fmt.Println("Publisher New Topics....")
	for _, s := range topics2 {
		fmt.Println(s)
	}

	interfaceStrVal, err := subCtx.GetInterfaceValue("Name")
	if(err != nil){
		fmt.Printf("Error to GetInterfaceValue: %v\n", err)
		return
	}
	
	fmt.Println("Interface Value:", interfaceStrVal.Value)

	client, err := eismsgbus.NewMsgbusClient(config)
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
