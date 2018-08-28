/*
Copyright (c) 2018 Intel Corporation.

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

package main

import (
	"bufio"
	"flag"
	"fmt"
	databus "ElephantTrunkArch/DataBusAbstraction/go"
	"os"
)

/*
endpoint:
<examples>
	OPCUA -> opcua://0.0.0.0:65003/elephanttrunk/
	MQTT -> mqtt://localhost:1883/
	NATS -> nats://127.0.0.1:4222/
*/

func cbFunc(topic string, msg interface{}) {
	fmt.Println("Received msg: " + msg.(string) + " on topic: " + topic)
}

//Custom flag var
type topicType []string

func (t *topicType) String() string {
	return fmt.Sprintf("%s", *t)
}

func (t *topicType) Set(value string) error {
	fmt.Printf("%s\n", value)
	*t = append(*t, value)
	return nil
}

func errHandler() {
	if r := recover(); r != nil {
		fmt.Println(r)
		fmt.Println("Exting Test program with ERROR!!!")
		os.Exit(1)
	}
}

func main() {
	endPoint := flag.String("endpoint", "", "Provide the message bus details")
	direction := flag.String("direction", "", "One of PUB/SUB")
	ns := flag.String("ns", "", "namespace")
	var topics topicType
	flag.Var(&topics, "topic", "List of topics")
	msg := flag.String("msg", "", "Message to send")

	flag.Parse()
	//for glog
	flag.Lookup("logtostderr").Value.Set("true")
	flag.Lookup("log_dir").Value.Set("/var/log")
	//now start the tests
	contextConfig := map[string]string{
		"endpoint":  *endPoint,
		"direction": *direction,
		"name":      *ns,
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
	flag := "NONE"
	if *direction == "PUB" {
		for _, t := range topics {
			topicConfig := map[string]string{
				"name": t,
				"type": "string",
			}
			etaDatab.Publish(topicConfig, *msg)
		}
	} else if *direction == "SUB" {
		for _, t := range topics {
			topicConfig := map[string]string{
				"name": t,
				"type": "string",
			}
			etaDatab.Subscribe(topicConfig, "START", cbFunc)
		}
		flag = "START"
	} else {
		panic("Wrong direction argument")
	}
	scanner := bufio.NewScanner(os.Stdin)
	fmt.Print("Enter CMDs['START', 'STOP', 'TERM']/MSG: ")
	for scanner.Scan() {
		text := scanner.Text()
		if *direction == "PUB" {
			if text == "TERM" {
				etaDatab.ContextDestroy()
				break
			} else if text == "START" || text == "STOP" {
				fmt.Println("Not a valid CMD in PUB context!")
			} else {
				for _, t := range topics {
					topicConfig := map[string]string{
						"name": t,
						"type": "string",
					}
					etaDatab.Publish(topicConfig, text)
				}
			}
		} else if *direction == "SUB" {
			if text == "TERM" {
				etaDatab.ContextDestroy()
				break
			} else if text == "STOP" && flag == "START" {
				for _, t := range topics {
					topicConfig := map[string]string{
						"name": t,
						"type": "string",
					}
					etaDatab.Subscribe(topicConfig, "STOP", nil)
				}
				flag = "STOP"
			} else if text == "START" && flag == "STOP" {
				for _, t := range topics {
					topicConfig := map[string]string{
						"name": t,
						"type": "string",
					}
					etaDatab.Subscribe(topicConfig, "START", cbFunc)
				}
				flag = "START"
			}
		}
		fmt.Print("Enter CMDs['START', 'STOP', 'TERM']/MSG: ")
	}
}
