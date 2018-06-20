package main

import (
	"flag"

	"github.com/golang/glog"

	util "iapoc_elephanttrunkarch/Util"
)

const (
	//InfluxDB config
	host     = "localhost"
	port     = "8086"
	userName = "root"
	password = "root"
	dbName   = "testDB"
	subName  = "testSubName"

	//UDP server config
	strmMgrUDPServHost = "localhost"
	strmMgrUDPServPort = "61971"
)

func main() {
	flag.Parse()
	flag.Lookup("alsologtostderr").Value.Set("true")

	defer glog.Flush()

	glog.Infoln("Creating InfluxDB HTTP client...")

	client, err := util.CreateHTTPClient(host, port, userName, password)

	if err != nil {
		glog.Errorf("Error creating InfluxDB client: %v", err)
	}

	glog.Infof("Creating database: %s\n", dbName)
	response, err := util.CreateDatabase(client, dbName)

	if err == nil && response.Error() == nil {
		glog.Infof("Response: %v", response.Results)
	} else {
		glog.Errorf("Error: %v while creating database: %s", err, dbName)
	}

	glog.Infof("Creating subscription: %s on db: %s", subName, dbName)
	response, err = util.CreateSubscription(client, subName, dbName,
		strmMgrUDPServHost, strmMgrUDPServPort)

	if err == nil && response.Error() == nil {
		glog.Infof("Response: %v", response.Results)
	} else {
		glog.Errorf("CreateSubscription error: %v", err)
		glog.Errorf("CreateSubscription response error: %v", response.Error())
	}

}
