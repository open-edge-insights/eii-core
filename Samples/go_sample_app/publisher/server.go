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
)

func start_server() {
	mode := os.Getenv("DEV_MODE")
	devMode, err := strconv.ParseBool(mode)
	sConfig, err := readServerConfig(devMode)
	fmt.Printf("-- Initializing message bus context for server %v\n", sConfig)
	sClient, err := eismsgbus.NewMsgbusClient(sConfig)
	if err != nil {
		fmt.Printf("-- Error initializing message bus context: %v\n", err)
		return
	}
	defer sClient.Close()

	serviceName := os.Getenv("AppName")
	fmt.Printf("-- Initializing service %s\n", serviceName)
	service, err := sClient.NewService(serviceName)
	if err != nil {
		fmt.Printf("-- Error initializing service: %v\n", err)
	}
	defer service.Close()

	fmt.Println("-- Running server...")

	for {
		req, err := service.ReceiveRequest(-1)
		if err != nil {
			fmt.Printf("-- Error receiving request: %v\n", err)
		}
		fmt.Printf("--Server received request: %v\n", req)
		service.Response(map[string]interface{}{"Response": "OK"})
	}
}

func readServerConfig(devMode bool) (map[string]interface{}, error) {
	appName := os.Getenv("AppName")
	cfgMgrConfig := util.GetCryptoMap(appName)
	return envconfig.GetMessageBusConfig(appName, "server", devMode, cfgMgrConfig), nil
}
