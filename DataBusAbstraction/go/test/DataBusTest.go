/*
Copyright (c) 2018 Intel Corporation.

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

package main

import (
	databus "IEdgeInsights/DataBusAbstraction/go"
	"flag"
	"fmt"
	"os"
	"strings"
	"time"

	"github.com/golang/glog"
)

/*
endpoint:
<examples>
	OPCUA -> opcua://localhost:4840
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

	endPoint := flag.String("endpoint", "", "Provide the message bus details Eg:opcua://localhost:4840")
	direction := flag.String("direction", "", "One of PUB/SUB")
	ns := flag.String("ns", "", "namespace")
	topics := flag.String("topics", "", "list of comma separated topics (Eg: topic1,topic2)")
	certFile := flag.String("certFile", "", "provide server or client certificate file in der format as value")
	privateFile := flag.String("privateFile", "", "provide server or client private key file in der format as value")
	trustFile := flag.String("trustFile", "", "provide ca cert file in der format as value")

	flag.Parse()

	flag.Lookup("logtostderr").Value.Set("true")
	flag.Lookup("log_dir").Value.Set("/var/log")

	defer glog.Flush()

	topicsArr := strings.Split(*topics, ",")
	topicConfigs := make([]map[string]string, len(topicsArr))
	for i, topic := range topicsArr {
		topicConfigs[i] = map[string]string{"namespace": *ns, "name": topic, "dType": "string"}
	}

	contextConfig := map[string]string{
		"endpoint":    *endPoint,
		"direction":   *direction,
		"certFile":    *certFile,
		"privateFile": *privateFile,
		"trustFile":   *trustFile,
	}

	defer errHandler()
	ieiDatab, err := databus.NewDataBus()
	if err != nil {
		panic(err)
	}

	err = ieiDatab.ContextCreate(contextConfig)
	if err != nil {
		panic(err)
	}

	if *direction == "PUB" {

		for i := 0; i < 100; i++ {
			for _, topicConfig := range topicConfigs {
				result := fmt.Sprintf("%s %d", topicConfig["name"], i)
				ieiDatab.Publish(topicConfig, result)
				glog.Infof("Published result: %s\n", result)
				time.Sleep(time.Second)
			}
		}
	} else if *direction == "SUB" {
		glog.Info("opcua subscriber is not implemented...")
		// ieiDatab.Subscribe(topicConfigs, len(topicConfigs) "START", cbFunc)
		// time.Sleep(5 * time.Second)

		// for i := 0; i < 200; i++ {
		// 	time.Sleep(time.Second)
		// }
	}
	ieiDatab.ContextDestroy()
}
