CURDIR = `pwd`
PYXPATH = ../open62541
SERVER_CERTS = ../../../Certificates/server
CLIENT_CERTS = ../../../Certificates/client
CA_CERTS = ../../../Certificates/ca
INVALID_CERTS = ../../c/open62541/src/test/invalid_certs
HOST = localhost
PORT = 4840

build: clean
	
	@echo "Building the open62541 wrapper library open62541W.so.."
	cd $(PYXPATH) && make && cp open62541W.so .. && cd $(CURDIR)

pub: build
	@echo "Starting the opcua server and start publishing the data.."
	python3.6 DataBusTest.py -direction PUB -endpoint opcua://$(HOST):$(PORT) -ns StreamManager \
				  -topic classifier_results \
				  -certFile $(SERVER_CERTS)/server_cert.der \
		  	          -privateFile $(SERVER_CERTS)/server_key.der \
				  -trustFile $(CA_CERTS)/ca_cert.der

sub: build
	@echo "Establish connection to opcua server and start subscribing the data.."
	python3.6 DataBusTest.py -direction SUB -endpoint opcua://$(HOST):$(PORT) -ns StreamManager \
				  -topic classifier_results \
				  -certFile $(CLIENT_CERTS)/client0_cert.der \
	                          -privateFile $(CLIENT_CERTS)/client0_key.der \
		                  -trustFile $(CA_CERTS)/ca_cert.der

invalid_sub: build
	@echo "Establish connection to opcua server with invalid cert and start subscribing the data.."
	python3.6 DataBusTest.py -direction SUB -endpoint opcua://$(HOST):$(PORT) -ns StreamManager \
				  -topic classifier_results \
				  -certFile $(CLIENT_CERTS)/client0_cert.der \
  	                          -privateFile $(CLIENT_CERTS)/client0_key.der \
        		          -trustFile $(INVALID_CERTS)/ca/ca_cert.der

clean:
	@echo "Removing all the binary files..."
	rm -rf open62541W.so $(PYXPATH)/open62541W.so open62541W.c build c
