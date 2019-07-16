package main

import (
	responder "IEdgeInsights/libs/ZmqLib/go/rep"
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

// callbackA is the callback called by the responder
func callbackA(request string) responder.RespondList {

	glog.Infof("Request from client: %s", request)

	// input frame to be sent
	inputData, err := readFile("./pic2.jpg")
	if err != nil {
		glog.Errorf("Error while reading file: %v", err)
	}
	frame := string(inputData)

	// JSON to be sent
	metaData := map[string]string{
		"dtype": "uint8",
		"shape": "1200, 1920, 3",
		"name":  "1908",
	}

	p := responder.RespondList{MetaData: metaData, FrameBlob: frame}
	return p
}

// main method
func main() {

	var publicKeysDir string
	var privateKeysDir string
	flag.StringVar(&publicKeysDir, "public_keys", "", "Path to public keys dir")
	flag.StringVar(&privateKeysDir, "private_keys", "", "Path to private keys dir")
	flag.Parse()
	flag.Lookup("alsologtostderr").Value.Set("true")

	// Initialize the socket
	soc, err := responder.Init("tcp://*:5564", publicKeysDir, privateKeysDir)
	if err != nil {
		glog.Errorf("Initializing server failed: %v", err)
	}

	// Close the socket after transfer
	defer responder.Close(soc)

	for {
		// Send meta-data and frame
		err := responder.Send(soc, callbackA)
		if err != nil {
			glog.Errorf("Error while publishing message: %v", err)
		}
	}
}
