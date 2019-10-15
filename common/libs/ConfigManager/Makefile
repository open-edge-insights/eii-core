build:
	cd go/cgo && \
	go build -buildmode=c-shared -o config_manager.so && \
	mv config_manager.h ../../include/eis/config_manager/
	rm -rf build && mkdir build && pwd
	mv go/cgo/config_manager.so build/libconfigmanager.so

run_test: build
	cd tests/ && \
	g++ -I ../include/eis/config_manager/ test.cpp ../src/config_manager_wrappers.cpp ../build/libconfigmanager.so -o test && \
	./test

clean:
	rm -rf build include/eis/config_manager/config_manager.h
