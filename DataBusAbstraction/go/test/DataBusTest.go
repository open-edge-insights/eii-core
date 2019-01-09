/*
Copyright (c) 2018 Intel Corporation.

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

package main

import (
	databus "ElephantTrunkArch/DataBusAbstraction/go"
	"flag"
	"fmt"
	"os"
	"time"

	"github.com/golang/glog"
)

/*
endpoint:
<examples>
	OPCUA -> opcua://localhost:4840
	MQTT -> mqtt://localhost:1883
	NATS -> nats://127.0.0.1:4222
*/

func errHandler() {
	if r := recover(); r != nil {
		glog.Errorln(r)
		glog.Errorln("Exting Test program with ERROR!!!")
		os.Exit(1)
	}
}

func cbFunc(topic string, msg interface{}) {
	glog.Errorln("Received msg: " + msg.(string) + " on topic: " + topic)
}

func main() {

	endPoint := flag.String("endpoint", "", "Provide the message bus details")
	direction := flag.String("direction", "", "One of PUB/SUB")
	certFile := flag.String("certFile", "", "provide server or client certificate file path as value")
	privateFile := flag.String("privateFile", "", "provide server or client private key file pathas value")
	trustFile := flag.String("trustFile", "", "provide ca cert file path as value")

	ns := flag.String("ns", "", "namespace")
	topic := flag.String("topic", "", "topic name")

	flag.Parse()

	flag.Lookup("logtostderr").Value.Set("true")
	flag.Lookup("log_dir").Value.Set("/var/log")

	defer glog.Flush()
	//now start the tests
	contextConfig := map[string]string{
		"endpoint":    *endPoint,
		"direction":   *direction,
		"name":        *ns,
		"certFile":    *certFile,
		"privateFile": *privateFile,
		"trustFile":   *trustFile,
	}

	defer errHandler()
	etaDatab, err := databus.NewDataBus()
	if err != nil {
		panic(err)
	}

	err = etaDatab.ContextCreate(contextConfig)
	if err != nil {
		panic(err)
	}

	glog.Errorln("Direction..", *direction)
	if *direction == "PUB" {
		topicConfig := map[string]string{
			"name": *topic,
			"type": "string",
		}

		for i := 0; i < 20; i++ {
			time.Sleep(5 * time.Second)
			result := fmt.Sprintf("%s %d", "Hello ", i)
			etaDatab.Publish(topicConfig, result)
			glog.Errorf("Published result: %s\n", result)
		}
	} else if *direction == "SUB" {
		panic("SUB APIs not integrated")
		// topicConfig := map[string]string{
		// 	"name": *topic,
		// 	"type": "string",
		// }
		// etaDatab.Subscribe(topicConfig, "START", cbFunc)
		// for true {
		// 	time.Sleep(10 * time.Second)
		// }
	}
	etaDatab.ContextDestroy()
}
