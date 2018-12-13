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
	"io/ioutil"
	"net"
	"os"
	"os/exec"
	"path/filepath"
	"time"

	config "ElephantTrunkArch/DataAgent/config"
	internalserver "ElephantTrunkArch/DataAgent/da_grpc/server/server_internal"
	stm "ElephantTrunkArch/StreamManager"
	util "ElephantTrunkArch/Util/influxdb"

	"github.com/golang/glog"
)

var strmMgrUDPServHost = "ia_data_agent"

const (
	strmMgrUDPServPort             = "61971"
	INFLUX_SERVER_CERTIFICATE_PATH = "/etc/ssl/influxdb/influxdb_server_certificate.pem"
	INFLUX_SERVER_KEY_PATH         = "/etc/ssl/influxdb/influxdb_server_key.pem"
)

// DaCfg - stores parsed DataAgent config
var DaCfg config.DAConfig

func main() {

	flag.Parse()

	flag.Lookup("alsologtostderr").Value.Set("true")

	defer glog.Flush()
	if len(os.Args) < 1 {
		glog.Errorf("Usage: go run DataAgent/DataAgent.go " +
			"[-log_dir=<glog_dir_path>]")
		os.Exit(-1)
	}

	glog.Infof("**************STARTING DA**************")

	err := DaCfg.ReadFromVault()
	if err != nil {
		glog.Errorf("Error: %s", err)
		os.Exit(-1)
	}

	//Create the cert and key file for influx server
	fileName := filepath.Base(INFLUX_SERVER_CERTIFICATE_PATH)
	to_write := DaCfg.Certs[fileName].([]byte)
	err = ioutil.WriteFile(INFLUX_SERVER_CERTIFICATE_PATH, to_write, 0700)
	if err != nil {
		glog.Errorf("Failed to write file %s, error: %s", INFLUX_SERVER_CERTIFICATE_PATH, err)
		os.Exit(-1)
	}

	fileName = filepath.Base(INFLUX_SERVER_KEY_PATH)
	to_write = DaCfg.Certs[fileName].([]byte)
	err = ioutil.WriteFile(INFLUX_SERVER_KEY_PATH, to_write, 0700)
	if err != nil {
		glog.Errorf("Failed to write file %s, error: %s", INFLUX_SERVER_KEY_PATH, err)
		os.Exit(-1)
	}

	glog.Infof("**************Share grpc internal certs/keys to other dependency ETA modules")
	secretsDir := "/etc/ssl/grpc_int_ssl_secrets"
	keyArr := []string{"grpc_internal_client_certificate.pem", "grpc_internal_client_key.pem", "ca_certificate.pem"}
	for _, fileName := range keyArr {
		if data, ok := DaCfg.Certs[fileName].([]byte); ok {
			outputFile := secretsDir + "/" + fileName
			err := ioutil.WriteFile(outputFile, data, 0700)
			if err != nil {
				glog.Errorf("Not able to write to secret file: %v, error: %v", outputFile, err)
				os.Exit(-1)
			}
		}
	}

	glog.Infof("*************STARTING INFLUX DB***********")
	influx_server := exec.Command("/root/go/src/ElephantTrunkArch/DataAgent/influx_start.sh")
	err = influx_server.Run()
	if err != nil {
		glog.Errorf("Failed to start influxdb Server, Error: %s", err)
		os.Exit(-1)
	}

	for {
		InfluxConn, err := net.DialTimeout("tcp", net.JoinHostPort("", "8086"), (5 * time.Second))
		if err != nil {
		}
		if InfluxConn != nil {
			InfluxConn.Close()
			break
		}
	}

	glog.Infof("*************INFLUX DB STARTED*********")
	influxCfg := DaCfg.InfluxDB
	influxServer := "localhost"
	clientAdmin, err := util.CreateHTTPClient(influxServer, influxCfg.Port, "", "")
	if err != nil {
		glog.Errorf("Error creating InfluxDB client: %v", err)
		os.Exit(-1)
	}
	resp, err := util.CreateAdminUser(clientAdmin, influxCfg.UserName, influxCfg.Password, influxCfg.DBName)

	if err == nil && resp.Error() == nil {
		glog.Infof("Successfully created admin user: %s", influxCfg.UserName)
	} else {
		if resp != nil && resp.Error() != nil {
			glog.Errorf("Error code: %v, Error Response: %s while creating "+
				"admin user: %s", err, resp.Error(), influxCfg.UserName)
		} else {
			glog.Errorf("Error code: %v while creating "+"admin user: %s", err, influxCfg.UserName)
		}
		//os.Exit(-1)
	}
	clientAdmin.Close()

	// Create InfluxDB database
	glog.Infof("Creating InfluxDB database: %s", influxCfg.DBName)
	client, err := util.CreateHTTPClient(influxServer,
		influxCfg.Port, influxCfg.UserName, influxCfg.Password)

	if err != nil {
		glog.Errorf("Error creating InfluxDB client: %v", err)
		os.Exit(-1)
	}

	response, err := util.CreateDatabase(client, influxCfg.DBName, influxCfg.Retention)
	if err != nil {
		glog.Errorf("Cannot create database: %s", response.Error())
	}

	if err == nil && response.Error() == nil {
		glog.Infof("Successfully created database: %s", influxCfg.DBName)
	} else {
		if response.Error() != nil {
			glog.Errorf("Error code: %v, Error Response: %s while creating "+
				"database: %s", err, response.Error(), influxCfg.DBName)
		} else {
			glog.Errorf("Error code: %v while creating "+"database: %s", err, influxCfg.DBName)
		}
		os.Exit(-1)
	}
	defer client.Close()

	// Init StreamManager
	glog.Infof("**************STARTING STREAM MANAGER**************")

	var pStreamManager = new(stm.StrmMgr)

	// This change is required to tie the opcua address to localhost or container's address
	hostname, err := os.Hostname()
	if err != nil {
		glog.Errorf("Failed to fetch the hostname of the node: %v", err)
	}
	if strmMgrUDPServHost != hostname {
		strmMgrUDPServHost = "localhost"
	}

	pStreamManager.ServerHost = strmMgrUDPServHost
	pStreamManager.ServerPort = strmMgrUDPServPort
	pStreamManager.InfluxDBHost = influxServer
	pStreamManager.InfluxDBPort = DaCfg.InfluxDB.Port
	pStreamManager.InfluxDBName = DaCfg.InfluxDB.DBName
	pStreamManager.InfluxDBUserName = DaCfg.InfluxDB.UserName
	pStreamManager.InfluxDBPassword = DaCfg.InfluxDB.Password
	pStreamManager.MsrmtTopicMap = make(map[string]stm.OutStreamConfig)
	pStreamManager.MeasurementPolicy = make(map[string]bool)
	pStreamManager.OpcuaPort = DaCfg.Opcua.Port

	glog.Infof("Going to start UDP server for influx subscription")
	err = pStreamManager.Init(DaCfg)
	if err != nil {
		glog.Errorf("Failed to initialize StreamManager : %v", err)
		os.Exit(-1)
	}

	var config = new(stm.OutStreamConfig)

	// Fetch the streams from the DA config file
	for key, val := range DaCfg.OutStreams {
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

	// glog.Infof("**************STARTING GRPC SERVER**************")
	done := make(chan bool)
	glog.Infof("**************STARTING GRPC Internal SERVER**************")
	go internalserver.StartGrpcServer(DaCfg)
	// TODO: The external gRPC server will be enabled when we expose Config and
	// Query interfaces from DataAgent in future
	// glog.Infof("**************STARTING GRPC External SERVER**************")
	// go server.StartGrpcServer(DaCfg)
	// glog.Infof("**************Started GRPC servers**************")

	// Currently running this channel to keep the goroutine running
	// for StreamManager
	<-done
	glog.Infof("**************Exiting**************")
}
