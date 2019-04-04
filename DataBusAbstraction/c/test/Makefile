CURDIR = $(shell pwd)
SAFESTRING_DIR = safestringlib
SAFESTRING_VER = 77b772849eda2321fb0dca56a321e3939930d7b9
CPATH = ..
CFLAGS = -std=c99
INCLUDE = -I $(CPATH)/open62541/include
SOURCE_FILE = $(CPATH)/open62541/src
SERVER_CERTS = /etc/ssl/opcua
CLIENT_CERTS = /etc/ssl/opcua
CA_CERTS = /etc/ssl/ca

prebuild: clean
	@echo "Clone the safestringlib if not cloned already..."
	git clone https://github.com/intel/safestringlib.git && \
	cd $(SAFESTRING_DIR) && \
	git checkout $(SAFESTRING_VER) && \
	make && \
	cp -f libsafestring.a ../../ && \
	cp -f libsafestring.a ../../../go/ && \
	cp -f libsafestring.a ../ && \
	cd $(CURDIR) && \
	rm -rf $(SAFESTRING_DIR)

build: prebuild
	@echo "Building the open62541 test file.."
	gcc $(CFLAGS) $(INCLUDE) -I $(CPATH) DataBus_test.c \
	$(CPATH)/DataBus.c $(SOURCE_FILE)/open62541_wrappers.c $(SOURCE_FILE)/open62541.c -L. -lsafestring -lmbedtls -lmbedx509 -lmbedcrypto -pthread \
	-o DataBus_test && pwd

pub: build
	@echo "Start server, publish and destroy..."
	./DataBus_test PUB opcua://localhost:65003 streammanager classifier_results \
							$(SERVER_CERTS)/opcua_server_certificate.der \
	                    	$(SERVER_CERTS)/opcua_server_key.der \
							$(CA_CERTS)/ca_certificate.der
sub: build
	@echo "Start client, subscribe and destroy..."
	./DataBus_test SUB opcua://localhost:65003 streammanager classifier_results \
							$(CLIENT_CERTS)/opcua_client_certificate.der \
	                        $(CLIENT_CERTS)/opcua_client_key.der \
							$(CA_CERTS)/ca_certificate.der

clean:
	@echo "Removing all the binary files..."
	rm -rf DataBus_test libsafestring.a $(CPATH)/open62541_wrappers.o $(CPATH)/open62541.o $(CURDIR)/safestringlib

