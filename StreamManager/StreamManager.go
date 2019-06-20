/*
Copyright (c) 2018 Intel Corporation.

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

Explicit permissions are required to publish, distribute, sublicense, and/or sell copies of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

package streammanger

import (
	config "IEdgeInsights/DataAgent/config"
	databus "IEdgeInsights/DataBusAbstraction/go"
	util "IEdgeInsights/Util"
	influxDBHelper "IEdgeInsights/Util/influxdb"
	"fmt"
	"io/ioutil"
	"net/http"
	"os"
	re "regexp"
	"strconv"
	"strings"
	"time"

	"github.com/golang/glog"
)

var opcuaDatab databus.DataBus

// StrmDaCfg config
var StrmDaCfg config.DAConfig

const (
	subscriptionName  = "StreamManagerSubscription"
	maxPointsBuffered = 100
)

var opcuaContext = map[string]string{
	"direction":   "PUB",
	"endpoint":    "opcua://%s:%s",
	"certFile":    "/etc/ssl/opcua/opcua_server_certificate.der",
	"privateFile": "/etc/ssl/opcua/opcua_server_key.der",
	"trustFile":   "/etc/ssl/ca/ca_certificate.der",
}

var strmCertificatesPath = map[string]string{
	"strmMngrCertFile": "/etc/ssl/streammanager/streammanager_server_certificate.pem",
	"strmMngrKeyFile":  "/etc/ssl/streammanager/streammanager_server_key.pem",
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
	pData             chan string // The point data from Influx DB
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

	glog.V(1).Infof("Published [%s] : '%s'\n", topicConfig, data)
	return nil

}

//This convertToJSON handels only the current line protocol which is specific.
//ToDo: Generic convertToJSON. To Handle all type of stream coming to StreamManager (Example: To handle Tags).
func convertToJSON(data string) string {
	finalData := "Measurement="
	jbuf := strings.Split(data, " ")
	tagsValue := strings.Split(jbuf[0], ",")

	//  To handle json if tags are given in the line protocol
	tagsValue[0] = "\"" + tagsValue[0] + "\""
	var tagsValueList []string
	tagsValueList = append(tagsValueList, tagsValue[0])

	//  To identify the datapoints with the TAGS.
	matchString, err := re.MatchString(`([a-zA-Z0-9_]+)([,])([a-zA-Z0-9_]*)`, jbuf[0])
	if err != nil {
		fmt.Println("Matching regex failed..")
	}

	// To Replace the values with the quoted values.
	if matchString {
		for i := 1; i < len(tagsValue); i++ {
			tagKeyValue := strings.Split(tagsValue[i], "=")
			tagValue := "\"" + tagKeyValue[1] + "\""
			quotedKeyValueTag := tagKeyValue[0] + "=" + tagValue
			tagsValueList = append(tagsValueList, quotedKeyValueTag)
		}

		jbuf[0] = strings.Join(tagsValueList, ",")
		finalData += jbuf[0] + ","
	} else {
		finalData += "\"" + jbuf[0] + "\"" + ","
	}

	for i := 2; i < len(jbuf); i++ {
		jbuf[1] += " " + jbuf[i]
	}

	// Add the influx_ts key-value pair
	influxTS := ",influx_ts=" + jbuf[len(jbuf)-1]
	jbuf[1] = strings.Replace(jbuf[1], jbuf[len(jbuf)-1], influxTS, -1)
	finalData = finalData + jbuf[1]

	keyValueBuf := strings.Split(finalData, "=")
	quotedKey := "\"" + keyValueBuf[0] + "\""
	finalData = strings.Replace(finalData, keyValueBuf[0], quotedKey, -1)

	// Trim white spaces
	finalData = strings.Replace(finalData, " ", "", -1)

	// Replacing the Keys field with the quoted Keys.
	for j := 1; j < (len(keyValueBuf) - 1); j++ {
		keyBuf := strings.Split(keyValueBuf[j], ",")
		keyToFind := "," + keyBuf[len(keyBuf)-1] + "="
		quotedKeyToFind := ",\"" + keyBuf[len(keyBuf)-1] + "\"="
		finalData = strings.Replace(finalData, keyToFind, quotedKeyToFind, -1)
	}

	// Replacing all = with :
	finalData = strings.Replace(finalData, "=", ":", -1)

	//  Removal of "i" added by influx,from the integer value
	rex := re.MustCompile(`[0-9]+i`)
	variable := rex.FindAllString(finalData, -1)
	for i := 0; i < len(variable); i++ {
		strippedI := strings.Trim(variable[i], "i")
		finalData = strings.Replace(finalData, variable[i], strippedI, -1)
	}

	finalData = "{" + finalData + "}"
	return finalData
}

func (pStrmMgr *StrmMgr) handlePointData() {
	var jsonBuf string

	value := os.Getenv("PROFILING")
	profiling, err := strconv.ParseBool(value)
	if err != nil {
		glog.Errorf("Fail to read PROFILING environment variable: %s", err)
	}

	for {
		// Wait for data in point data buffer
		buf := <-pStrmMgr.pData

		point := strings.Split(buf, " ")
		// It can have tag attached to it, let's split them too.
		msrTagsLst := strings.Split(point[0], ",")

		// Only allowing the measurements that are known to StreamManager
		for key, val := range pStrmMgr.MsrmtTopicMap {
			if key == msrTagsLst[0] {
				glog.Infof("Publishing topic: %s\n", key)
				// Publish only if a proper databus context available
				if opcuaDatab != nil {
					topicConfig := map[string]string{
						"ns":   "streammanager",
						"name": val.Topic,
						"type": "string",
					}

					if profiling {
						var tempBuf string
						jbuf := strings.Split(buf, " ")

						for i := 0; i < (len(jbuf) - 1); i++ {
							if i == 0 {
								tempBuf += jbuf[i]
							} else {
								tempBuf += " " + jbuf[i]
							}
						}

						tempS := ",ts_sm_pub_entry="
						tempS += strconv.FormatInt((time.Now().UnixNano() / 1e6), 10)
						tempBuf += tempS + " " + jbuf[len(jbuf)-1]
						buf = tempBuf
					}

					jsonBuf = convertToJSON(buf)
					databPublish(topicConfig, jsonBuf)
				}
			}
		}
	}
}

// Init func subscribes to InfluxDB and starts up the udp server
func (pStrmMgr *StrmMgr) Init(DaCfg config.DAConfig) error {

	//Setup the subscription for the DB
	// We have one DB only to be used by DA. Hence adding subscription
	// only during inititialization.

	StrmDaCfg = DaCfg

	client, err := influxDBHelper.CreateHTTPClient(pStrmMgr.InfluxDBHost,
		pStrmMgr.InfluxDBPort, pStrmMgr.InfluxDBUserName, pStrmMgr.InfluxDBPassword, DaCfg.DevMode)

	if err != nil {
		glog.Errorf("Error creating InfluxDB client: %v", err)
	}

	defer client.Close()

	response, err := influxDBHelper.DropAllSubscriptions(client, pStrmMgr.InfluxDBName)
	if err != nil {
		glog.Errorln("Error in dropping subscriptions")

	}

	response, err = influxDBHelper.CreateSubscription(client, subscriptionName,
		pStrmMgr.InfluxDBName, pStrmMgr.ServerHost, pStrmMgr.ServerPort, StrmDaCfg.DevMode)

	if err == nil && response.Error() == nil {
		glog.Infoln("Successfully created subscription")
		go startServer(pStrmMgr, DaCfg.DevMode)
	} else if response.Error() != nil {
		glog.Errorf("Response error: %v while creating subscription", response.Error())
		const str = "already exists"

		// TODO: we need to handle this situation in a more better way in
		// future in cases when DataAgent dies abruptly, system reboots etc.,
		if strings.Contains(response.Error().Error(), str) {
			glog.Infoln("subscription already exists, let's start the UDP" +
				" server anyways..")
			go startServer(pStrmMgr, DaCfg.DevMode)
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

	select {
	case pStrmMgr.pData <- string(reqBody):
	default:
		glog.Infof("Discarding the point. Stream generation faster than Publish!")
	}
}

func startServer(pStrmMgr *StrmMgr, devMode bool) {
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

	hostname := os.Getenv("DATA_AGENT_GRPC_SERVER")

	if !devMode {
		opcuaFileList := []string{opcuaContext["certFile"], opcuaContext["privateFile"], opcuaContext["trustFile"]}
		util.WriteCertFile(opcuaFileList, StrmDaCfg.Certs)
	} else {
		// We need to nullify as this global variable created with initialized data.
		opcuaContext["certFile"] = ""
		opcuaContext["privateFile"] = ""
		opcuaContext["trustFile"] = ""
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
			topicConfig := map[string]string{"ns": "streammanager", "name": val.Topic, "type": "string"}
			err := opcuaDatab.Publish(topicConfig, dummyMsg)
			if err != nil {
				glog.Errorf("Publish Error: %v", err)
			}
		}
	}

	// Make the channel for handling point data
	pStrmMgr.pData = make(chan string, maxPointsBuffered)
	go pStrmMgr.handlePointData()

	// Start the HTTP server handler
	http.HandleFunc("/", pStrmMgr.httpHandlerFunc)

	if !devMode {
		// Populate the certificates from vault. TODO: make a util function to fill the same.
		StrmMngrFileList := []string{strmCertificatesPath["strmMngrCertFile"], strmCertificatesPath["strmMngrKeyFile"]}
		util.WriteCertFile(StrmMngrFileList, StrmDaCfg.Certs)

		err = http.ListenAndServeTLS(dstAddr,
			strmCertificatesPath["strmMngrCertFile"],
			strmCertificatesPath["strmMngrKeyFile"],
			nil)
	} else {
		err = http.ListenAndServe(dstAddr, nil)
	}
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
