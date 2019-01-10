CPATH = ../../c/open62541/src
CFLAGS = -std=c99
INCLUDE = -I../include
SERVER_CERTS = /etc/ssl/opcua
CLIENT_CERTS = /etc/ssl/opcua
CA_CERTS = /etc/ssl/grpc_internal
INVALID_CERTS = ../../c/open62541/src/test/invalid_certs

build_lib: clean prebuild

	@echo "Building the open62541 wrapper library libopen62541W.a.."
	cd $(CPATH) && gcc $(CFLAGS) $(INCLUDE) -c open62541_wrappers.c && gcc $(CFLAGS) $(INCLUDE) -c open62541.c && \
	ar crU libopen62541W.a open62541_wrappers.o open62541.o && ar crU libsafestring.a open62541_wrappers.o open62541.o && \
	rm -rf open62541_wrappers.o open62541.o && \
	pwd

prebuild:
	git clone https://github.com/intel/safestringlib.git && \
	cd safestringlib/ && \
	make && \
	cp -rf libsafestring.a ../ && \
	cp -rf libsafestring.a ../../ && \
	cp -rf libsafestring.a ../../../c/open62541/src/

build: clean build_lib

	@echo "Building the DataBusTest.go file"
	go build DataBusTest.go

pub: build
	@echo "Starting the opcua server and start publishing the data.."
	./DataBusTest -direction PUB -endpoint opcua://localhost:65003 -ns StreamManager \
				  -topic classifier_results \
				  -certFile $(SERVER_CERTS)/opcua_server_certificate.der \
			      -privateFile $(SERVER_CERTS)/opcua_server_key.der \
				  -trustFile $(CA_CERTS)/ca_certificate.der

sub: build
	@echo "Establish connection to opcua server and start subscribing the data.."
	./DataBusTest -direction SUB -endpoint opcua://localhost:65003 -ns StreamManager \
				  -topic classifier_results \
				  -certFile $(CLIENT_CERTS)/opcua_client_certificate.der \
	              -privateFile $(CLIENT_CERTS)/opcua_client_key.der \
		          -trustFile $(CA_CERTS)/ca_certificate.der

invalid_sub: build
	@echo "Establish connection to opcua server with invalid cert and start subscribing the data.."
	./DataBusTest -direction SUB -endpoint opcua://localhost:4840 -ns StreamManager \
				  -topic classifier_results \
				  -certFile $(CLIENT_CERTS)/opcua_client_certificate.der \
	              -privateFile $(CLIENT_CERTS)/opcua_client_key.der \
		          -trustFile $(INVALID_CERTS)/ca/ca_certificate.der

clean:
	@echo "Removing all the binary files..."
	rm -rf DataBusTest cgo $(CPATH)/open62541_wrappers.o $(CPATH)/open62541.o $(CPATH)/libopen62541W.a safestringlib libsafestring.a
