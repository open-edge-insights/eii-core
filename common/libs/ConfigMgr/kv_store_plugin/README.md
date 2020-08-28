### EIS KV Store Abstraction

EIS KVStorePlugin provides an abstraction over underlying Key-Value datastores(eg. etcd)

## Dependency Installation

The EIS KVStorePlugin depends on CMake version 3.11+.For Ubuntu 18.04 this is not
the default version installed via `apt-get`. To install the correct version
of CMake, execute the following commands:

```sh
# Remove old CMake version
$ sudo apt -y purge cmake
$ sudo apt -y autoremove

# Download CMake
$ wget https://cmake.org/files/v3.15/cmake-3.15.0-Linux-x86_64.sh

# Installation CMake
$ sudo mkdir /opt/cmake
$ sudo cmake-3.15.0-Linux-x86_64.sh --prefix=/opt/cmake --skip-license

# Make the command available to all users
$ sudo update-alternatives --install /usr/bin/cmake cmake /opt/cmake/bin/cmake 1 --force
```

To install the remaining dependencies for the message bus execute the following
command:

```sh
$ sudo -E ./install.sh
```

## Compilation

The EIS KVStorePlugin utilizes CMake as the build tool for compiling the library.
The simplest sequence of commands for building the library are shown below.

```sh
$ mkdir build
$ cd build
$ cmake ..
$ make
$ sudo make install
```

To compile the KVStorePlugin C examples in addition to the library, then you can set the
the `WITH_EXAMPLES` to `ON` when executing the `cmake` command (as shown
below).

```sh
$ cmake -DWITH_EXAMPLES=ON ..
```

To compile the KVStorePlugin C examples in addition to the library, then you can set the
the `WITH_EXAMPLES` to `ON` when executing the `cmake` command (as shown
below).

```sh
$ cmake -DWITH_TESTS=ON ..
```

## Running examples
> **NOTE:** The unit tests will only be compiled if the `WITH_EXAMPLES=ON` option
> is specified when running CMake.

```sh
$ cd examples
$ ./kv_store_etcd ./etcd_kv_store_config.json
```

## Running Unit Tests
> **NOTE:** The unit tests will only be compiled if the `WITH_TESTS=ON` option
> is specified when running CMake.

```sh
$ cd tests
$ ./kv_store_client_tests ./kv_store_unittest_config.json
```