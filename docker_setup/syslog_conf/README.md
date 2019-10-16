1. The EIS centralized logging architecture can be visualized as eis-containers--->rsyslog--->logstash--->elasticsearch--->kibana

2. eis-containers logs has to be forwarded to the rsyslog, running onto the local system. 
   For that, the eis-container's logging driver need to be syslog. 
   Anyone wants to forward the eis-container's log to the local rsyslog has to modify the file named '/etc/docker/daemon.json'.
   The sample 'daemon.json' is availale in the directory.
   After the modification of 'daemon.json' docker daemon has to be restarted using the below command
   'sudo systemctl restart docker'

3. rsyslog has to forward the received logs from containers to logstash.
   The file named 'eis.conf' has to be copyied into the directory name '/etc/rsyslog.d/'
   After copying this file, the rsyslog service need to restarted using the below command
   'sudo systemctl restart rsyslog'.
   For more information on the rsyslog, plase refer [https://www.rsyslog.com/plugins/](https://www.rsyslog.com/plugins/)

4. logstash has to write the received logs from rsyslog to elasticsearch.
   The required logstash configuration is available in the file named 'log-receiver.conf'
   nohup logstash  -f ./log-receiver.conf &

   Note: To start elasticsearch & logstash, java (java 11), has to be installed.
   Please refer [https://www.elastic.co/](https://www.elastic.co/) to install
   elasticsearch, logstash and kibana