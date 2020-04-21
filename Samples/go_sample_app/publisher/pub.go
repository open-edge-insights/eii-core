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
	"flag"
	"fmt"
	"os"
	"strconv"
	"time"
        "encoding/json"
        configmgr "ConfigManager"
)

func start_publisher(topic string) {

	mode := os.Getenv("DEV_MODE")
	devMode, err := strconv.ParseBool(mode)
	flag.Parse()

	config, err := readPubConfig(topic, devMode)
	fmt.Printf("-- Initializing message bus context %v\n", config)
	client, err := eismsgbus.NewMsgbusClient(config)
	if err != nil {
		fmt.Printf("-- Error initializing message bus context: %v\n", err)
		return
	}
	defer client.Close()

	fmt.Printf("-- Creating publisher for topic %s\n", topic)
	publisher, err := client.NewPublisher(topic)
	if err != nil {
		fmt.Printf("-- Error creating publisher: %v\n", err)
		return
	}
	defer publisher.Close()

	fmt.Println("-- Running publisher...")

        sMsg := map[string]interface{}{
                "str":   "hello",
                "count":   0,
        }

        count := 0

        data, err := get_app_config()
        loop_interval, _ := time.ParseDuration(data["loop_interval"])

	for {
                sMsg["count"] = count

		err = publisher.Publish(sMsg)

		if err != nil {
			fmt.Printf("-- Failed to publish message: %v \n", err)
		}else{
		        fmt.Printf("-- Published message : %v \n", sMsg)
                }

                count++
		time.Sleep(loop_interval)
	}
}

func readPubConfig(topicName string, devMode bool) (map[string]interface{}, error) {
	appName := os.Getenv("AppName")
	cfgMgrConfig := util.GetCryptoMap(appName)
	return envconfig.GetMessageBusConfig(topicName, "pub", devMode, cfgMgrConfig), nil
}

func get_app_config()(map[string]string, error){
        data := make(map[string]string)
	appName := os.Getenv("AppName")
	config := util.GetCryptoMap(appName)
        mgr := configmgr.Init("etcd", config)
	if mgr == nil {
		glog.Fatal("Config Manager initialization failed...")
	}

        value, err := mgr.GetConfig("/" + appName + "/config")
        if err != nil {
                fmt.Printf("Not able to read value from etcd for /%v/config", appName)
                return nil, err
        }

        err = json.Unmarshal([]byte(value), &data)
        if err != nil {
                fmt.Printf("json error: %s", err.Error())
                return nil, err
        }
        return data,nil
}


