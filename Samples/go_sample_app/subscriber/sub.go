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
	eismsgbus "EISMessageBus/eismsgbus"
	util "IEdgeInsights/common/util"
	envconfig "EnvConfig"
	"fmt"
	"os"
	"strconv"
	"strings"
	"time"
)

func start_subscriber(topic string) {
	mode := os.Getenv("DEV_MODE")
	devMode, err := strconv.ParseBool(mode)

	config, err := readSubConfig(topic, devMode)
	fmt.Printf("-- Initializing message bus context for sub %v\n", config)
	client, err := eismsgbus.NewMsgbusClient(config)
	if err != nil {
		fmt.Printf("-- Error initializing message bus context: %v\n", err)
		return
	}
	defer client.Close()

	subTopics := strings.Split(topic, "/")
	fmt.Printf("-- Subscribing to topic %s\n", subTopics[1])
	subscriber, err := client.NewSubscriber(subTopics[1])
	if err != nil {
		fmt.Printf("-- Error subscribing to topic: %v\n", err)
		return
	}
	defer subscriber.Close()

	fmt.Println("-- Running Subscriber...")

	for {
		select {
		case msg := <-subscriber.MessageChannel:
			fmt.Printf("-- Received Message from publisher: %v \n", msg)
		case err := <-subscriber.ErrorChannel:
			fmt.Printf("-- Error receiving message: %v\n", err)
		}

		time.Sleep(1 * time.Second)
	}
}

func readSubConfig(topicName string, devMode bool) (map[string]interface{}, error) {
	appName := os.Getenv("AppName")
	cfgMgrConfig := util.GetCryptoMap(appName)
	return envconfig.GetMessageBusConfig(topicName, "sub", devMode, cfgMgrConfig), nil
}
