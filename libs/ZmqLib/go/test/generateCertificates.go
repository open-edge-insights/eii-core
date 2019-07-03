package main

import (
	zmq "github.com/pebbe/zmq4"

	"io/ioutil"
	"log"
	"os"
	"runtime"
)

//  generateCertificates is used to generate certificates
func generateCertificates(path string) {

	os.MkdirAll(path+"public_keys", 0777)
	os.MkdirAll(path+"private_keys", 0777)
	clientPublic, clientSecret, err := zmq.NewCurveKeypair()
	checkErr(err)

	clientPublicKey := []byte(clientPublic)
	writeCPKErr := ioutil.WriteFile(path+"/public_keys/"+"client.key", clientPublicKey, 0777)
	checkErr(writeCPKErr)

	clientSecretKey := []byte(clientSecret)
	writeCSKErr := ioutil.WriteFile(path+"/private_keys/"+"client.key_secret", clientSecretKey, 0777)
	checkErr(writeCSKErr)

	serverPublic, serverSecret, err := zmq.NewCurveKeypair()
	checkErr(err)

	serverPublicKey := []byte(serverPublic)
	writeSPKErr := ioutil.WriteFile(path+"/public_keys/"+"server.key", serverPublicKey, 0777)
	checkErr(writeSPKErr)

	serverSecretKey := []byte(serverSecret)
	writeSSKErr := ioutil.WriteFile(path+"/private_keys/"+"server.key_secret", serverSecretKey, 0777)
	checkErr(writeSSKErr)
}

//  checkErr is used to check for errors
func checkErr(err error) {
	if err != nil {
		log.SetFlags(0)
		_, filename, lineno, ok := runtime.Caller(1)
		if ok {
			log.Fatalf("%v:%v: %v", filename, lineno, err)
		} else {
			log.Fatalln(err)
		}
	}
}

func main() {

	//  generateCertificates is used to generate certificates
	generateCertificates("./")

}
