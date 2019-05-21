SHELL := /bin/bash
CURDIR = $(shell pwd)
SAFESTRING_DIR = safestringlib
SAFESTRING_VER = 77b772849eda2321fb0dca56a321e3939930d7b9
CPATH = ..
CFLAGS = -std=c99 -g
INCLUDE = -I $(CPATH)/open62541/include
SOURCE_FILE = $(CPATH)/open62541/src
SERVER_CERTS = /etc/ssl/opcua
CLIENT_CERTS = /etc/ssl/opcua
CA_CERTS = /etc/ssl/ca
WRAPPERS_PATH = $(SOURCE_FILE)/open62541_wrappers.c
TOPICS = cam_serial1_results,cam_serial2_results
HOST = localhost
PORT = 65003
MAX_SAFESTRING_SIZE = 60

build_safestring_lib:
	echo "Clone the safestringlib if not cloned already..."
	git clone https://github.com/intel/safestringlib.git && \
	cd $(SAFESTRING_DIR) && \
	git checkout $(SAFESTRING_VER) && \
	cd include && \
	sed -i "s|4|$(MAX_SAFESTRING_SIZE)|g" "safe_str_lib.h" && \
	cd .. && \
	make && \
	cp -rf libsafestring.a $(CURDIR) && \
	cd $(CURDIR) && \
	rm -rf $(SAFESTRING_DIR)

build_static_lib:
	if [ -a $(WRAPPERS_PATH) ] ; \
	then \
		echo "Building the open62541_wrappers static lib file.." ; \
		gcc -c $(INCLUDE) $(SOURCE_FILE)/open62541_wrappers.c $(SOURCE_FILE)/open62541.c ; \
		ar rcs libopen62541_wrappers.a open62541_wrappers.o open62541.o ; \
		rm -rf *.o ; \
	fi

build: build_static_lib
	@echo "Building the DataBus_test.c file.."
	gcc $(CFLAGS) $(INCLUDE) -I $(CPATH) DataBus_test.c \
	$(CPATH)/DataBus.c -L. -lopen62541_wrappers -lsafestring -lmbedtls -lmbedx509 -lmbedcrypto -pthread \
	-o DataBus_test && pwd

pub: build
	@echo "Start secure server, publish and destroy..."
	./DataBus_test PUB opcua://$(HOST):$(PORT) streammanager \
				   $(TOPICS) \
				   $(SERVER_CERTS)/opcua_server_certificate.der \
				   $(SERVER_CERTS)/opcua_server_key.der \
				   $(CA_CERTS)/ca_certificate.der
sub: build
	@echo "Start secure client, subscribe and destroy..."
	./DataBus_test SUB opcua://$(HOST):$(PORT) streammanager \
				   $(TOPICS) \
				   $(CLIENT_CERTS)/opcua_client_certificate.der \
	               $(CLIENT_CERTS)/opcua_client_key.der \
				   $(CA_CERTS)/ca_certificate.der

pub_insecure: build
	@echo "Start insecure server, publish and destroy..."
	./DataBus_test PUB opcua://$(HOST):$(PORT) streammanager $(TOPICS) "" "" ""

sub_insecure: build
	@echo "Start insecure client, subscribe and destroy..."
	./DataBus_test SUB opcua://$(HOST):$(PORT) streammanager $(TOPICS) "" "" ""

clean:
	@echo "Removing all the binary files..."
	rm -rf DataBus_test libsafestring.a $(CPATH)/open62541_wrappers.o $(CPATH)/open62541.o $(CURDIR)/safestringlib \
	       $(CURDIR)/*.a $(CURDIR)/*.o $(CURDIR)/../*.a


