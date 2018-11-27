CPATH = ../../c/open62541/src
CFLAGS = -std=c99
INCLUDE = -I../include 
SERVER_CERTS = ../../../Certificates/server
CLIENT_CERTS = ../../../Certificates/client
CA_CERTS = ../../../Certificates/ca
INVALID_CERTS = ../../c/open62541/src/test/invalid_certs

build_lib: clean
	
	@echo "Building the open62541 wrapper library libopen62541W.a.."
	cd $(CPATH) && gcc $(CFLAGS) $(INCLUDE) -c open62541_wrappers.c && gcc $(CFLAGS) $(INCLUDE) -c open62541.c && \
	ar crU libopen62541W.a open62541_wrappers.o open62541.o && rm -rf open62541_wrappers.o open62541.o \
	&& pwd

build: clean build_lib
	
	@echo "Building the DataBusTest.go file"
	go build DataBusTest.go

pub: build
	@echo "Starting the opcua server and start publishing the data.."
	./DataBusTest -direction PUB -endpoint opcua://localhost:65003 -ns StreamManager \
				  -topic classifier_results \
				  -certFile $(SERVER_CERTS)/server_cert.der \
			      -privateFile $(SERVER_CERTS)/server_key.der \
				  -trustFile $(CA_CERTS)/ca_cert.der

sub: build
	@echo "Establish connection to opcua server and start subscribing the data.."
	./DataBusTest -direction SUB -endpoint opcua://localhost:65003 -ns StreamManager \
				  -topic classifier_results \
				  -certFile $(CLIENT_CERTS)/client0_cert.der \
	              -privateFile $(CLIENT_CERTS)/client0_key.der \
		          -trustFile $(CA_CERTS)/ca_cert.der

invalid_sub: build
	@echo "Establish connection to opcua server with invalid cert and start subscribing the data.."
	./DataBusTest -direction SUB -endpoint opcua://localhost:4840 -ns StreamManager \
				  -topic classifier_results \
				  -certFile $(CLIENT_CERTS)/client0_cert.der \
	              -privateFile $(CLIENT_CERTS)/client0_key.der \
		          -trustFile $(INVALID_CERTS)/ca/ca_cert.der

clean:
	@echo "Removing all the binary files..."
	rm -rf DataBusTest cgo $(CPATH)/open62541_wrappers.o $(CPATH)/open62541.o $(CPATH)/libopen62541W.a
