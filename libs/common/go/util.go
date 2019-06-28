package util

import (
	"net"
	"os"
	"strconv"
	"strings"
	"time"
	"errors"

	"github.com/golang/glog"
)

// CheckPortAvailability - checks for port availability on hostname
func CheckPortAvailability(hostname, port string) bool {
	maxRetries := 1000
	retryCount := 0

	portUp := false
	glog.Infof("Waiting for Port: %s on hostname: %s ", port, hostname)
	for retryCount < maxRetries {
		conn, _ := net.DialTimeout("tcp", net.JoinHostPort(hostname, port), (5 * time.Second))
		if conn != nil {
			glog.Infof("Port: %s on hostname: %s is up.", port, hostname)
			conn.Close()
			portUp = true
			break
		}
		time.Sleep(100 * time.Millisecond)
		retryCount++
	}
	return portUp
}

//GetTopicConfig - returns topic related congigurations
func GetTopicConfig(topic string, topicType string) map[string]interface{} {
	subConfigList := strings.Split(os.Getenv(topic+"_cfg"), ",")
	var config map[string]interface{}

	if subConfigList[0] == "zmq_tcp" {
		address := strings.Split(subConfigList[1], ":")
		hostname := address[0]
		port, err := strconv.ParseInt(address[1], 10, 64)
		if err != nil {
			glog.Errorf("string to int64 converstion Error: %v", err)
			os.Exit(1)
		}
		host := map[string]interface{}{
			"host": hostname,
			"port": port,
		}
		if strings.ToLower(topicType) == "pub" {
			config = map[string]interface{}{
				"type":            "zmq_tcp",
				"zmq_tcp_publish": host,
			}
		} else if strings.ToLower(topicType) == "sub" {
			config = map[string]interface{}{
				"type": "zmq_tcp",
				topic:  host,
			}
		} else {
			panic("Unsupported Topic Type!!!")
		}

	} else if subConfigList[0] == "zmq_ipc" {
		config = map[string]interface{}{
			"type":       "zmq_ipc",
			"socket_dir": subConfigList[1],
		}
	} else {
		panic("Unsupported MessageBus Type!!!")
	}
	return config
}

//GetTopics - returns list of topics based on topic type
func GetTopics(topicType string) []string {
	var topics []string
	if strings.ToLower(topicType) == "pub" {
		topics = strings.Split(os.Getenv("PubTopics"), ",")
	} else {
		topics = strings.Split(os.Getenv("SubTopics"), ",")
	}
	return topics
}

//GetEndPointMap will convert the env variable into the map 
//required for the zmq context creation
func GetEndPointMap(keywordName string, endPointInfo string) (map[string]interface{}, error) {
	const zmqTypeTCP = "zmq_tcp"
        const zmqTypeIpc = "zmq_ipc"
        const socketDir = "socket_dir"


	parts := strings.Split(endPointInfo, ",")

	if len(parts) <= 0 {
		return nil, errors.New("EndPont doesn not have type info")
	}

	infoEndPoint := make(map[string]interface{})
	if strings.Compare(parts[0], zmqTypeTCP) != 0 && strings.Compare(parts[0], zmqTypeIpc) != 0 {
		return nil, errors.New("Invalid type info")
	}

	typeInfo := strings.TrimSpace(parts[0])
	infoEndPoint["type"] = typeInfo

	if strings.Compare(typeInfo, zmqTypeTCP) == 0 {

		parts = strings.Split(parts[1], ":")
		if len(parts) != 2 {
			return nil, errors.New("Invalid endpoint info")
		}

		endPointInfo := make(map[string]interface{})
		endPointInfo["host"] = strings.TrimSpace(parts[0])
		portNumber, err := strconv.ParseInt(strings.TrimSpace(parts[1]), 10, 64)
		if err != nil {
			glog.Errorf("Not able to convert the port to Int %s", err)
		}
		endPointInfo["port"] = portNumber
		infoEndPoint[keywordName] = endPointInfo

	} else {
		infoEndPoint[socketDir] = strings.TrimSpace(parts[1])
	}

	return infoEndPoint, nil
}

