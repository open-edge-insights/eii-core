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
	os.Setenv("AppName", "VideoIngestion")

	configMgr, err := eiscfgmgr.ConfigManager()
	if err != nil {
		fmt.Printf("Error occured with error:%v", err)
		return
	}
	fmt.Printf("client obj: %v", configMgr)

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

	numOfServers, _ := configMgr.GetNumServers()
	fmt.Printf("Servers : %v\n", numOfServers)

	serverCtx, err := configMgr.GetServerByName("default")
	// serverCtx, err := configMgr.GetServerByIndex(0)
	if err != nil {
		fmt.Printf("Error occured with error:%v", err)
		return
	}

	endpoint, err:= serverCtx.GetEndPoints()
	if err != nil {
		fmt.Printf("Error occured with error:%v", err)
		return
	}
	fmt.Println("endpoint:", endpoint)

	interfaceVal, err := serverCtx.GetInterfaceValue("Name")
	if err != nil {
		fmt.Printf("Error to GetInterfaceValue: %v\n", err)
		return
	}

	fmt.Println("Interface Value:", interfaceVal.Value)
	serviceName, err := interfaceVal.GetString()
	if err != nil {
		fmt.Printf("Error to GetString value %v\n", err)
		return
	}

	config, err := serverCtx.GetMsgbusConfig()

	if err != nil {
		fmt.Printf("Error occured with error:%v", err)
		return
	}

	fmt.Println("config:", config["echo_service"])

	fmt.Println("Allowed clients for server")
	allowedClients, err:= serverCtx.GetAllowedClients()
	if err != nil {
		fmt.Printf("Error occured with error:%v", err)
		return
	}
	for _, s := range allowedClients {
		fmt.Println(s)
	}

	fmt.Println("-- Initializing message bus context")
	client, err := eismsgbus.NewMsgbusClient(config)
	if err != nil {
		fmt.Printf("-- Error initializing message bus context: %v\n", err)
		return
	}
	defer client.Close()

	fmt.Printf("-- Initializing service %s\n", serviceName)
	service, err := client.NewService(serviceName)
	if err != nil {
		fmt.Printf("-- Error initializing service: %v\n", err)
		return
	}
	defer service.Close()

	fmt.Println("-- Running...")
	for {
		msg, err := service.ReceiveRequest(-1)
		if err != nil {
			fmt.Printf("-- Error receiving request: %v\n", err)
			return
		}
		fmt.Printf("-- Received request: %v\n", msg)
		service.Response(msg.Data)
	}

}
