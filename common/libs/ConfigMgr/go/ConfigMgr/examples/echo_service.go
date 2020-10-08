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
	fmt.Printf("client obj: %v", configMgr)

	// serverCtx,_ := configMgr.GetServerByName("echo_service")
	serverCtx,_ := configMgr.GetServerByIndex(0)

	endpoint := serverCtx.GetEndPoints()
	fmt.Println("endpoint:", endpoint)

	interfaceVal, err := serverCtx.GetInterfaceValue("Name")
	if(err != nil){
		fmt.Printf("Error to GetInterfaceValue: %v\n", err)
		return
	}

	fmt.Println("Interface Value:", interfaceVal.Value)
	serviceName, err := interfaceVal.GetString()
	if(err != nil) {
		fmt.Printf("Error to GetString value %v\n", err)
		return
	}

	config, err := serverCtx.GetMsgbusConfig()

	if(err != nil) {
		fmt.Printf("Error occured with error:%v", err)
		return
	}

	fmt.Println("config:", config["echo_service"])

	fmt.Println("Allowed clients for server")
	allowed_clients := serverCtx.GetAllowedClients()
	for _, s := range allowed_clients {
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
