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
	"time"
	"os"
)

func main() {
	os.Setenv("AppName", "VideoIngestion")
	os.Setenv("DEV_MODE", "True")

	configMgr, _ := eiscfgmgr.ConfigManager()

	// pubCtx,_ := configMgr.GetPublisherByName("sample_pub")

	pubCtx,_ := configMgr.GetPublisherByIndex(0)

	endpoint := pubCtx.GetEndPoints()
	fmt.Println("endpoint:", endpoint)

	config, err := pubCtx.GetMsgbusConfig()
	fmt.Println("err in main:", err)
	if(err != nil) {
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
	topicsSet := pubCtx.SetTopics(topicsList);
	if(topicsSet == true) {
		fmt.Println("Pub topics are set succesfully")
	} else {
		fmt.Println("Failed to set pub topics")
	}

	topics2 := pubCtx.GetTopics()
	fmt.Println("Publisher New Topics....")
	for _, s := range topics2 {
		fmt.Println(s)
	}

	client, err := eismsgbus.NewMsgbusClient(config)
	if err != nil {
		fmt.Printf("-- Error initializing message bus context: %v\n", err)
		return
	}

	defer client.Close()

	fmt.Printf("-- Creating publisher for topic")
	publisher, err := client.NewPublisher("publish_test")
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
