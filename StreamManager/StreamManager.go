package StreamManager

import (
	"github.com/golang/glog"
	"github.com/influxdata/influxdb/client/v2"
	"net"
	"os"
	"strings"
)

type StrmMgr struct {
	ServerAdress      string //Server address
	ServerPort        string //Server Port
	InfluxAddr        string
	InfluxPort        string
	Database          string            // Database Name
	MeasurementPolicy map[string]bool   //This DS to map policy name with action
	Topic_map         map[string]string //A map of mesurement to topic
}

type OutStreamConfig struct {
	RetenPolicy string // retention policy
	Measurement string // Measurement name
	Topic       string // Topic on databus
	StreamFlow  bool   // Whether to start the streaming or stop
}

func chkErr(er error) {
	//TODO: Will remove it with proper logging mechanism
	if er != nil {
		glog.Error("Stream Manager Error: ", er)
		// TODO: Yet to decide whether we can exit here or
		os.Exit(-1)
	}
}

func (pStrmMgr *StrmMgr) handlePointData(buf string, n int) error {
	var er error
	er = nil
	point := strings.Split(buf, " ")
	// It can have tag attached to it, let's split them too.
	msr_tags_lst := strings.Split(point[0], ",")
	glog.Infof("Data Rcvd: %s\n", buf[0:n])
	glog.Infof("measurement and tag list: %s\n", msr_tags_lst)
	return er
}

func (pStrmMgr *StrmMgr) StreamManagerInit() error {

	//Setup the subscription for the DB
	// We have one DB only to be used by DA Hence adding subscription
	// only during inititialization.
	c, err := client.NewHTTPClient(client.HTTPConfig{
		Addr: "http://" + pStrmMgr.InfluxAddr + ":" + pStrmMgr.InfluxPort,
	})
	if err != nil {
		glog.Errorln("Error creating InfluxDB Client: ", err.Error())
	}
	defer c.Close()

	q := client.NewQuery("create subscription "+"StreamManagerSubscription"+
		" ON \""+pStrmMgr.Database+"\".\"autogen\" DESTINATIONS ALL 'udp://"+
		pStrmMgr.ServerAdress+":"+pStrmMgr.ServerPort+"'", "", "")
	response, err := c.Query(q)
	if err != nil && response.Error() != nil {
		glog.Infoln(response.Results)
		glog.Errorln(response.Error())
		return err
	}

	go start_server(pStrmMgr)

	return err
}

func start_server(pStrmMgr *StrmMgr) {
	var dst_addr string

	if pStrmMgr.ServerAdress != "" {
		dst_addr = pStrmMgr.ServerAdress + ":" + pStrmMgr.ServerPort
	} else {
		dst_addr = ":" + pStrmMgr.ServerPort
	}

	// Initialize the UDP server which listen on a predefined port
	ServAddr, err := net.ResolveUDPAddr("udp", dst_addr)
	chkErr(err)

	ServConnFd, err := net.ListenUDP("udp", ServAddr)
	chkErr(err)
	glog.Infof("UDP server started gracefully at addr %s", dst_addr)
	defer ServConnFd.Close()

	buf := make([]byte, 1024*1024)
	//TODO: break from for loop based on user signal,
	// Have a way to break from infinite loop
	for {
		//TODO: Should we break. Need channels here
		n, addr, err := ServConnFd.ReadFromUDP(buf)
		if err != nil {
			glog.Errorln("ErrorSocketRecv: ", err)
			glog.Errorln("Stream manager: error in reading from socket")
		}
		glog.Infof("Recieved Data from address %s", addr)
		go pStrmMgr.handlePointData(string(buf[:n]), n)
	}
}

func (pStrmMgr *StrmMgr) SetupOutStream(config *OutStreamConfig) error {
	//populate the measurment->topic map
	var err error = nil

	pStrmMgr.Topic_map[config.Measurement] = config.Topic

	return err
}
