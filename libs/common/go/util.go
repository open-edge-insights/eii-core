package util

import (
	"net"
	"os"
	"strconv"
	"strings"
	"time"

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
