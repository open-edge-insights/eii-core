package streammanger

import (
	"net"
	"os"
	"strings"

	influxDBHelper "iapoc_elephanttrunkarch/Util"

	"github.com/golang/glog"
	nats "github.com/nats-io/go-nats"
)

const (
	//NatsServerURL is same as nats.DefaultUrl now
	NatsServerURL = "nats://localhost:4222"

	subscriptionName = "StreamManagerSubscription"
)

// StrmMgr type
type StrmMgr struct {
	ServerHost        string //Server address
	ServerPort        string //Server Port
	InfluxDBHost      string
	InfluxDBPort      string
	InfluxDBName      string            // Database Name
	MeasurementPolicy map[string]bool   //This DS to map policy name with action
	MsrmtTopicMap     map[string]string //A map of mesurement to topic
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

func natsPublish(nc *nats.Conn, topic string, data string) error {

	nc.Publish(topic, []byte(data))
	nc.Flush()

	if err := nc.LastError(); err != nil {
		glog.Errorf("nats LastError: %v", err)
		return err
	}
	glog.Infof("Published [%s] : '%s'\n", topic, data)
	return nil

}

func (pStrmMgr *StrmMgr) handlePointData(nc *nats.Conn, addr *net.UDPAddr, buf string, n int) error {
	var err error
	err = nil
	glog.Infof("Received Data from address %s", addr)

	point := strings.Split(buf, " ")
	// It can have tag attached to it, let's split them too.
	msrTagsLst := strings.Split(point[0], ",")

	// Only allowing the measurements that are known to StreamManager
	for key, val := range pStrmMgr.MsrmtTopicMap {
		if key == msrTagsLst[0] {
			glog.Infof("Data Rcvd: %s\n", buf[0:n])
			glog.Infof("measurement and tag list: %s\n", msrTagsLst)
			// Publish only if nats connection is successful
			if nc != nil {
				go natsPublish(nc, val, buf[0:n])
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
		pStrmMgr.InfluxDBPort, "", "")

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
		glog.Errorf("Response error: %v", response.Error())
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

func startServer(pStrmMgr *StrmMgr) {

	var dstAddr string

	if pStrmMgr.ServerHost != "" {
		dstAddr = pStrmMgr.ServerHost + ":" + pStrmMgr.ServerPort
	} else {
		dstAddr = ":" + pStrmMgr.ServerPort
	}

	// Initialize the UDP server which listen on a predefined port
	servAddr, err := net.ResolveUDPAddr("udp", dstAddr)
	chkErr(err)

	servConnFd, err := net.ListenUDP("udp", servAddr)
	chkErr(err)
	glog.Infof("UDP server started gracefully at addr %s", dstAddr)
	defer servConnFd.Close()

	buf := make([]byte, 1024*1024)

	// TODO: check if this is the right place to put this
	glog.Infof("NATS publisher client connected to nats server at: %s", NatsServerURL)
	natsConn, err := nats.Connect(NatsServerURL)
	if err != nil {
		glog.Errorf("Failed to connect to nats server: %s, error: %v", NatsServerURL, err)
	}

	defer natsConn.Close()

	//TODO: break from for loop based on user signal,
	// Have a way to break from infinite loop
	for {
		//TODO: Should we break. Need channels here
		n, addr, err := servConnFd.ReadFromUDP(buf)
		if err != nil {
			glog.Errorln("ErrorSocketRecv: ", err)
			glog.Errorln("Stream manager: error in reading from socket")
		}

		go pStrmMgr.handlePointData(natsConn, addr, string(buf[:n]), n)
	}
}

// SetupOutStream func setups up out stream
func (pStrmMgr *StrmMgr) SetupOutStream(config *OutStreamConfig) error {

	var err error

	//populate the measurement->topic map
	pStrmMgr.MsrmtTopicMap[config.Measurement] = config.Topic

	return err
}
