SHELL := /bin/bash
CURDIR = $(shell pwd)
SAFESTRING_VER = 77b772849eda2321fb0dca56a321e3939930d7b9
SAFESTRING_DIR = safestringlib
DBA_PATH = ../../../DataBusAbstraction
PYXPATH = ../open62541
CERT_DIR_PATH = $(DBA_PATH)/../cert-tool/Certificates
SERVER_CERTS = /etc/ssl/opcua
CLIENT_CERTS = /etc/ssl/opcua
CA_CERTS = /etc/ssl/ca
INVALID_CERTS = ../../c/open62541/src/test/invalid_certs
HOST = localhost
PORT = 65003

build_safestring_lib:
	@echo "Clone the safestringlib if not cloned already..."
	git clone https://github.com/intel/safestringlib.git && \
	cd $(SAFESTRING_DIR) && \
	git checkout $(SAFESTRING_VER) && \
	make && \
	cp -rf libsafestring.a $(CURDIR)/../open62541 && \
	cd $(CURDIR) && \
	rm -rf $(SAFESTRING_DIR)

build:
	if [ -a $(PYXPATH) ] ; \
	then \
		echo "Building the open62541 wrapper library open62541W.so.." ; \
		cd $(PYXPATH) && make && cp open62541W.so .. && cd $(CURDIR) ; \
	fi

pub: build
	@echo "Starting the opcua secure server and start publishing the data.."
	python3.6 DataBusTest.py -direction PUB -endpoint opcua://$(HOST):$(PORT) -ns StreamManager \
							-topic classifier_results \
							-certFile $(SERVER_CERTS)/opcua_server_certificate.der \
							-privateFile $(SERVER_CERTS)/opcua_server_key.der \
							-trustFile $(CA_CERTS)/ca_certificate.der

sub: build
	@echo "Establish connection to opcua secure server and start subscribing the data.."
	python3.6 DataBusTest.py -direction SUB -endpoint opcua://$(HOST):$(PORT) -ns StreamManager \
							-topic classifier_results \
							-certFile $(CLIENT_CERTS)/opcua_client_certificate.der \
							-privateFile $(CLIENT_CERTS)/opcua_client_key.der \
							-trustFile $(CA_CERTS)/ca_certificate.der

pub_insecure: build
	@echo "Starting the opcua insecure server and start publishing the data.."
	python3.6 DataBusTest.py -direction PUB -endpoint opcua://$(HOST):$(PORT) -ns StreamManager \
							-topic classifier_results \
							-certFile "" -privateFile "" -trustFile ""

sub_insecure: build
	@echo "Establish connection to opcua insecure server and start subscribing the data.."
	python3.6 DataBusTest.py -direction SUB -endpoint opcua://$(HOST):$(PORT) -ns StreamManager \
							-topic classifier_results \
							-certFile "" -privateFile "" -trustFile ""

invalid_sub: build
	@echo "Establish connection to opcua server with invalid cert and start subscribing the data.."
	python3.6 DataBusTest.py -direction SUB -endpoint opcua://$(HOST):$(PORT) -ns StreamManager \
							-topic classifier_results \
							-certFile $(CLIENT_CERTS)/opcua_client_certificate.der \
							-privateFile $(CLIENT_CERTS)/opcua_client_key.der \
							-trustFile $(INVALID_CERTS)/ca/ca_certificate.der

clean:
	@echo "Removing all the binary files..."
	find $(DBA_PATH) -name \open62541W.so -type f -delete
	rm -rf $(CURDIR)/safestringlib $(CURDIR)/*.a $(CURDIR)/*.o $(CURDIR)/../*.a $(CURDIR)/../*.so
