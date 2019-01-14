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

prebuild: clean
	@echo "Clone the safestringlib if not cloned already..."
	git clone https://github.com/intel/safestringlib.git && \
	cd $(SAFESTRING_DIR) && \
	git checkout $(SAFESTRING_VER) && \
	make && \
	cp -rf libsafestring.a ../ && \
	cp -rf libsafestring.a ../../ && \
	cp -rf libsafestring.a ../../../c/open62541/src/ && \
	cd $(CURDIR) && \
	rm -rf $(SAFESTRING_DIR)

build: prebuild
	@echo "Building the open62541 wrapper library open62541W.so.."
	cd $(PYXPATH) && make && cp open62541W.so .. && cd $(CURDIR)

pub: build
	@echo "Starting the opcua server and start publishing the data.."
	python3.6 DataBusTest.py -direction PUB -endpoint opcua://$(HOST):$(PORT) -ns StreamManager \
							-topic classifier_results \
							-certFile $(SERVER_CERTS)/opcua_server_certificate.der \
							-privateFile $(SERVER_CERTS)/opcua_server_key.der \
							-trustFile $(CA_CERTS)/ca_certificate.der

sub: build
	@echo "Establish connection to opcua server and start subscribing the data.."
	python3.6 DataBusTest.py -direction SUB -endpoint opcua://$(HOST):$(PORT) -ns StreamManager \
							-topic classifier_results \
							-certFile $(CLIENT_CERTS)/opcua_client_certificate.der \
							-privateFile $(CLIENT_CERTS)/opcua_client_key.der \
							-trustFile $(CA_CERTS)/ca_certificate.der

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
	rm -rf $(CURDIR)/safestringlib
