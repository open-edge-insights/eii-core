/*
Copyright (c) 2018 Intel Corporation.

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

package client

import (
	pb "IEdgeInsights/DataAgent/da_grpc/protobuff/go/pb_internal"
	util "IEdgeInsights/Util"

	"crypto/tls"
	"crypto/x509"
	"encoding/json"
	"time"

	"github.com/golang/glog"
	"golang.org/x/net/context"
	"google.golang.org/grpc"
	"google.golang.org/grpc/credentials"
)

// GrpcInternalClient structure
type GrpcInternalClient struct {
	hostname   string //hostname of the gRPC server
	port       string //gRPC port
	caCert     string //CA cert path
	clientCert string //client cert path
	clientKey  string //client key path
}

// NewGrpcInternalClient : This is the constructor to initialize the GrpcClient
func NewGrpcInternalClient(clientCert, clientKey, caCert, hostname, port string) (*GrpcInternalClient, error) {
	return &GrpcInternalClient{hostname: hostname, port: port, clientCert: clientCert, clientKey: clientKey, caCert: caCert}, nil
}

// NewGrpcInternalClientUnsecured : This is the constructor to initialize the GrpcClient in dev mode
func NewGrpcInternalClientUnsecured(hostname, port string) (*GrpcInternalClient, error) {
	return &GrpcInternalClient{hostname: hostname, port: port, clientCert: "", clientKey: "", caCert: ""}, nil
}

// getDaClient: It is a private method called to get the DaClient object
func (pClient *GrpcInternalClient) getDaClient() (pb.DainternalClient, error) {

	var conn *grpc.ClientConn
	var err error
	addr := pClient.hostname + ":" + pClient.port

	if (pClient.clientKey != "") && (pClient.clientCert != "") &&
		(pClient.caCert != "") {
		// Read certificate binary
		certPEMBlock, err := util.GetDecryptedBlob(pClient.clientCert)
		if err != nil {
			glog.Errorf("Failed to Read Client Certificate : %s", err)
			return nil, err
		}

		keyPEMBlock, err := util.GetDecryptedBlob(pClient.clientKey)
		if err != nil {
			glog.Errorf("Failed to Read Client Key : %s", err)
			return nil, err
		}

		// Load the certificates from binary
		certificate, err := tls.X509KeyPair(certPEMBlock, keyPEMBlock)
		if err != nil {
			glog.Errorf("Failed to Load ClientKey Pair : %s", err)
			return nil, err
		}

		// Create a certificate pool from the certificate authority
		certPool := x509.NewCertPool()
		ca, err := util.GetDecryptedBlob(pClient.caCert)
		if err != nil {
			glog.Errorf("Failed to Read CA Certificate : %s", err)
			return nil, err
		}

		// Append the Certificates from the CA
		if ok := certPool.AppendCertsFromPEM(ca); !ok {
			glog.Errorf("Failed to Append Certificate")
			return nil, nil
		}

		// Create the TLS credentials for transport
		creds := credentials.NewTLS(&tls.Config{
			Certificates: []tls.Certificate{certificate},
			RootCAs:      certPool,
		})

		conn, err = grpc.Dial(addr, grpc.WithTransportCredentials(creds))
		if err != nil {
			glog.Errorf("Grpc Dial Error, Not able to Connect %s: %s", addr, err)
			return nil, err
		}

	} else {
		conn, err = grpc.Dial(addr, grpc.WithInsecure())
		if err != nil {
			glog.Errorf("Grpc Dial Error, Not able to Connect %s: %s", addr, err)
			return nil, err
		}
	}
	daClient := pb.NewDainternalClient(conn)
	return daClient, nil
}

// GetConfigInt is a wrapper around gRPC go client implementation for
// GetConfigInt interface. It takes the config as the parameter
// (InfluxDBCfg, RedisCfg etc.,) returning the json as map for that
// particular config
func (pClient *GrpcInternalClient) GetConfigInt(config string) (map[string]string, error) {

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
