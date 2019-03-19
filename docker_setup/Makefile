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
install:
	@echo "***********Building and Installing IEI ***********"
	./compose_startup.sh
	./create_client_dist_package.sh
	./setup_iei.py -s
install-registry:
	@echo "***********Building and Installing IEI using DOCKER_REGISTRY ***********"
	./registry.sh pull $(DOCKER_REGISTRY)
	./compose_startup.sh --up
	./create_client_dist_package.sh
	./setup_iei.py -s
uninstall:
	./compose_startup.sh --down
	./setup_iei.py -u

