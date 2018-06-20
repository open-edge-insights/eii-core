package main

import (
	"flag"
	client "iapoc_elephanttrunkarch/DataAgent/da_grpc/client"

	"github.com/golang/glog"
)

const count int = 20

func chkErr(msg map[string]string, err error) {
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
		respMap, err := client.GetConfigInt("InfluxDBCfg")

		chkErr(respMap, err)

		glog.Infof("Getting Redis config:")
		respMap, err = client.GetConfigInt("RedisCfg")
		chkErr(respMap, err)

	}

	glog.Flush()
}
