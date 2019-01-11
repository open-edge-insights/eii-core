## **Configuring the Host as per Docker security recommendations.**

**1) Ensure a separate partition for containers has been created**
  
• To ensure proper isolation, it's a good idea to keep Docker containers and all of /var/lib/docker on their own filesystem partition.

After installing docker, in order to steps:

Remove all Docker containers and images.

``` 
	docker rm -f $(docker ps -aq); docker rmi -f $(docker images -q) 
```

Stop the Docker service.

``` 
	systemctl stop docker 
```

 Remove the Docker storage directory.

 ``` 
 	rm -rf /var/lib/docker
 ```

Create a new /var/lib/docker storage directory.

``` 
	mkdir /var/lib/docker
```

Use bind mount to set the new location. For example, to set the new location as /mnt/docker run the following commands:

```
	mkdir /mnt/docker
    
	mount --rbind /mnt/docker /var/lib/docker
```
  
Start the Docker service.

``` 
  	systemctl start docker
```

• Please find below instruction for creating new partition

[Ubuntu]- https://help.ubuntu.com/community/HowtoPartition/CreatingPartitions

[Clear Linux]-
https://clearlinux.org/documentation/clear-linux/get-started/bare-metal-install/cgdisk-manual-install
 

**2) Ensure Docker is up to Date**

Check which version is the current stable release by visiting the Docker CE release notes. If you're not up to date, please upgrade the Docker package.

3) Ensure auditing is configured for various Docker files

Install and configure auditd to enable auditing of some of Docker's files, directories, and sockets. Auditd is a Linux access monitoring and accounting subsystem that logs noteworthy system operations at the kernel level. More info https://www.systutorials.com/docs/linux/man/7-audit.rules/

Place the following snippet at the bottom of the file “/etc/audit/audit.rules”

```
  	    -w /usr/bin/docker -p wa
	    -w /var/lib/docker -p wa
	    -w /etc/docker -p wa
	    -w /lib/systemd/system/docker.service -p wa
	    -w /lib/systemd/system/docker.socket -p wa
	    -w /etc/default/docker -p wa
	    -w /etc/docker/daemon.json -p wa
	    -w /usr/bin/docker-containerd -p wa
	    -w /usr/bin/docker-runc -p wa
```
 

## Configuring the Docker Daemon as per Docker security recommendations
 

**2.1 Ensure security related parameter exists for Docker Daemon**

This section deals with the configuration of the Docker daemon. Create a configuration file for the daemon called daemon.json, to which we’ll add some security-related configuration parameters.

To begin, open up the configuration file in your favorite editor:

```
	sudo nano /etc/docker/daemon.json
```

This will present you with a blank text file. Paste in the following:

```
    {
    
    "icc": false,
    
    "log-driver": "syslog",
    
    "disable-legacy-registry": true,
    
    "live-restore": true,
    
    "userland-proxy": false,
    
    "no-new-privileges": true
    
    }

 ```
• Save and close the file, then restart the Docker daemon so it picks up this new configuration:

``` 
	sudo systemctl restart docker
```

**2.2 Ensure that authorization for Docker client commands is enabled**

If you need to allow network access to the Docker socket you should consult the official Docker documentation to find out how to set up the certificates and keys necessary to do so securely. https://docs.docker.com/engine/security/https/

*Please find more information about Docker bench security below. The Docker Bench for Security is a script that checks for dozens of common best-practices around deploying Docker containers in production.

https://github.com/docker/docker-bench-security







