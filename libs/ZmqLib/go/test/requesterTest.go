package main

import (
	requester "IEdgeInsights/libs/ZmqLib/go/req"
	"flag"
	"os"

	"github.com/golang/glog"
)

// writeFile is a local method to write images to a specified file
func writeFile(filename string, message string) error {
	filePointer, err := os.Create(filename)
	if err != nil {
		glog.Errorf("Error: %v", err)
		return err
	}
	defer filePointer.Close()
	n3, _ := filePointer.WriteString(message)
	glog.Infof("wrote %d bytes\n", n3)
	filePointer.Sync()
	return nil
}

// main method
func main() {

	var publicKeysDir string
	var privateKeysDir string
	flag.StringVar(&publicKeysDir, "public_keys", "", "Path to public keys dir")
	flag.StringVar(&privateKeysDir, "private_keys", "", "Path to private keys dir")
	flag.Parse()
	flag.Lookup("alsologtostderr").Value.Set("true")

	// request to be sent to responder
	request := "req_imgHandle"

	// Initialize the socket
	soc, err := requester.Init("tcp://localhost:5564", publicKeysDir, privateKeysDir)
	if err != nil {
		glog.Errorf("Error while initializing server: %v", err)
	}

	// Close the socket after transfer
	defer requester.Close(soc)

	for {
		// Receive the meta-data and frame
		metaData, frame, err := requester.Recv(soc, request)
		if err != nil {
			glog.Errorf("Error while subscribing: %v", err)
		}
		glog.Infof("Received message %s", metaData)
		writeFile("./pic2_go_output.jpg", frame)
	}
}
