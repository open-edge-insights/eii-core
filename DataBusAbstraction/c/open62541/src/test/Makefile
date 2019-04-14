CURDIR = $(shell pwd)
SAFESTRING_DIR = safestringlib
SAFESTRING_VER = 77b772849eda2321fb0dca56a321e3939930d7b9
CPATH = ..
CFLAGS = -std=c99
INCLUDE = -I../../include
SERVER_CERTS = /etc/ssl/opcua
CLIENT_CERTS = /etc/ssl/opcua
CA_CERTS = /etc/ssl/ca
INVALID_CERTS = invalid_certs

prebuild: clean
	@echo "Clone the safestringlib if not cloned already..."
	git clone https://github.com/intel/safestringlib.git && \
	cd $(SAFESTRING_DIR) && \
	git checkout $(SAFESTRING_VER) && \
	make && \
	cp -f libsafestring.a ../../ && \
	cp -f libsafestring.a ../../../../../go/ && \
	cp -f libsafestring.a ../ && \
	cd $(CURDIR) && \
	rm -rf $(SAFESTRING_DIR)

build: prebuild
	@echo "Building the open62541 test file.."
	gcc $(CFLAGS) $(INCLUDE) open62541_test.c \
	$(CPATH)/open62541_wrappers.c $(CPATH)/open62541.c -L. -lsafestring -lmbedtls -lmbedx509 -lmbedcrypto -pthread \
	-o open62541_test && pwd

pub: build
	@echo "Start secure server, publish and destroy..."
	./open62541_test server $(SERVER_CERTS)/opcua_server_certificate.der \
	                        $(SERVER_CERTS)/opcua_server_key.der \
							$(CA_CERTS)/ca_certificate.der
sub: build
	@echo "Start secure client, subscribe and destroy..."
	./open62541_test client $(CLIENT_CERTS)/opcua_client_certificate.der \
	                        $(CLIENT_CERTS)/opcua_client_key.der \
							$(CA_CERTS)/ca_certificate.der

pub_insecure: build
	@echo "Start insecure server, publish and destroy..."
	./open62541_test server "" "" ""

sub_insecure: build
	@echo "Start insecure client, subscribe and destroy..."
	./open62541_test client "" "" ""

invalid_client: build
	@echo "Establish connection to opcua server and start subscribing the data with invalid client certs.."
	./open62541_test client $(INVALID_CERTS)/client/client2_cert.der \
	                        $(INVALID_CERTS)/client/client2_key.der \
							$(INVALID_CERTS)/ca/ca_certificate.der

build_static_lib: prebuild
	@echo "Building the open62541 so file.."
	gcc -c -o open62541_wrappers.o $(INCLUDE) $(CPATH)/open62541_wrappers.c
	ar rcs libopen62541_wrappers.a open62541_wrappers.o
	gcc -c -o open62541.o $(INCLUDE) $(CPATH)/open62541.c
	ar rcs libopen62541.a open62541.o

clean:
	@echo "Removing all the binary files..."
	rm -rf open62541_test libsafestring.a $(CPATH)/open62541_wrappers.o $(CPATH)/open62541.o $(CURDIR)/safestringlib
