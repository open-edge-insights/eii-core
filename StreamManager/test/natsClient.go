/*
Copyright (c) 2018 Intel Corporation.

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

package main

import (
	"flag"
	"runtime"

	stm "iapoc_elephanttrunkarch/StreamManager"

	"github.com/golang/glog"
	nats "github.com/nats-io/go-nats"
)

func printMsg(m *nats.Msg, i int) {
	glog.Infof("[#%d] Received on [%s]: '%s'\n", i, m.Subject, string(m.Data))
}

func main() {
	var topic string
	flag.StringVar(&topic, "topic", "", "topic name interested to listen on")

	flag.Parse()

	flag.Lookup("alsologtostderr").Value.Set("true")

	nc, err := nats.Connect(stm.NatsServerURL)
	if err != nil {
		glog.Fatalf("Failed to connect to nats server: %s, error: %v", err)
	}

	i := 0
	nc.Subscribe(topic, func(msg *nats.Msg) {
		i++
		printMsg(msg, i)
	})
	nc.Flush()

	if err := nc.LastError(); err != nil {
		glog.Errorf("nats LastError: %v", err)
	}

	glog.Infof("Listening on [%s]\n", topic)
	runtime.Goexit()
}
