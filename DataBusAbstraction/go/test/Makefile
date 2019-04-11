SHELL := /bin/bash
CURDIR = $(shell pwd)
SAFESTRING_DIR = safestringlib
SAFESTRING_VER = 77b772849eda2321fb0dca56a321e3939930d7b9
DBA_PATH = ../../../DataBusAbstraction
CERT_DIR_PATH = $(DBA_PATH)/cert-tool/Certificates
CPATH = ../../c/open62541/src
CFLAGS = -std=c99
INCLUDE = -I../include -I../../
SERVER_CERTS = /etc/ssl/opcua
CLIENT_CERTS = /etc/ssl/opcua
CA_CERTS = /etc/ssl/ca
INVALID_CERTS = ../../c/open62541/src/test/invalid_certs

define build_dependencies
	@echo "Building the open62541 wrapper library libopen62541W.a.."
	cd $(CPATH) && gcc $(CFLAGS) $(INCLUDE) -c ../../DataBus.c open62541_wrappers.c && gcc $(CFLAGS) $(INCLUDE) -c open62541.c && \
	ar crU libopen62541W.a DataBus.o open62541_wrappers.o open62541.o && ar crU libsafestring.a DataBus.o open62541_wrappers.o open62541.o && \
	cp -rf libsafestring.a ../../../../Util/cpuid/ && \
	rm -rf DataBus.o open62541_wrappers.o open62541.o && \
	pwd
endef

build_safestring_lib:
	@echo "Clone the safestringlib if not cloned already..."
	git clone https://github.com/intel/safestringlib.git && \
	cd $(SAFESTRING_DIR) && \
	git checkout $(SAFESTRING_VER) && \
	make && \
	cp -rf libsafestring.a $(CURDIR)/../ && \
	cd $(CURDIR) && \
	rm -rf $(SAFESTRING_DIR)

build:
	$(call build_dependencies)
	@echo "Building the DataBusTest.go file"
	go build DataBusTest.go

build_lib_for_docker:
	$(call build_dependencies)

pub: build
	@echo "Starting the secure opcua server and start publishing the data.."
	./DataBusTest -direction PUB -endpoint opcua://localhost:65003 -ns StreamManager \
				  -topic classifier_results \
				  -certFile $(SERVER_CERTS)/opcua_server_certificate.der \
			      -privateFile $(SERVER_CERTS)/opcua_server_key.der \
				  -trustFile $(CA_CERTS)/ca_certificate.der

sub: build
	@echo "Establish connection to opcua secure server and start subscribing the data.."
	./DataBusTest -direction SUB -endpoint opcua://localhost:65003 -ns StreamManager \
				  -topic classifier_results \
				  -certFile $(CLIENT_CERTS)/opcua_client_certificate.der \
	              -privateFile $(CLIENT_CERTS)/opcua_client_key.der \
		          -trustFile $(CA_CERTS)/ca_certificate.der

pub_insecure: build
	@echo "Starting the insecure opcua server and start publishing the data.."
	./DataBusTest -direction PUB -endpoint opcua://localhost:65003 -ns StreamManager \
				  -topic classifier_results \
				  -certFile "" -privateFile "" -trustFile ""

sub_insecure: build
	@echo "Establish connection to insecure opcua server and start subscribing the data.."
	./DataBusTest -direction SUB -endpoint opcua://localhost:65003 -ns StreamManager \
				  -topic classifier_results \
				  -certFile "" -privateFile "" -trustFile ""

invalid_sub: build
	@echo "Establish connection to opcua server with invalid cert and start subscribing the data.."
	./DataBusTest -direction SUB -endpoint opcua://localhost:4840 -ns StreamManager \
				  -topic classifier_results \
				  -certFile $(CLIENT_CERTS)/opcua_client_certificate.der \
	              -privateFile $(CLIENT_CERTS)/opcua_client_key.der \
		          -trustFile $(INVALID_CERTS)/ca/ca_certificate.der

clean:
	@echo "Removing all the binary files..."
	rm -rf DataBusTest cgo $(CPATH)/open62541_wrappers.o $(CPATH)/open62541.o $(CPATH)/libopen62541W.a
	rm -rf $(CURDIR)/safestringlib $(CURDIR)/*.a $(CURDIR)/*.o
