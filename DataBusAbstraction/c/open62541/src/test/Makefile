CPATH = ..
CFLAGS = -std=c99
INCLUDE = -I../../include 
SERVER_CERTS = /etc/ssl/opcua
CLIENT_CERTS = /etc/ssl/opcua
CA_CERTS = /etc/ssl/grpc_internal
INVALID_CERTS = invalid_certs

build: clean
	@clear
	@echo "Building the open62541 test file.."
	gcc $(CFLAGS) $(INCLUDE) open62541_test.c \
	$(CPATH)/open62541_wrappers.c $(CPATH)/open62541.c -lmbedtls -lmbedx509 -lmbedcrypto -pthread \
	-o open62541_test && pwd

server: build
	@echo "Start server, publish and destroy..."
	./open62541_test server $(SERVER_CERTS)/opcua_server_certificate.der \
	                        $(SERVER_CERTS)/opcua_server_key.der \
							$(CA_CERTS)/ca_certificate.der
client: build
	@echo "Start client, subscribe and destroy..."
	./open62541_test client $(CLIENT_CERTS)/opcua_client_certificate.der \
	                        $(CLIENT_CERTS)/opcua_client_key.der \
							$(CA_CERTS)/ca_certificate.der

invalid_client: build
	@echo "Establish connection to opcua server and start subscribing the data with invalid client certs.."
	./open62541_test client $(INVALID_CERTS)/client/client2_cert.der \
	                        $(INVALID_CERTS)/client/client2_key.der \
							$(INVALID_CERTS)/ca/ca_cert.der

clean:
	@echo "Removing all the binary files..."
	rm -rf open62541_test $(CPATH)/open62541_wrappers.o $(CPATH)/open62541.o
