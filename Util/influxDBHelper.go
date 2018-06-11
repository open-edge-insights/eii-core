package util

import (
	"fmt"
	"strings"

	"github.com/influxdata/influxdb/client/v2"
)

// CreateHTTPClient returns a http client connected to Influx DB
func CreateHTTPClient(host string, port string, userName string, passwd string) (client.Client, error) {
	var buff strings.Builder

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
	var buff strings.Builder

	fmt.Fprintf(&buff, "create subscription %s ON \"%s\".\"autogen\" DESTINATIONS ALL 'udp://%s:%s'", subName, dbName, udpHost, udpPort)

	q := client.NewQuery(buff.String(), "", "")
	response, err := cli.Query(q)
	return response, err
}

// CreateDatabase creates the InfluxDB database
func CreateDatabase(cli client.Client, dbName string) (*client.Response, error) {
	var buff strings.Builder

	fmt.Fprintf(&buff, "create database %s", dbName)

	q := client.NewQuery(buff.String(), "", "")
	response, err := cli.Query(q)
	return response, err
}
