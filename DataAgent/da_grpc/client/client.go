package client

import (
	"encoding/json"
	pb "iapoc_elephanttrunkarch/DataAgent/da_grpc/protobuff"
	"io"
	"time"

	"github.com/golang/glog"
	"golang.org/x/net/context"
	"google.golang.org/grpc"
)

const (
	address = "localhost:50051"
)

// GetBlob is a wrapper around gRPC go client implementation for GetBlob interface.
// It takes the imgHandle string(key from ImageStore) of the image frame ingested in the ImageStore
// and returns the consolidated byte array(value from ImageStore) from ImageStore associated with
// that imgHandle
func GetBlob(imgHandle string) ([]byte, error) {
	conn, err := grpc.Dial(address, grpc.WithInsecure())
	if err != nil {
		glog.Errorf("Did not connect: %v", err)
		return nil, err
	}
	cc := pb.NewDaClient(conn)
	client, err := cc.GetBlob(context.Background(), &pb.BlobReq{ImgHandle: imgHandle})
	if err != nil {
		glog.Errorf("Error: %v", err)
		return nil, err
	}
	var blob []byte
	for {
		c, err := client.Recv()
		if err != nil {
			if err == io.EOF {
				glog.Infof("Transfer of %d bytes successful", len(blob))
				break
			}
			glog.Errorf("Error while receiving: %v", err)
			return nil, err
		}
		blob = append(blob, c.Chunk...)
	}
	return blob, err
}

// GetConfigInt is a wrapper around gRPC go client implementation for
// GetConfigInt interface. It takes the config as the parameter
// (InfluxDBCfg, RedisCfg etc.,) returning the json as map for that
// particular config
func GetConfigInt(config string) (map[string]string, error) {

	var err error
	err = nil

	var configMap map[string]string

	// Set up a connection to the server.
	conn, err := grpc.Dial(address, grpc.WithInsecure())
	if err != nil {
		glog.Errorf("Did not connect: %v", err)
		return configMap, err
	}

	defer conn.Close()

	c := pb.NewDaClient(conn)

	// Set the gRPC timeout
	ctx, cancel := context.WithTimeout(context.Background(), time.Second)
	defer cancel()

	// gRPC call
	resp, err := c.GetConfigInt(ctx, &pb.ConfigIntReq{CfgType: config})
	if err != nil {
		glog.Errorf("Error: %v", err)
		return configMap, err
	}

	configBytes := []byte(resp.JsonMsg)

	json.Unmarshal(configBytes, &configMap)

	return configMap, err
}
