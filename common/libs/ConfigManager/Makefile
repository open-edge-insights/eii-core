build:
	cd go/cgo && \
	go build -buildmode=c-shared -o config_manager.so && \
	mv config_manager.h ../../include/eis/config_manager/
	rm -rf build && mkdir build && pwd
	mv go/cgo/config_manager.so build/libconfigmanager.so

run_test: build
	cd examples/ && \
	gcc -I ../include config_manager.c ../src/config_manager_wrappers.c ../build/libconfigmanager.so -o test && \
	./test

clean:
	rm -rf build include/eis/config_manager/config_manager.h
