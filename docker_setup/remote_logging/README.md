EIS distributed services centralized logging using ELK
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

4. To start ELK containers Please follow below commands
   ```
   $sudo sysctl -w vm.max_map_count=262144
   
   #To start in dev mode elk.yml can be used.
   $docker-compose -f elk.yml up -d
   
   #To start in prod mode elk_prod.yml can be used.
   $docker-compose -f elk_prod.yml up -d
   
   ```
   Pleas visit [https://localhost:5601](https://localhost:5601) for viewing the logs in KIBANA.
   Note: The certificate attached to kibana is self signed and has to be accepted in
   browser as an exception. The attached certificates are sample certificcates only and need to
   be replaced for production environment.
