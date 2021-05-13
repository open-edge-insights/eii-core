Dynamic Library Loader
======================

C library for loading shared libraries at runtime.

## Dependency Installation

The dynamic library loading library depends on the libraries listed below:

* [EIIUtils](../../util/c/README.md)
* [IntelSafeString](../../IntelSafeString/README.md)

Please see the documentation for those libraries to install them.

## Compilation

The dynamic shared object loading library utilizes the CMake build system. 

CMAKE_INSTALL_PREFIX needs to be set for the build and installation:

```sh
    $ export CMAKE_INSTALL_PREFIX="/opt/intel/eii"
```

To build just the library, execute the following commands:

```sh
$ mkdir build
$ cmake -DCMAKE_INSTALL_INCLUDEDIR=$CMAKE_INSTALL_PREFIX/include -DCMAKE_INSTALL_PREFIX=$CMAKE_INSTALL_PREFIX ..
$ make
```

To build the library in debug mode, execute the following CMake command in place
of the one above:

```sh
$ cmake -DCMAKE_INSTALL_INCLUDEDIR=$CMAKE_INSTALL_PREFIX/include -DCMAKE_INSTALL_PREFIX=$CMAKE_INSTALL_PREFIX -DCMAKE_BUILD_TYPE=Debug ..
```

## Installation

After running the compilation steps above, install the library with the command
below:

```sh
$ sudo make install
```

By default, CMake will install the library into the `/opt/intel/eii/lib/` directory.
Ubuntu does not include that in the default `LD_LIBRARY_PATH` environmental variable.
To include it, export the following variable:

```sh
$ export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/opt/intel/eii/lib/
```

>**NOTE:** You can also specify a different library prefix to CMake through
the `CMAKE_INSTALL_PREFIX` flag. If different installation path is given via `CMAKE_INSTALL_PREFIX`, then `$LD_LIBRARY_PATH` should be appended by $CMAKE_INSTALL_PREFIX/lib.

## Running Example

The Dynamic Loader Library comes with an example usage of the library. To build
and run this example, add the `WITH_EXAMPLES` flag to your CMake command (as
shown below).

```sh
$ cmake -DCMAKE_INSTALL_INCLUDEDIR=$CMAKE_INSTALL_PREFIX/include -DCMAKE_INSTALL_PREFIX=$CMAKE_INSTALL_PREFIX -DWITH_EXAMPLES=ON ..
```

Once you have done this, go into the `examples/` directory inside of your build
directory. Then, execute the following commands

```sh
# This script updates your environmental varaible
$ source ./source.sh

# Run the example
$ ./load-example
```

The example will load a shared object and a function symbol from the library.
It then calls the function which will print to `stdout`.

## Running Unit Tests

To run the unit tests, use the following CMake command:

```sh
$ cmake -DCMAKE_INSTALL_INCLUDEDIR=$CMAKE_INSTALL_PREFIX/include -DCMAKE_INSTALL_PREFIX=$CMAKE_INSTALL_PREFIX -DWITH_TESTS=ON ..
```

Then, go into the `tests/` directory inside of your build directory and run
the command below:

```sh
$ ./dynlibload-tests
```
