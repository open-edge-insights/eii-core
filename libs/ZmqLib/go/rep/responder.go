package responder

import (
	"io/ioutil"

	"github.com/golang/glog"
	zmq "github.com/pebbe/zmq4"
)

// RespondList struct is used to initialize metadata & frame
type RespondList struct {
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

	// Responder for dev mode
	if publicKeysDir == "" && privateKeysDir == "" {

		//  Create socket for sending
		responder, err := zmq.NewSocket(zmq.REP)
		if err != nil {
			glog.Errorf("Socket creation failed : %v", err)
			return nil, err
		}
		//  Bind to the socket
		errSoc := responder.Bind(address)
		if errSoc != nil {
			glog.Errorf("Error binding to socket : %v", errSoc)
			return nil, errSoc
		}

		return responder, nil
	}

	// Load required certs
	clientPublic, _, _, serverSecret := loadCertificates(publicKeysDir, privateKeysDir)

	zmq.AuthSetVerbose(true)
	zmq.AuthStart()

	//  Tell authenticator to use this public client key
	zmq.AuthCurveAdd("domain1", clientPublic)

	//  Create socket for sending
	responder, err := zmq.NewSocket(zmq.REP)
	if err != nil {
		glog.Errorf("Socket creation failed : %v", err)
		return nil, err
	}

	responder.ServerAuthCurve("domain1", serverSecret)

	//  Bind to the socket
	errSoc := responder.Bind(address)
	if errSoc != nil {
		glog.Errorf("Error binding to socket : %v", errSoc)
		return nil, errSoc
	}

	return responder, nil
}

// Send method is used to send the meta-data and image
func Send(responder *zmq.Socket, callbck func(string) RespondList) error {

	request, err := responder.Recv(0)
	if err != nil {
		glog.Errorf("Error while receiving request : %v", err)
		return err
	}

	// Call the callback with obtained request
	listOfElements := callbck(request)

	glog.Infof("Sending message %s", listOfElements.MetaData)

	//  Send meta-data and frame
	_, errMsg := responder.SendMessage(listOfElements.MetaData, listOfElements.FrameBlob)
	if errMsg != nil {
		glog.Errorf("Error while sending JSON & buffer : %v", errMsg)
		return errMsg
	}
	return nil
}

// Close method is used to close the socket
func Close(responder *zmq.Socket) {
	responder.Close()
}
