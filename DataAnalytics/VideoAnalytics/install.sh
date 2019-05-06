#!/bin/bash -x

source ../../docker_setup/.env

#Install python dependencies
sudo -H pip3.6 install -r va_requirements.txt

# Create symbolic link
mkdir -p algos
ln -sdf ../../../docker_setup/config/algo_config ./algos/algo_config

if [ $DEV_MODE = "false" ]
then
	# Set the Permission
	chmod -R 777 /etc/ssl

	# create the required directories for certificate and keys on bare-metal (docker host machine).
	mkdir -p /etc/ssl/ca/
	mkdir -p /etc/ssl/streamsublib
	mkdir -p /etc/ssl/imagestore/

	#Copy CA, certificates/keys to respective directories
	cp ../../cert-tool/Certificates/ca/ca_certificate.pem /etc/ssl/ca/ca_certificate.pem

	# Copy certificates from data agent
	#docker cp ia_data_agent:/etc/ssl/grpc_int_ssl_secrets /etc/ssl/
fi
