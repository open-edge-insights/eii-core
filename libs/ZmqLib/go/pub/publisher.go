package publisher

import (
	"io/ioutil"
	"time"

	"github.com/golang/glog"
	zmq "github.com/pebbe/zmq4"
)

// PublishList struct is used to initialize topic, metadata & frame
type PublishList struct {
	Topic     string
	MetaData  map[string]string
	FrameBlob string
}

// loadCertificates is used to load all required certs
func loadCertificates(publicKeysDir string, privateKeysDir string) (string, string, string, string) {
	clientPublicKey, err := ioutil.ReadFile(publicKeysDir + "/client.key")
	if err != nil {
		glog.Errorf("Error reading clientPublicKey : %v", err)
		return "", "", "", ""
	}
	clientPublic := string(clientPublicKey)

	clientSecretKey, err := ioutil.ReadFile(privateKeysDir + "/client.key_secret")
	if err != nil {
		glog.Errorf("Error reading clientSecretKey : %v", err)
		return "", "", "", ""
	}
	clientSecret := string(clientSecretKey)

	serverPublicKey, err := ioutil.ReadFile(publicKeysDir + "/server.key")
	if err != nil {
		glog.Errorf("Error reading serverPublicKey : %v", err)
		return "", "", "", ""
	}
	serverPublic := string(serverPublicKey)

	serverSecretKey, err := ioutil.ReadFile(privateKeysDir + "/server.key_secret")
	if err != nil {
		glog.Errorf("Error reading serverSecretKey : %v", err)
		return "", "", "", ""
	}
	serverSecret := string(serverSecretKey)

	return clientPublic, clientSecret, serverPublic, serverSecret
}

// Init method is used to initialize the socket
func Init(address string, publicKeysDir string, privateKeysDir string) (*zmq.Socket, error) {

	// Publisher for dev mode
	if publicKeysDir == "" && privateKeysDir == "" {

		//  Create socket for publish
		publisher, err := zmq.NewSocket(zmq.PUB)
		if err != nil {
			glog.Errorf("Socket creation failed : %v", err)
			return nil, err
		}
		//  Bind to the socket
		errSoc := publisher.Bind(address)
		if errSoc != nil {
			glog.Errorf("Error binding to socket : %v", errSoc)
			return nil, errSoc
		}

		return publisher, nil
	}

	// Load required certs
	clientPublic, _, _, serverSecret := loadCertificates(publicKeysDir, privateKeysDir)

	zmq.AuthSetVerbose(true)
	zmq.AuthStart()

	//  Tell authenticator to use this public client key
	zmq.AuthCurveAdd("domain1", clientPublic)

	//  Create socket for publish
	publisher, err := zmq.NewSocket(zmq.PUB)
	if err != nil {
		glog.Errorf("Socket creation failed : %v", err)
		return nil, err
	}

	publisher.ServerAuthCurve("domain1", serverSecret)

	//  Bind to the socket
	errSoc := publisher.Bind(address)
	if errSoc != nil {
		glog.Errorf("Error binding to socket : %v", errSoc)
		return nil, errSoc
	}

	return publisher, nil
}

// Send method is used to send the meta-data and image
func Send(publisher *zmq.Socket, listOfElements PublishList) error {

	glog.Infof("Publishing message %s on topic %s", listOfElements.MetaData, listOfElements.Topic)

	//  Send topic
	_, err := publisher.Send(listOfElements.Topic, zmq.SNDMORE)
	if err != nil {
		glog.Errorf("Error while sending topic : %v", err)
		return err
	}

	//  Send meta-data and frame
	_, errMsg := publisher.SendMessage(listOfElements.MetaData, listOfElements.FrameBlob)
	if err != nil {
		glog.Errorf("Error while sending JSON & buffer : %v", errMsg)
		return errMsg
	}

	time.Sleep(time.Second)
	return nil
}

// Close method is used to close the socket
func Close(publisher *zmq.Socket) {
	publisher.Close()
}
