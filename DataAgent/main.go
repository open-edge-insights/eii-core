package main

import (
	"flag"
	"fmt"
	"github.com/golang/glog"
	stm "iapoc_elephanttrunkarch/StreamManager"
)

func main() {
	var err error
	err = nil
	flag.Parse()
	flag.Lookup("logtostderr").Value.Set("true")
	flag.Lookup("log_dir").Value.Set("/home/das/go/log_dir/")

	glog.Infof("**************STARTING DA & STREAM MANAGER**************")
	var pStreamManager *(stm.StrmMgr) = new(stm.StrmMgr)

	pStreamManager.ServerAdress = "localhost"
	pStreamManager.ServerPort = "61971"
	pStreamManager.InfluxAddr = "localhost"
	pStreamManager.InfluxPort = "8086"
	pStreamManager.Database = "DA_INTERNAL_DB"
	pStreamManager.Topic_map = make(map[string]string)
	pStreamManager.MeasurementPolicy = make(map[string]bool)

	//TODO: Remeber to add error to log file in serv_init()
	glog.Infof("Going to start UDP server for influx subscription")
	pStreamManager.StreamManagerInit()
	// TODO: Obtain Influx credential from DA
	// Hardcoding for now to "no-auth"

	var config *stm.OutStreamConfig = new(stm.OutStreamConfig)

	config.Measurement = "test_measure"
	config.Topic = "test_topic"

	err = pStreamManager.SetupOutStream(config)
	if err != nil {
		fmt.Println(err)
	}

	//TODO for a gracefull shutdown we need a trigger from DA to tear down
	// That will make the ever listening UDP server to stop and return control
	// to main program. A channel nned to listen to a exit cmd in UDP server code.

	//Currently running this infinite loop to keep the goroutine running
	for {
	}
}
