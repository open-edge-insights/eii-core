package main

import (
	"flag"

	stm "iapoc_elephanttrunkarch/StreamManager"

	"github.com/golang/glog"
)

func main() {
	var err error
	err = nil

	flag.Parse()
	flag.Lookup("alsologtostderr").Value.Set("true")

	glog.Infof("**************STARTING DA & STREAM MANAGER**************")

	var pStreamManager = new(stm.StrmMgr)

	pStreamManager.ServerHost = "localhost"
	pStreamManager.ServerPort = "61971"
	pStreamManager.InfluxDBHost = "localhost"
	pStreamManager.InfluxDBPort = "8086"
	pStreamManager.InfluxDBName = "DA_INTERNAL_DB"
	pStreamManager.MsrmtTopicMap = make(map[string]string)
	pStreamManager.MeasurementPolicy = make(map[string]bool)

	glog.Infof("Going to start UDP server for influx subscription")
	pStreamManager.Init()
	// TODO: Obtain Influx credential from DA
	// Hardcoding for now to "no-auth"

	var config = new(stm.OutStreamConfig)

	config.Measurement = "test_measure"
	config.Topic = "test_topic"
	config.MsgBusType = "OPCUA"

	err = pStreamManager.SetupOutStream(config)
	if err != nil {
		glog.Errorf("pStreamManager.SetupOutStream error: %s", err)
	}

	glog.Flush()

	// TODO: For a gracefull shutdown we need a trigger from DA to tear down
	// That will make the ever listening UDP server to stop and return control
	// to main program. A channel nned to listen to a exit cmd in UDP server
	// code.

	//Currently running this infinite loop to keep the goroutine running
	for {
	}
}
