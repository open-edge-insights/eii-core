package main

import (
	"flag"
	"os"

	server "iapoc_elephanttrunkarch/DataAgent/da_grpc/server"
	stm "iapoc_elephanttrunkarch/StreamManager"
	util "iapoc_elephanttrunkarch/Util"

	"github.com/golang/glog"
)

const (
	strmMgrUDPServHost = "localhost"
	strmMgrUDPServPort = "61971"
)

func main() {

	var cfgPath string
	flag.StringVar(&cfgPath, "config", "", "config file path")

	flag.Parse()

	flag.Lookup("alsologtostderr").Value.Set("true")

	defer glog.Flush()

	if len(os.Args) < 2 {
		glog.Errorf("Usage: go run DataAgent/DataAgent.go " +
			"-config=<config_file_path> [-log_dir=<glog_dir_path>]")
		os.Exit(-1)
	}

	glog.Infof("**************STARTING DA**************")

	glog.Infof("Parsing the config file: %s", cfgPath)

	// Parse the DA config file
	err := server.DaCfg.ParseConfig(cfgPath)

	if err != nil {
		glog.Errorf("Error: %s while parsing config file: %s", err, cfgPath)
		os.Exit(-1)
	}

	influxCfg := server.DaCfg.InfluxDB

	// Create InfluxDB database
	glog.Infof("Creating InfluxDB database: %s", influxCfg.DBName)
	client, err := util.CreateHTTPClient(influxCfg.Host,
		influxCfg.Port, influxCfg.UserName, influxCfg.Password)

	if err != nil {
		glog.Errorf("Error creating InfluxDB client: %v", err)
	}

	defer client.Close()

	response, err := util.CreateDatabase(client, influxCfg.DBName)

	if err == nil && response.Error() == nil {
		glog.Infof("Successfully created database: %s", influxCfg.DBName)
	} else {
		glog.Errorf("err: %v and response.Error(): %v while creating "+
			"database: %s", err, response.Error(), influxCfg.DBName)

		os.Exit(-1)
	}

	// Init StreamManager
	glog.Infof("**************STARTING STREAM MANAGER**************")

	var pStreamManager = new(stm.StrmMgr)

	pStreamManager.ServerHost = strmMgrUDPServHost
	pStreamManager.ServerPort = strmMgrUDPServPort
	pStreamManager.InfluxDBHost = server.DaCfg.InfluxDB.Host
	pStreamManager.InfluxDBPort = server.DaCfg.InfluxDB.Port
	pStreamManager.InfluxDBName = server.DaCfg.InfluxDB.DBName
	pStreamManager.MsrmtTopicMap = make(map[string]string)
	pStreamManager.MeasurementPolicy = make(map[string]bool)

	glog.Infof("Going to start UDP server for influx subscription")
	pStreamManager.Init()

	var config = new(stm.OutStreamConfig)

	// Fetch the streams from the DA config file
	for key, val := range server.DaCfg.OutStreams {
		// TODO: Just using the 'key' as both measurement and topic for now.
		// This will change later
		config.Measurement = key
		config.Topic = key
		config.MsgBusType = val.DatabusFormat

		err = pStreamManager.SetupOutStream(config)
		if err != nil {
			glog.Errorf("Stream Manager, Error while setting up out stream: ", err)
		}
	}

	glog.Infof("**************STARTING GRPC SERVER**************")
	// Start GRPC server for GetConfig (Internal), GetConfig and GetQuery
	// external interfaces
	server.StartGrpcServer()

	// TODO: For a gracefull shutdown we need a trigger from DA to tear down
	// That will make the ever listening UDP server to stop and return control
	// to main program. A channel nned to listen to a exit cmd in UDP server
	// code.Currently running this infinite loop to keep the goroutine running
	// for StreamManager
	for {
	}

}
