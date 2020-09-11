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
	"strconv"
)

func main() {
	os.Setenv("AppName", "VideoAnalytics")
	os.Setenv("DEV_MODE", "True")

	serviceName := "echo_service"
	configMgr, _ := eiscfgmgr.ConfigManager()

	// clientCtx, _ := configMgr.GetClientByName("sample_client")

	clientCtx, _ := configMgr.GetClientByIndex(0)

	endpoint := clientCtx.GetEndPoints()
	fmt.Printf("Client endpoint:%s", endpoint)

	config, err := clientCtx.GetMsgbusConfig()

	if(err != nil) {
		fmt.Printf("Error occured with error:%v", err)
		return
	}

	fmt.Printf("GetMsgbusConfig:%v", config)

	fmt.Println("-- Initializing message bus context")
	client, err := eismsgbus.NewMsgbusClient(config)
	if err != nil {
		fmt.Printf("-- Error initializing message bus context: %v\n", err)
		return
	}
	defer client.Close()

	fmt.Printf("-- Initializing service requester %s\n", serviceName)
	service, err := client.GetService(serviceName)
	if err != nil {
		fmt.Printf("-- Error initializing service requester: %v\n", err)
		return
	}
	defer service.Close()

	fmt.Printf("-- Sending request")
	req := map[string]interface{}{"hello": "world"}
	count := 1

	for {
		var cmd interface{} = req["hello"].(string) + strconv.Itoa(count)
		req["hello"] = cmd

		if count == 10 {
			count = 0
			req["hello"] = "world"
		} else {
			count++
		}


		err = service.Request(req)
		if err != nil {
			fmt.Printf("-- Error sending request: %v\n", err)
			return
		}

		fmt.Printf("-- Waiting for response")
		resp, err := service.ReceiveResponse(-1)
		if err != nil {
			fmt.Printf("-- Error receiving response: %v\n", err)
			return
		}

		fmt.Printf("-- Received response: %v\n", resp)
		time.Sleep(1)
	}

}
