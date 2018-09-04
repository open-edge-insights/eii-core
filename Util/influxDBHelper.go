/*
Copyright (c) 2018 Intel Corporation.

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

package util

import (
	"bytes"
	"fmt"

	"github.com/influxdata/influxdb/client/v2"
)

// CreateHTTPClient returns a http client connected to Influx DB
func CreateHTTPClient(host string, port string, userName string, passwd string) (client.Client, error) {
	var buff bytes.Buffer

	fmt.Fprintf(&buff, "http://%s:%s", host, port)

	client, err := client.NewHTTPClient(client.HTTPConfig{
		Addr:     buff.String(),
		Username: userName,
		Password: passwd,
	})
	return client, err
}

// CreateSubscription creates the subscription in InfluxDB
func CreateSubscription(cli client.Client, subName string, dbName string, udpHost string, udpPort string) (*client.Response, error) {
	var buff bytes.Buffer

	fmt.Fprintf(&buff, "create subscription %s ON \"%s\".\"autogen\" DESTINATIONS ALL 'udp://%s:%s'", subName, dbName, udpHost, udpPort)

	q := client.NewQuery(buff.String(), "", "")
	response, err := cli.Query(q)
	return response, err
}

// CreateDatabase creates the InfluxDB database
func CreateDatabase(cli client.Client, dbName string, retentionPolicy string) (*client.Response, error) {
	var buff bytes.Buffer

	if(retentionPolicy != "" ){
		fmt.Fprintf(&buff, "create database %s with duration %s", dbName, retentionPolicy)
	}else{
		fmt.Fprintf(&buff, "create database %s", dbName)
	}

	q := client.NewQuery(buff.String(), "", "")
	response, err := cli.Query(q)
	return response, err
}
