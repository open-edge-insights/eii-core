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
	"flag"
	"fmt"
	"os"
	"time"
)
import "C"

func cbFunc(key string, value map[string]interface{}, user_data interface{}) {
	fmt.Printf("Callback triggered for key %s, value obtained %v with user_data %v\n", key, value, user_data)
}

func main() {
	flag.Parse()

	os.Setenv("AppName", "VideoIngestion")

	configMgr, _ := eiscfgmgr.ConfigManager()

	appConfig, err := configMgr.GetAppConfig()

	// Fetching Watch object
	watchObj, err := configMgr.GetWatchObj()
	if err != nil {
		fmt.Println("Failed to fetch watch object")
	}

	// Testing Watch API
	var watchUserData interface{} = "watch"
	watchObj.Watch("/VideoIngestion/config", cbFunc, watchUserData)

	// Testing WatchPrefix API
	var watchPrefixUserData interface{} = 1234
	watchObj.WatchPrefix("/VideoAnalytics", cbFunc, watchPrefixUserData)

	// Testing WatchConfig API
	var watchConfigUserData interface{} = 45.67
	watchObj.WatchConfig(cbFunc, watchConfigUserData)

	// Testing WatchInterface API
	var watchInterfaceUserData interface{} = true
	watchObj.WatchInterface(cbFunc, watchInterfaceUserData)

	defer configMgr.Destroy()

	if err != nil {
		fmt.Println("Error found:", err)
	} else {
		fmt.Println("App_config:", appConfig)
	}
	fmt.Println("Watching on app config & app interface for 60 seconds")
	time.Sleep(60 * time.Second)
}
