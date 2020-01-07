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
	msgbusutil "IEdgeInsights/common/util/msgbusutil"
	"fmt"
	"os"
	"strconv"
	"time"
)

func start_client() {
	mode := os.Getenv("DEV_MODE")
	devMode, err := strconv.ParseBool(mode)
	serviceName := "GoPublisher"

	config1, err := readClientConfig(serviceName, devMode)
	fmt.Printf("-- Initializing message bus context for client %v", config1)

	client1, err := eismsgbus.NewMsgbusClient(config1)
	if err != nil {
		fmt.Printf("-- Error initializing message bus context: %v\n", err)
		return
	}
	defer client1.Close()

	fmt.Printf("-- Initializing service requester %s\n", serviceName)
	serviceRequester, err := client1.GetService(serviceName)
	if err != nil {
		fmt.Printf("-- Error initializing service requester: %v\n", err)
		return
	}
	defer serviceRequester.Close()
	fmt.Println("-- Running Client...")

	req := map[string]interface{}{"command": "C"}
	count := 1
	for {
		var cmd interface{} = req["command"].(string) + strconv.Itoa(count)
		req["command"] = cmd

		if count == 10 {
			count = 0
			req["command"] = "C"
		} else {
			count++
		}

		err := serviceRequester.Request(req)
		if err != nil {
			fmt.Printf("-- Error sending request: %v\n", err)
			return
		}

		//fmt.Printf("-- Sent Request %v", req)
		res, err := serviceRequester.ReceiveResponse(-1)
		if err != nil {
			fmt.Printf("-- Error receiving response: %v\n", err)
			return
		}

		fmt.Printf("---Client sent Request %v and received response %v\n", req, res)
		time.Sleep(1 * time.Second)
	}
}

func readClientConfig(serviceName string, devMode bool) (map[string]interface{}, error) {
	appName := os.Getenv("AppName")
	cfgMgrConfig := util.GetCryptoMap(appName)
	return msgbusutil.GetMessageBusConfig(serviceName, "client", devMode, cfgMgrConfig), nil
}
