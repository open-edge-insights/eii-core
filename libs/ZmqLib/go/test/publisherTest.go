package main

import (
	publisher "IEdgeInsights/libs/ZmqLib/go/pub"
	"flag"
	"os"

	"github.com/golang/glog"
)

// readFile is a local method to read image frame
func readFile(filename string) ([]byte, error) {
	file, err := os.Open(filename)
	if err != nil {
		glog.Errorf("Error: %v", err)
		return nil, nil
	}
	defer file.Close()

	fileinfo, err := file.Stat()
	if err != nil {
		glog.Errorf("Error: %v", err)
		return nil, nil
	}

	filesize := fileinfo.Size()
	buffer := make([]byte, filesize)
	_, err = file.Read(buffer)
	if err != nil {
		glog.Errorf("Error: %v", err)
		return nil, nil
	}
	return buffer, nil
}

// main method
func main() {

	var publicKeysDir string
	var privateKeysDir string
	flag.StringVar(&publicKeysDir, "public_keys", "", "Path to public keys dir")
	flag.StringVar(&privateKeysDir, "private_keys", "", "Path to private keys dir")
	flag.Parse()
	flag.Lookup("alsologtostderr").Value.Set("true")

	// input frame to be published
	inputData, err := readFile("./pic2.jpg")
	if err != nil {
		glog.Errorf("Error while reading file: %v", err)
	}
	frame := string(inputData)

	// topic over which to publish
	topic := "stream1_results"

	// JSON to be published
	metaData := map[string]string{
		"dtype": "uint8",
		"shape": "1200, 1920, 3",
		"name":  "1908",
	}

	// Initialize the socket
	soc, err := publisher.Init("tcp://*:5564", publicKeysDir, privateKeysDir)
	if err != nil {
		glog.Errorf("Initializing server failed: %v", err)
	}

	// Close the socket after transfer
	defer publisher.Close(soc)

	p := publisher.PublishList{Topic: topic, MetaData: metaData, FrameBlob: frame}

	for {
		// Publish topic meta-data and frame
		err := publisher.Send(soc, p)
		if err != nil {
			glog.Errorf("Error while publishing message: %v", err)
		}
	}
}
