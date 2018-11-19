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
	pb "ElephantTrunkArch/DataAgent/da_grpc/protobuff/go"
	imagestore "ElephantTrunkArch/ImageStore/go/ImageStore"
	"encoding/json"
	"errors"
	"net"
	"os"

	"github.com/golang/glog"
	"golang.org/x/net/context"
	"google.golang.org/grpc"

	"crypto/tls"
	"crypto/x509"
	"io/ioutil"

	"google.golang.org/grpc/credentials"
)

var gRPCHost = "localhost"

const (
	gRPCPort  = "50051"
	chunkSize = 64 * 1024 // 64 KB
)

// Server Certificates
const (
	RootCA     = "Certificates/ca/ca_certificate.pem"
	ServerCert = "Certificates/server/server_certificate.pem"
	ServerKey  = "Certificates/server/server_key.pem"
)

// ExtDaCfg - stores parsed DataAgent config
var ExtDaCfg config.DAConfig

// DaServer is used to implement DataAgent.DaServer
type DaServer struct{}

// GetBlob implementation
func (s *DaServer) GetBlob(in *pb.BlobReq, srv pb.Da_GetBlobServer) error {
	jsonStr, err := getConfig("RedisCfg")
	if err != nil {
		glog.Errorf("getConfig(\"RedisCfg\") method failed. Error: %v", err)
		return nil
	}

	var configMap map[string]string
	configBytes := []byte(jsonStr)
	json.Unmarshal(configBytes, &configMap)

	imgStore, err := imagestore.GetImageStoreInstance(configMap)
	if err != nil {
		glog.Errorf("Failed to instantiate NewImageStore(). Error: %v", err)
		return err
	}
	data, err := imgStore.Read(in.ImgHandle)
	if err != nil {
		glog.Errorf("Unable to read image frame or key not found")
		return err
	}
	byteMessage := []byte(data)

	chnk := &pb.Chunk{}
	//Iterating through the ByteArray for every 64 KB of chunks
	for currentByte := 0; currentByte < len(byteMessage); currentByte += chunkSize {
		if currentByte+chunkSize > len(byteMessage) {
			chnk.Chunk = byteMessage[currentByte:len(byteMessage)]
		} else {
			chnk.Chunk = byteMessage[currentByte : currentByte+chunkSize]
		}
		if err := srv.Send(chnk); err != nil {
			byteMessage = nil
			return err
		}
	}
	byteMessage = nil
	return nil
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
		buf, err = json.Marshal(ExtDaCfg.InfluxDB)
	case "RedisCfg":
		buf, err = json.Marshal(ExtDaCfg.Redis)
	default:
		return "", errors.New("Not a valid config type")
	}

	return string(buf), err
}

// StartGrpcServer starts gRPC server
func StartGrpcServer(DaCfg config.DAConfig) {

	ExtDaCfg = DaCfg
	ipAddr, err := net.LookupIP("ia_data_agent")
	if err != nil {
		glog.Errorf("Failed to fetch the IP address for host: %v, error:%v", ipAddr, err)
		os.Exit(-1)
	} else {
		gRPCHost = ipAddr[0].String()
	}

	addr := gRPCHost + ":" + gRPCPort

	// Read certificate binary
	certPEMBlock, err := ioutil.ReadFile(ServerCert)

	if err != nil {
		glog.Errorf("Failed to Read Server Certificate : %s", err)
		os.Exit(-1)
	}

	keyPEMBlock, err := ioutil.ReadFile(ServerKey)

	if err != nil {
		glog.Errorf("Failed to Read Server Key : %s", err)
		os.Exit(-1)
	}

	// Load the certificates from binary
	certificate, err := tls.X509KeyPair(certPEMBlock, keyPEMBlock)

	if err != nil {
		glog.Errorf("Failed to Load ServerKey Pair : %s", err)
		os.Exit(-1)
	}

	// Create a certificate pool from the certificate authority
	certPool := x509.NewCertPool()
	ca, err := ioutil.ReadFile(RootCA)
	if err != nil {
		glog.Errorf("Failed to Read CA Certificate : %s", err)
		os.Exit(-1)
	}

	// Append the certificates from the CA
	if ok := certPool.AppendCertsFromPEM(ca); !ok {
		glog.Errorf("Failed to Append CA Certificate")
		os.Exit(-1)
	}

	lis, err := net.Listen("tcp", addr)
	if err != nil {
		glog.Errorf("Failed to Listen: %v", err)
		os.Exit(-1)
	}

	// Create the TLS configuration to pass to the GRPC server
	creds := credentials.NewTLS(&tls.Config{
		ClientAuth:   tls.RequireAndVerifyClientCert,
		Certificates: []tls.Certificate{certificate},
		ClientCAs:    certPool,
	})

	//Create the gRPC server
	s := grpc.NewServer(grpc.Creds(creds))

	//Register the handle object
	pb.RegisterDaServer(s, &DaServer{})

	glog.Infof("Secure gRPC server Started & Listening at: %s", addr)

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
