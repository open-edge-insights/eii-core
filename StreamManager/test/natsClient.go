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
