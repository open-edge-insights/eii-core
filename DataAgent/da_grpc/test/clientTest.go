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

	var inputFile string
	var outputFile string
	flag.StringVar(&inputFile, "input_file", "", "input file path to write to ImageStore")
	flag.StringVar(&outputFile, "output_file", "", "output file that gets" +
				   "created from ImageStore read")

	flag.Parse()

	if len(os.Args) < 2 {
		glog.Errorf("Usage: go run DataAgent/da_grpc/test/clientTest.go " +
			"-input_file=<input_file_path> [-output_file=<output_file_path>]")
		os.Exit(-1)
	}

	flag.Lookup("alsologtostderr").Value.Set("true")
	defer glog.Flush()

	glog.Infof("******Go client gRPC testing******")

	glog.Infof("Testing GetConfigInt(\"InfluxDB\") gRPC interface...")
	respMap, err := client.GetConfigInt("InfluxDBCfg")
	chkErr(respMap, err)

	glog.Infof("Testing GetConfigInt(\"RedisCfg\") gRPC interface...")
	respMap, err = client.GetConfigInt("RedisCfg")
	chkErr(respMap, err)

	imagestore, err := imagestore.NewImageStore()
	if err != nil {
		glog.Infof("Failed to instantiate ImageStore. Error: %s", err)
		os.Exit(1)
	}
	imagestore.SetStorageType("inmemory")
	readBuffer := readFile(inputFile)
	imgHandle, err := imagestore.Store(readBuffer)
	if err != nil {
		glog.Infof("Failed imagestore.Store(). Error: %v", err)
		os.Exit(1)
	}

	glog.Infof("imgHandle: %s", imgHandle)

	glog.Infof("Testing GetBlob() Interface...")
	blob, err := client.GetBlob(imgHandle)

	ioutil.WriteFile(outputFile, blob, 0644)
	md5SumInput, _ := hashfilemd5(inputFile)
	md5SumOutput, _ := hashfilemd5(outputFile)
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
