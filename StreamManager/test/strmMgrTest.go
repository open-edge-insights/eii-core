/*
Copyright (c) 2018 Intel Corporation.

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

Explicit permissions are required to publish, distribute, sublicense, and/or sell copies of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. 
*/

package main

import (
	"flag"

	stm "IEdgeInsights/StreamManager"

	"github.com/golang/glog"
)

func main() {
	var err error
	err = nil

	flag.Parse()
	flag.Lookup("alsologtostderr").Value.Set("true")

	defer glog.Flush()

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

	// TODO: For a gracefull shutdown we need a trigger from DA to tear down
	// That will make the ever listening UDP server to stop and return control
	// to main program. A channel nned to listen to a exit cmd in UDP server
	// code.

	//Currently running this infinite loop to keep the goroutine running
	for {
	}
}
