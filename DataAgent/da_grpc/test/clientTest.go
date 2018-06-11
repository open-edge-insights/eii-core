package main

import (
	"flag"
	client "iapoc_elephanttrunkarch/DataAgent/da_grpc/client"

	"github.com/golang/glog"
)

const count int = 20

func chkErr(msg string, err error) {
	if err != nil {
		glog.Errorf("Error: %v", err)
	} else {
		glog.Infof("Response: %s", msg)
	}

}

func main() {

	flag.Parse()
	flag.Lookup("alsologtostderr").Value.Set("true")

	glog.Infof("******Go client gRPC testing******")

	for i := 1; i <= count; i++ {
		glog.Infof("Iter#: %d", i)
		glog.Infof("Getting InfluxDB config:")
		jsonMsg, err := client.GetConfigInt("InfluxDBCfg")

		chkErr(jsonMsg, err)

		glog.Infof("Getting Redis config:")
		jsonMsg, err = client.GetConfigInt("RedisCfg")
		chkErr(jsonMsg, err)

	}

	glog.Flush()
}
