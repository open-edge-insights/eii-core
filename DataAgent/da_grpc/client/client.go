package client

import (
	"time"

	"github.com/golang/glog"

	pb "iapoc_elephanttrunkarch/DataAgent/da_grpc/protobuff"

	"golang.org/x/net/context"
	"google.golang.org/grpc"
)

const (
	address = "localhost:50051"
)

// GetConfigInt is a wrapper around gRPC go client implementation for
// GetConfigInt interface. It takes the config as the parameter
// (InfluxDBCfg, RedisCfg etc.,) returning the json string for that
// particular config
func GetConfigInt(config string) (string, error) {

	var err error
	err = nil

	// Set up a connection to the server.
	conn, err := grpc.Dial(address, grpc.WithInsecure())
	if err != nil {
		glog.Errorf("Did not connect: %v", err)
		return "", err
	}

	defer conn.Close()

	c := pb.NewDaClient(conn)

	// Set the gRPC timeout
	ctx, cancel := context.WithTimeout(context.Background(), time.Second)
	defer cancel()

	// gRPC call
	resp, err := c.GetConfigInt(ctx, &pb.ConfigIntReq{CfgType: config})
	if err != nil {
		glog.Errorf("Erorr: %v", err)
		return "", err
	}
	return resp.JsonMsg, err
}
