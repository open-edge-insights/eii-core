/*
Copyright (c) 2019 Intel Corporation.

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

package main

import (
	etcdclient "IEdgeInsights/libs/ConfigManager/etcd/go"
	"flag"
	"fmt"
	"log"
	"time"
)

func callback(key string, value string) {
	fmt.Println("\ncallback is called")
	fmt.Printf("watch function is watching the key: %s \nnew value is: %s\n", key, value)
}

func main() {

	endPointArg := flag.String("endpoint", "", "Provide the endpoint Eg:localhost:2379")
	certFile := flag.String("certFile", "", "provide client certificate file")
	privateFile := flag.String("privateFile", "", "provide client private key file")
	trustFile := flag.String("trustFile", "", "provide ca cert file")
	key := flag.String("key", "", "provide etcd key")
	action := flag.String("action", "", "provide the action to be performed on etcd key Eg: get|watchkey|watchdir")
	flag.Parse()

	endPoint := []string{*endPointArg}

	config := etcdclient.Config{Endpoint: endPoint, CertFile: *certFile, KeyFile: *privateFile, TrustFile: *trustFile}

	etcd, err := etcdclient.NewEtcdCli(config)
	if err != nil {
		panic(err)
	}

	if *action == "get" {
		value, err := etcd.GetConfig(*key)
		fmt.Printf("value is %s", value)
		if err != nil {
			log.Fatal(err)
		}
		fmt.Printf("GetConfig is called and the value is: %s", value)
		return
	} else if *action == "watchkey" {
		fmt.Printf("Watching on the key %s", *key)
		etcd.RegisterKeyWatch(*key, callback)
	} else if *action == "watchdir" {
		fmt.Printf("Watching on the dir %s", *key)
		etcd.RegisterDirWatch(*key, callback)
	} else {
		fmt.Println("Provided action is not supported")
		return
	}

	for {
		time.Sleep(1)
	}

}
