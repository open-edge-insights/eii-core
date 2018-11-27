CPATH = ../../c/open62541/src

default: build

build: clean
	@clear
	@echo "Creating open62541W.so file..."
	python3.6 setup.py build_ext -i

	mv open62541W.cpython*.so open62541W.so

server: build
	@echo "Start server, publish and destroy..."
	python3.6 test.py -s server

client: build
	@echo "Start client, subscribe and destroy..."
	python3.6 test.py -c client

clean:
	@echo "Removing all the build files..."
	rm -rf build c *.so *.c
