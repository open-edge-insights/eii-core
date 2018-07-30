package main

import (
	"crypto/md5"
	"encoding/hex"
	"flag"
	"fmt"
	client "iapoc_elephanttrunkarch/DataAgent/da_grpc/client"
	imagestore "iapoc_elephanttrunkarch/ImageStore/go/ImageStore"
	"io"
	"io/ioutil"
	"os"

	"github.com/golang/glog"
)

const count int = 20

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

func main() {
	if len(os.Args) != 3 {
		glog.Infof("Please specify an input and output path as command line arguments")
		os.Exit(1)
	}
	inputFile := os.Args[1] // input Index
	outputDir := os.Args[2] // output Index
	flag.Parse()
	flag.Lookup("alsologtostderr").Value.Set("true")
	defer glog.Flush()
	glog.Infof("******Go client gRPC testing******")
	glog.Infof("Getting InfluxDB config:")
	respMap, err := client.GetConfigInt("InfluxDBCfg")
	chkErr(respMap, err)
	glog.Infof("Getting Redis config:")
	respMap, err = client.GetConfigInt("RedisCfg")
	chkErr(respMap, err)
	imagestore, err := imagestore.NewImageStore()
	if err != nil {
		fmt.Println("Some Issue in Connection", err)
	}
	imagestore.SetStorageType("inmemory")
	readBuffer := readFile(inputFile)
	status, Imghandle := imagestore.Store(readBuffer)
	if status == false {
		os.Exit(1)
	}
	glog.Infof("Calling GetBlob() Interface...")
	blob, err := client.GetBlob(Imghandle)
	ioutil.WriteFile(outputDir+"getBlobOutput.jpg", blob, 0644)
	md5SumInput, _ := hashfilemd5(inputFile)
	md5SumOutput, _ := hashfilemd5(outputDir + "getBlobOutput.jpg")
	if md5SumInput == md5SumOutput {
		glog.Infof("md5sum of both files match")
	} else {
		glog.Infof("md5sum for both the files does not match")
	}
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
