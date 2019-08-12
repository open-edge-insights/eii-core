SHELL := /bin/bash
CPATH = ..
CFLAGS = -g
INCLUDE = -I../../ -I../../open62541/include/ -I/usr/include/ -I.
LIBS = -lstdc++ -L../ -lopen62541_wrappers -L/usr/lib -lgtest -lgtest_main -lpthread

install:
	echo "Installing libgtest-dev..." && \
	sudo apt-get install -y libgtest-dev && \
	sudo apt-get install -y cmake && \
	cd /usr/src/gtest && \
	sudo cmake CMakeLists.txt && \
	sudo make && \
	sudo cp *.a /usr/lib && \
	cd -

build: build_unit_tests build_integration_tests build_integration_tests2


build_unit_tests:
	@echo "Building the UnitTests_DBA.cpp file.."
	gcc -c CommonTestUtils.cpp ${INCLUDE}
	gcc -c -o ../../DataBus.o ../../DataBus.c ${INCLUDE}
	gcc ${CFLAGS} -o UnitTests_DBA UnitTests_DBA.cpp ../../DataBus.o CommonTestUtils.o \
	${INCLUDE} -L../ -lopen62541_wrappers -L/usr/lib -lgtest -lgtest_main -lpthread \
	-lstdc++ -lsafestring -lmbedtls -lmbedx509 -lmbedcrypto

build_integration_tests:
	@echo "Building the IntegrationTests_DBA.cpp file.."
	gcc -c CommonTestUtils.cpp ${INCLUDE}
	gcc -c -o ../../DataBus.o ../../DataBus.c ${INCLUDE}
	gcc ${CFLAGS} -o IntegrationTests_DBA IntegrationTests_DBA.cpp ../../DataBus.o CommonTestUtils.o \
	${INCLUDE} -L../ -lopen62541_wrappers -L/usr/lib -lgtest -lgtest_main -lpthread \
	-lstdc++ -lsafestring -lmbedtls -lmbedx509 -lmbedcrypto

build_integration_tests2:
	@echo "Building the UnitTests_DBA2.cpp file.."
	gcc -c CommonTestUtils.cpp ${INCLUDE}
	gcc -c -o ../../DataBus.o ../../DataBus.c ${INCLUDE}
	gcc ${CFLAGS} -o IntegrationTests_DBA2 IntegrationTests_DBA2.cpp ../../DataBus.o CommonTestUtils.o \
        ${INCLUDE} -L../ -lopen62541_wrappers -L/usr/lib -lgtest -lgtest_main -lpthread \
        -lstdc++ -lsafestring -lmbedtls -lmbedx509 -lmbedcrypto

clean:
	@echo "Removing all the binary files..."
	rm -f UnitTests_DBA IntegrationTests_DBA
