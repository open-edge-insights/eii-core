package server

import (
	"encoding/json"
	"errors"
	"net"
	"os"

	"iapoc_elephanttrunkarch/DataAgent/config"
	pb "iapoc_elephanttrunkarch/DataAgent/da_grpc/protobuff"

	"github.com/golang/glog"
	"golang.org/x/net/context"
	"google.golang.org/grpc"
)

var gRPCHost = "localhost"

const (
	gRPCPort = "50051"
)

// DaCfg - stores parsed DataAgent config
var DaCfg config.DAConfig

// DaServer is used to implement DataAgent.DaServer
type DaServer struct{}

// GetConfigInt implementation
func (s *DaServer) GetConfigInt(ctx context.Context, in *pb.ConfigIntReq) (*pb.ConfigIntResp, error) {
	glog.Infoln("Request received:", in)
	jsonStr, err := getConfig(in.CfgType)
	glog.Infof("Response being sent...")
	return &pb.ConfigIntResp{JsonMsg: jsonStr}, err
}

// Query implementation
func (s *DaServer) Query(ctx context.Context, in *pb.QueryReq) (*pb.QueryResp, error) {
	return &pb.QueryResp{}, nil
}

// Config implementation
func (s *DaServer) Config(ctx context.Context, in *pb.ConfigReq) (*pb.ConfigResp, error) {
	return &pb.ConfigResp{}, nil
}

func getConfig(cfgType string) (string, error) {

	var buf []byte
	var err error
	err = nil

	switch cfgType {
	case "InfluxDBCfg":
		buf, err = json.Marshal(DaCfg.InfluxDB)
	case "RedisCfg":
		buf, err = json.Marshal(DaCfg.Redis)
	default:
		return "", errors.New("Not a valid config type")
	}

	return string(buf), err
}

// StartGrpcServer starts gRPC server
func StartGrpcServer() {

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
	pb.RegisterDaServer(s, &DaServer{})

	glog.Infof("gRPC server is listening at: %s", addr)

	//Serve and listen
	if err := s.Serve(lis); err != nil {
		glog.Errorf("grpc serve error: %s", err)
		os.Exit(-1)
	}

}
