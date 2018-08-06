package server

import (
	"encoding/json"
	"errors"
	"iapoc_elephanttrunkarch/DataAgent/config"
	pb "iapoc_elephanttrunkarch/DataAgent/da_grpc/protobuff"
	imagestore "iapoc_elephanttrunkarch/ImageStore/go/ImageStore"
	"net"
	"os"

	"github.com/golang/glog"
	"golang.org/x/net/context"
	"google.golang.org/grpc"
)

var gRPCHost = "localhost"

const (
	gRPCPort  = "50051"
	chunkSize = 64 * 1024 // 64 KB
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
			return err
		}
	}
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
