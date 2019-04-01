SHELL := /bin/bash
provision:
	./provision_startup.sh $(CERT_PATH)
build:
	./compose_startup.sh --build	
push: 
	./registry.sh push $(DOCKER_REGISTRY) 
pull: 
	./registry.sh pull $(DOCKER_REGISTRY)
run:
	./compose_startup.sh --up
down:
	./compose_startup.sh --down
uninstall:
	./setup_iei.py -u
distlibs:
	./create_client_dist_package.sh
install: uninstall provision
	@echo "***********Building and Installing IEI ***********"
	./compose_startup.sh --build
	./create_client_dist_package.sh
	cp -r . /opt/intel/iei/
	./setup_iei.py -s
install-registry: uninstall provision
	@echo "***********Building and Installing IEI using DOCKER_REGISTRY ***********"
	./registry.sh pull $(DOCKER_REGISTRY)
	./create_client_dist_package.sh
	cp -r . /opt/intel/iei/
	./setup_iei.py -s