/*
Copyright (c) 2018 Intel Corporation.

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

Explicit permissions are required to publish, distribute, sublicense, and/or sell copies of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

package server

import (
	config "ElephantTrunkArch/DataAgent/config"
	pb "ElephantTrunkArch/DataAgent/da_grpc/protobuff/go/pb_internal"
	"encoding/json"
	"errors"
	"net"
	"os"

	"github.com/golang/glog"
	"golang.org/x/net/context"
	"google.golang.org/grpc"
)

var gRPCHost = "localhost"

const (
	gRPCPort  = "50052"
	chunkSize = 64 * 1024 // 64 KB
)

// IntDaCfg - stores parsed DataAgent config
var IntDaCfg config.DAConfig

// DainternalServer is used to implement DataAgent.DaServer
type DainternalServer struct{}

// GetConfigInt implementation
func (s *DainternalServer) GetConfigInt(ctx context.Context, in *pb.ConfigIntReq) (*pb.ConfigIntResp, error) {
	glog.Infoln("Request received:", in)
	jsonStr, err := getConfig(in.CfgType)
	glog.Infof("Response being sent...")
	return &pb.ConfigIntResp{JsonMsg: jsonStr}, err
}

func getConfig(cfgType string) (string, error) {

	var buf []byte
	var err error
	err = nil

	switch cfgType {
	case "InfluxDBCfg":
		buf, err = json.Marshal(IntDaCfg.InfluxDB)
	case "RedisCfg":
		buf, err = json.Marshal(IntDaCfg.Redis)
	case "PersistentImageStore":
		buf, err = json.Marshal(IntDaCfg.PersistentImageStore)
	case "MinioCfg":
		buf, err = json.Marshal(IntDaCfg.Minio)
	default:
		return "", errors.New("Not a valid config type")
	}

	return string(buf), err
}

// StartGrpcServer starts gRPC server
func StartGrpcServer(DaCfg config.DAConfig) {

	IntDaCfg = DaCfg
	ipAddr, err := net.LookupIP("ia_data_agent")
	if err != nil {
		glog.Errorf("Failed to fetch the IP address for host: %v, error:%v", ipAddr, err)
	} else {
		gRPCHost = ipAddr[0].String()
	}

	addr := gRPCHost + ":" + gRPCPort

	lis, err := net.Listen("tcp", addr)
	if err != nil {
		glog.Errorf("failed to listen: %v", err)
		os.Exit(-1)
	}

	//Create the gRPC server
	s := grpc.NewServer()

	//Register the handle object
	pb.RegisterDainternalServer(s, &DainternalServer{})

	glog.Infof("gRPC server is listening at: %s", addr)

	//Serve and listen
	if err := s.Serve(lis); err != nil {
		glog.Errorf("grpc serve error: %s", err)
		os.Exit(-1)
	}
}

// CloseGrpcServer closes gRPC server
func CloseGrpcServer(done chan bool) {
	done <- true
}
