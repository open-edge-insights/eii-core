/*
Copyright (c) 2018 Intel Corporation.

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. 
*/

package client

import (
	"encoding/json"
	pb "ElephantTrunkArch/DataAgent/da_grpc/protobuff"
	"io"
	"time"

	"github.com/golang/glog"
	"golang.org/x/net/context"
	"google.golang.org/grpc"
)

// GrpcClient structure
type GrpcClient struct {
	hostname string //hostname of the gRPC server
	port     string //gRPC port
}

// NewGrpcClient : This is the constructor to initialize the GrpcClient
func NewGrpcClient(hostname, port string) (*GrpcClient, error) {
	return &GrpcClient{hostname: hostname, port: port}, nil
}

// getDaClient: It is a private method called to get the DaClient object
func (pClient *GrpcClient) getDaClient() (pb.DaClient, error) {
	addr := pClient.hostname + ":" + pClient.port

	glog.Infof("Addr: %s", addr)
	conn, err := grpc.Dial(addr, grpc.WithInsecure())
	if err != nil {
		glog.Errorf("Did not connect: %v", err)
		return nil, err
	}

	daClient := pb.NewDaClient(conn)
	return daClient, nil
}

// GetBlob is a wrapper around gRPC go client implementation for GetBlob interface.
// It takes the imgHandle string(key from ImageStore) of the image frame ingested in the ImageStore
// and returns the consolidated byte array(value from ImageStore) from ImageStore associated with
// that imgHandle
func (pClient *GrpcClient) GetBlob(imgHandle string) ([]byte, error) {

	daClient, err := pClient.getDaClient()
	if err != nil {
		glog.Errorf("Error1: %v", err)
		return nil, err
	}
	client, err := daClient.GetBlob(context.Background(), &pb.BlobReq{ImgHandle: imgHandle})
	if err != nil {
		glog.Errorf("Error2: %v", err)
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
func (pClient *GrpcClient) GetConfigInt(config string) (map[string]string, error) {

	daClient, err := pClient.getDaClient()
	if err != nil {
		glog.Errorf("Error: %v", err)
		return nil, err
	}
	// Set the gRPC timeout
	ctx, cancel := context.WithTimeout(context.Background(), time.Second)
	defer cancel()

	var configMap map[string]string
	// gRPC call
	resp, err := daClient.GetConfigInt(ctx, &pb.ConfigIntReq{CfgType: config})
	if err != nil {
		glog.Errorf("Error3: %v", err)
		return configMap, err
	}

	configBytes := []byte(resp.JsonMsg)

	json.Unmarshal(configBytes, &configMap)

	return configMap, err
}
