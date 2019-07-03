package subscriber

import (
	"io/ioutil"

	"github.com/golang/glog"
	zmq "github.com/pebbe/zmq4"
)

// loadCertificates is used to load all required certs
func loadCertificates(publicKeysDir string, privateKeysDir string) (string, string, string, string) {
	clientPublicKey, err := ioutil.ReadFile(publicKeysDir + "/client.key")
	if err != nil {
		glog.Errorf("Error reading certs : %v", err)
		return "", "", "", ""
	}
	clientPublic := string(clientPublicKey)

	clientSecretKey, err := ioutil.ReadFile(privateKeysDir + "/client.key_secret")
	if err != nil {
		glog.Errorf("Error reading certs : %v", err)
		return "", "", "", ""
	}
	clientSecret := string(clientSecretKey)

	serverPublicKey, err := ioutil.ReadFile(publicKeysDir + "/server.key")
	if err != nil {
		glog.Errorf("Error reading certs : %v", err)
		return "", "", "", ""
	}
	serverPublic := string(serverPublicKey)

	serverSecretKey, err := ioutil.ReadFile(privateKeysDir + "/server.key_secret")
	if err != nil {
		glog.Errorf("Error reading certs : %v", err)
		return "", "", "", ""
	}
	serverSecret := string(serverSecretKey)

	return clientPublic, clientSecret, serverPublic, serverSecret
}

// Init method is used to initialize the socket
func Init(address string, publicKeysDir string, privateKeysDir string) (*zmq.Socket, error) {

	if publicKeysDir == "" && privateKeysDir == "" {

		//  Create socket for subscribe
		subscriber, err := zmq.NewSocket(zmq.SUB)
		if err != nil {
			glog.Errorf("Error creating socket : %v ", err)
			return nil, err
		}

		// Connect to the socket
		errSoc := subscriber.Connect(address)
		if errSoc != nil {
			glog.Errorf("Error connecting to socket : %v", errSoc)
			return nil, errSoc
		}

		return subscriber, nil
	}

	//  Create socket for subscribe
	subscriber, err := zmq.NewSocket(zmq.SUB)
	if err != nil {
		glog.Errorf("Error creating socket : %v ", err)
		return nil, err
	}

	// Load required certs
	clientPublic, clientSecret, serverPublic, _ := loadCertificates(publicKeysDir, privateKeysDir)

	subscriber.ClientAuthCurve(serverPublic, clientPublic, clientSecret)

	// Connect to the socket
	errSoc := subscriber.Connect(address)
	if errSoc != nil {
		glog.Errorf("Error connecting to socket : %v", errSoc)
		return nil, errSoc
	}

	return subscriber, nil
}

// Recv method is used to receive the meta-data and image
func Recv(subscriber *zmq.Socket, topic string) (string, string, error) {

	// Set topic to subscribe
	err := subscriber.SetSubscribe(topic)
	if err != nil {
		glog.Errorf("Error while setting topic : %v", err)
		return "", "", err
	}

	//  Receive the topic
	address, err := subscriber.Recv(0)
	if err != nil {
		glog.Errorf("Error while receiving topic %s : %v", address, err)
		return "", "", err
	}

	//  Receive the meta-data and frame
	contents, err := subscriber.RecvMessage(0)
	if err != nil {
		glog.Errorf("Error while receiving JSON & buffer : %v", err)
		return "", "", err
	}

	return contents[0], contents[1], nil
}

// Close method is used to close the socket
func Close(subscriber *zmq.Socket) {
	subscriber.Close()
}
