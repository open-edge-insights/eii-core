/*
Copyright (c) 2018 Intel Corporation.

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

package main

import (
	client "IEdgeInsights/DataAgent/da_grpc/client/go/client_internal"
	"errors"
	"fmt"
	"os"
	"time"

	"github.com/golang/glog"
)

const (
	iter = 50
)

var (
	start     time.Time
	end       time.Time
	timeTaken float64
	totalTime float64
)

func chkErr(msg map[string]string, err error) {
	if err != nil {
		glog.Errorf("Error: %v", err)
	} else {
		glog.Infof("Response: %s", msg)
	}
}

func readFile(filename string) []byte {
	file, err := os.Open(filename)
	if err != nil {
		fmt.Println(err)
	}
	defer file.Close()
	fileinfo, err := file.Stat()
	if err != nil {
		fmt.Println(err)
	}
	filesize := fileinfo.Size()
	buffer := make([]byte, filesize)
	bytesread, err := file.Read(buffer)
	if err != nil {
		fmt.Println(err, bytesread)
	}
	return buffer
}

func getAverageTime(grpcClient *client.GrpcInternalClient, iter int, config string) (float64, error) {

	timeTaken = 0.0
	totalTime = 0.0
	if config != "InfluxDBCfg" && config != "RedisCfg" {
		return 0, errors.New("Not a valid config")
	}

	for i := 0; i < iter; i++ {
		start = time.Now()
		respMap, err := grpcClient.GetConfigInt(config)
		timeTaken = time.Since(start).Seconds()
		chkErr(respMap, err)
		glog.Infof("index: %v, time: %v secs, resp: %v", i, timeTaken, respMap)
		totalTime += timeTaken
	}
	return totalTime / float64(iter), nil
}

// grpc client certificates
const (
	RootCA     = "/etc/ssl/ca/ca_certificate.pem"
	ClientCert = "/etc/ssl/grpc_internal/grpc_internal_client_certificate.pem"
	ClientKey  = "/etc/ssl/grpc_internal/grpc_internal_client_key.pem"
)

func main() {

	grpcClient, err := client.NewGrpcInternalClient(ClientCert, ClientKey, RootCA, "localhost", "50052")
	if err != nil {
		glog.Errorf("Error while obtaining GrpcClient object...")
		os.Exit(-1)
	}
	glog.Infof("******Go client gRPC testing******")
	glog.Infof("Testing GetConfigInt(\"InfluxDB\") gRPC interface...")
	averageTime1, _ := getAverageTime(grpcClient, iter, "InfluxDBCfg")
	glog.Infof("Testing GetConfigInt(\"RedisCfg\") gRPC interface...")
	averageTime2, _ := getAverageTime(grpcClient, iter, "RedisCfg")
	glog.Infof("Average time taken for GetConfigInt(\"InfluxDBCfg\") %v calls: %v", iter, averageTime1)
	glog.Infof("Average time taken for GetConfigInt(\"RedisCfg\") %v calls: %v", iter, averageTime2)
}
