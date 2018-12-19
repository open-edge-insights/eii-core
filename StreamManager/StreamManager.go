/*
Copyright (c) 2018 Intel Corporation.

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

Explicit permissions are required to publish, distribute, sublicense, and/or sell copies of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

package streammanger

import (
	"fmt"
	"io/ioutil"
	"net/http"
	"os"
	"regexp"
	"strings"

	influxDBHelper "ElephantTrunkArch/Util/influxdb"

	databus "ElephantTrunkArch/DataBusAbstraction/go"

	"github.com/golang/glog"
)

var opcuaDatab databus.DataBus

const (
	subscriptionName = "StreamManagerSubscription"
)

var opcuaContext = map[string]string{
	"direction":   "PUB",
	"name":        "streammanager",
	"endpoint":    "opcua://%s:%s",
	"certFile":    "/etc/ssl/opcua/opcua_server_certificate.der",
	"privateFile": "/etc/ssl/opcua/opcua_server_key.der",
	"trustFile":   "/etc/ssl/ca/ca_certificate.der",
}

// StrmMgr type
type StrmMgr struct {
	ServerHost        string //Server address
	ServerPort        string //Server Port
	InfluxDBHost      string
	InfluxDBPort      string
	InfluxDBName      string // Database Name
	InfluxDBUserName  string
	InfluxDBPassword  string
	MeasurementPolicy map[string]bool            //This DS to map policy name with action
	MsrmtTopicMap     map[string]OutStreamConfig //A map of mesurement to topic
	OpcuaPort         string
}

// OutStreamConfig type
type OutStreamConfig struct {
	RetenPolicy string // retention policy
	Measurement string // Measurement name
	Topic       string // Topic on databus
	StreamFlow  bool   // Whether to start the streaming or stop
	MsgBusType  string // Message bus type
}

func chkErr(err error) {
	//TODO: Will remove it with proper logging mechanism
	if err != nil {
		glog.Error("Stream Manager Error: ", err)
		// TODO: Yet to decide whether we can exit here or
		os.Exit(-1)
	}
}

func databPublish(topicConfig map[string]string, data string) error {
	err := opcuaDatab.Publish(topicConfig, data)
	if err != nil {
		glog.Errorf("Publish Error: %v", err)
		return err
	}
	glog.Infof("Published [%s] : '%s'\n", topicConfig, data)
	return nil

}

//This convertToJSON handels only the current line protocol which is specific.
//ToDo: Generic convertToJSON. To Handle all type of stream coming to StreamManager (Example: To handle Tags).
func convertToJSON(data string) string {
	var jsonStr string
	jsonStr = "{\"Measurement\":" //Json string should start from { and
	//starting with a key "Measurement" for the value "classifier_results"

	jbuf := strings.Split(data, " ") //Split the data based on single white-space

	jsonStr += "\"" + jbuf[0] + "\"" + "," //Concatenating Measurement and classifier_results
	// and comma seperated for the next key-value pair.

	for i := 2; i <= len(jbuf)-1; i++ { // since number of white-spaces are there within the values
		jbuf[1] += " " + jbuf[i] // of key-value pair, concatenating all the split string
	} //to one string to handle the further steps.

	influxTS := ",influx_ts=" + jbuf[len(jbuf)-1]
	jbuf[1] = strings.Replace(jbuf[1], jbuf[len(jbuf)-1], influxTS, -1) //adding a key called influx_ts
	//for the value of influx timestamp
	jsonStr = jsonStr + " " + jbuf[1] + "}"
	keyValBuf := strings.Split(jbuf[1], "=")

	quotedKey := "\"" + keyValBuf[0] + "\""
	jsonStr = strings.Replace(jsonStr, keyValBuf[0], quotedKey, -1) //wrapping "ImgHandle" key within quotes

	for j := 1; j < len(keyValBuf)-1; j++ { //wrapping other keys with in the quotes
		keyBuf := strings.Split(keyValBuf[j], ",")
		quotedKey2 := "\"" + keyBuf[len(keyBuf)-1] + "\""
		jsonStr = strings.Replace(jsonStr, keyBuf[len(keyBuf)-1], quotedKey2, -1)
	}

	jsonStr = strings.Replace(jsonStr, "=", ":", -1)  //As the value of idx is in not in the Json
	regex := regexp.MustCompile(`([0-9]+i)`)          // acceptable format, hence making it string type
	jsonStr = regex.ReplaceAllString(jsonStr, `"$1"`) // by adding double quotes around it.
	return jsonStr
}

func (pStrmMgr *StrmMgr) handlePointData(buf string) error {
	var err error
	var jsonBuf string
	err = nil

	point := strings.Split(buf, " ")
	// It can have tag attached to it, let's split them too.
	msrTagsLst := strings.Split(point[0], ",")

	// Only allowing the measurements that are known to StreamManager
	for key, val := range pStrmMgr.MsrmtTopicMap {
		if key == msrTagsLst[0] {
			glog.Infof("measurement and tag list: %s\n", msrTagsLst)
			// Publish only if a proper databus context available
			if opcuaDatab != nil {
				topicConfig := map[string]string{
					"name": val.Topic,
					"type": "string",
				}

				jsonBuf = convertToJSON(buf)
				databPublish(topicConfig, jsonBuf)
			}
		}
	}
	return err
}

// Init func subscribes to InfluxDB and starts up the udp server
func (pStrmMgr *StrmMgr) Init() error {

	//Setup the subscription for the DB
	// We have one DB only to be used by DA. Hence adding subscription
	// only during inititialization.

	client, err := influxDBHelper.CreateHTTPClient(pStrmMgr.InfluxDBHost,
		pStrmMgr.InfluxDBPort, pStrmMgr.InfluxDBUserName, pStrmMgr.InfluxDBPassword)

	if err != nil {
		glog.Errorf("Error creating InfluxDB client: %v", err)
	}

	defer client.Close()

	response, err := influxDBHelper.CreateSubscription(client, subscriptionName,
		pStrmMgr.InfluxDBName, pStrmMgr.ServerHost, pStrmMgr.ServerPort)

	if err == nil && response.Error() == nil {
		glog.Infoln("Successfully created subscription")
		go startServer(pStrmMgr)
	} else if response.Error() != nil {
		glog.Errorf("Response error: %v while creating subscription", response.Error())
		const str = "already exists"

		// TODO: we need to handle this situation in a more better way in
		// future in cases when DataAgent dies abruptly, system reboots etc.,
		if strings.Contains(response.Error().Error(), str) {
			glog.Infoln("subscription already exists, let's start the UDP" +
				" server anyways..")
			go startServer(pStrmMgr)
		}
	}

	return err
}

func (pStrmMgr *StrmMgr) httpHandlerFunc(w http.ResponseWriter, req *http.Request) {

	reqBody, err := ioutil.ReadAll(req.Body)
	if err != nil {
		glog.Errorf("Error in reading the data: %v", err)
	}

	w.Write([]byte("Received a POST request\n"))
	pStrmMgr.handlePointData(string(reqBody))

}

func startServer(pStrmMgr *StrmMgr) {
	var dstAddr string
	var err error
	if pStrmMgr.ServerHost != "" {
		dstAddr = pStrmMgr.ServerHost + ":" + pStrmMgr.ServerPort
	} else {
		dstAddr = ":" + pStrmMgr.ServerPort
	}

	// TODO: check if this is the right place to put this
	opcuaDatab, err = databus.NewDataBus()
	if err != nil {
		glog.Errorf("New DataBus Instance creation Error: %v", err)
		os.Exit(1)
	}

	// This change is required to tie the opcua address to localhost or container's address
	hostname, err := os.Hostname()
	if err != nil {
		glog.Errorf("Failed to fetch the hostname of the node: %v", err)
	}
	if hostname != "ia_data_agent" {
		hostname = "localhost"
	}

	opcuaContext["endpoint"] = fmt.Sprintf(opcuaContext["endpoint"], hostname, pStrmMgr.OpcuaPort)

	err = opcuaDatab.ContextCreate(opcuaContext)
	if err != nil {
		glog.Errorf("DataBus-OPCUA context creation Error: %v", err)
		os.Exit(1)
	}
	defer opcuaDatab.ContextDestroy()
	glog.Infof("Databus-OPCUA context created")

	//TODO: break from for loop based on user signal,
	//Have a way to break from infinite loop
	//Dummy publish for OPCUA
	for _, val := range pStrmMgr.MsrmtTopicMap {
		if val.MsgBusType == "OPCUA" {
			dummyMsg := "dummy"
			topicConfig := map[string]string{"name": val.Topic, "type": "string"}
			err := opcuaDatab.Publish(topicConfig, dummyMsg)
			if err != nil {
				glog.Errorf("Publish Error: %v", err)
			}
		}
	}

	http.HandleFunc("/", pStrmMgr.httpHandlerFunc)
	err = http.ListenAndServeTLS(dstAddr,
		"/etc/ssl/streammanager/streammanager_server_certificate.pem",
		"/etc/ssl/streammanager/streammanager_server_key.pem",
		nil)
	if err != nil {
		glog.Errorf("Error in connection to client due to: %v\n", err)
		os.Exit(-1)
	}
}

// SetupOutStream func setups up out stream
func (pStrmMgr *StrmMgr) SetupOutStream(config *OutStreamConfig) error {

	var err error
	glog.Infoln("outstream config: ", *config)
	//populate the measurement->topic map
	pStrmMgr.MsrmtTopicMap[config.Measurement] = *config

	return err
}
