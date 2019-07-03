package main

import (
	subscriber "IEdgeInsights/libs/ZmqLib/go/sub"
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

	// topic over which subscription takes place
	topic := "stream1_results"

	// Initialize the socket
	soc, err := subscriber.Init("tcp://localhost:5564", publicKeysDir, privateKeysDir)
	if err != nil {
		glog.Errorf("Error while initializing server: %v", err)
	}

	// Close the socket after transfer
	defer subscriber.Close(soc)

	for {
		// Receive the meta-data and frame
		metaData, frame, err := subscriber.Recv(soc, topic)
		if err != nil {
			glog.Errorf("Error while subscribing: %v", err)
		}
		glog.Infof("Received message %s on topic %s", metaData, topic)
		writeFile("./pic2_go_output.jpg", frame)
	}
}
