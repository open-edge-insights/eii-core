CURDIR = `pwd`
PYXPATH = ../open62541
SERVER_CERTS = /etc/ssl/opcua
CLIENT_CERTS = /etc/ssl/opcua
CA_CERTS = /etc/ssl/grpc_internal
INVALID_CERTS = ../../c/open62541/src/test/invalid_certs
HOST = localhost
PORT = 65003

build: clean
	
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
        		          -trustFile $(INVALID_CERTS)/ca/ca_cert.der

clean:
	@echo "Removing all the binary files..."
	rm -rf open62541W.so $(PYXPATH)/open62541W.so open62541W.c build c
