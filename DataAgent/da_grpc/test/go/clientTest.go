/*
Copyright (c) 2018 Intel Corporation.
Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

package main

import (
	client "ElephantTrunkArch/DataAgent/da_grpc/client/go"
	imagestore "ElephantTrunkArch/ImageStore/go/ImageStore"
	"crypto/md5"
	"encoding/hex"
	"flag"
	"fmt"
	"io"
	"io/ioutil"
	"os"
	"time"

	"github.com/golang/glog"
)

const (
	iter1 = 20
)

var (
	start     time.Time
	end       time.Time
	timeTaken float64
	totalTime float64
)

func chkErr(msg map[string]string, err error) {
	if err != nil {
		glog.Errorf("Error: %v", err)
	} else {
		glog.Infof("Response: %s", msg)
	}
}

func readFile(filename string) []byte {
	file, err := os.Open(filename)
	if err != nil {
		fmt.Println(err)
	}
	defer file.Close()
	fileinfo, err := file.Stat()
	if err != nil {
		fmt.Println(err)
	}
	filesize := fileinfo.Size()
	buffer := make([]byte, filesize)
	bytesread, err := file.Read(buffer)
	if err != nil {
		fmt.Println(err, bytesread)
	}
	return buffer
}

// grpc client certificates
const (
	RootCA     = "/etc/ssl/grpc_int_ssl_secrets/ca_certificate.pem"
	ClientCert = "/etc/ssl/grpc_int_ssl_secrets/grpc_internal_client_certificate.pem"
	ClientKey  = "/etc/ssl/grpc_int_ssl_secrets/grpc_internal_client_key.pem"
)

func main() {

	var inputFile string
	var outputFile string
	flag.StringVar(&inputFile, "input_file", "", "input file path to write to ImageStore")
	flag.StringVar(&outputFile, "output_file", "", "output file that gets"+
		"created from ImageStore read")

	flag.Parse()

	if len(os.Args) < 2 {
		glog.Errorf("Usage: go run DataAgent/da_grpc/test/clientTest.go " +
			"-input_file=<input_file_path> [-output_file=<output_file_path>]")
		os.Exit(-1)
	}

	flag.Lookup("alsologtostderr").Value.Set("true")
	defer glog.Flush()

	grpcClient, err := client.NewGrpcClient(ClientCert, ClientKey, RootCA, "localhost", "50051")
	if err != nil {
		glog.Errorf("Error while obtaining GrpcClient object...")
		os.Exit(-1)
	}

	glog.Infof("******Go client gRPC testing******")

	imagestore, err := imagestore.NewImageStore()
	if err != nil {
		glog.Infof("Failed to instantiate ImageStore. Error: %s", err)
		os.Exit(1)
	}
	imagestore.SetStorageType("inmemory")
	timeTaken = 0.0
	totalTime = 0.0
	for i := 0; i < iter1; i++ {
		readBuffer := readFile(inputFile)
		imgHandle, err := imagestore.Store(readBuffer)
		if err != nil {
			glog.Infof("Failed imagestore.Store(). Error: %v", err)
			os.Exit(1)
		}
		glog.Infof("imgHandle: %s", imgHandle)

		glog.Infof("Testing GetBlob() Interface...")

		start = time.Now()
		blob, err := grpcClient.GetBlob(imgHandle)
		timeTaken = time.Since(start).Seconds()
		if err != nil {
			glog.Errorf("Error: %v", err)
		}
		glog.Infof("GetBlob call...index: %v, time: %v secs", i, timeTaken)
		totalTime += timeTaken

		ioutil.WriteFile(outputFile, blob, 0644)
		md5SumInput, _ := hashfilemd5(inputFile)
		md5SumOutput, _ := hashfilemd5(outputFile)
		if md5SumInput == md5SumOutput {
			glog.Infof("md5sum of both files match")
		} else {
			glog.Infof("md5sum for both the files does not match")
		}
	}
	glog.Infof("Average time taken for GetBlob() %v calls: %v", iter1, totalTime/iter1)
}

func hashfilemd5(filePath string) (string, error) {
	//Initialize variable returnMD5String now in case an error has to be returned
	var returnMD5String string
	//Open the passed argument and check for any error
	file, err := os.Open(filePath)
	if err != nil {
		return returnMD5String, err
	}
	//Tell the program to call the following function when the current function returns
	defer file.Close()
	//Open a new hash interface to write to
	hash := md5.New()
	//Copy the file in the hash interface and check for any error
	if _, err := io.Copy(hash, file); err != nil {
		return returnMD5String, err
	}
	//Get the 16 bytes hash
	hashInBytes := hash.Sum(nil)[:16]
	//Convert the bytes to a string
	returnMD5String = hex.EncodeToString(hashInBytes)
	return returnMD5String, nil
}
