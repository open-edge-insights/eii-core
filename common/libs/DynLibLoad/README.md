Dynamic Library Loader
======================

C library for loading shared libraries at runtime.

## Dependency Installation

The dynamic library loading library depends on the libraries listed below:

* [EISUtils](../../util/c/README.md)
* [IntelSafeString](../../IntelSafeString/README.md)

Please see the documentation for those libraries to install them.

## Compilation

The dynamic shared object loading library utilizes the CMake build system. To
build just the library, execute the following commands:

```sh
$ mkdir build
$ cmake ..
$ make
```

To build the library in debug mode, execute the following CMake command in place
of the one above:

```sh
$ cmake -DCMAKE_BUILD_TYPE=Debug ..
```

## Installation

After running the compilation steps above, install the library with the command
below:

```sh
$ sudo make install
```

By default, CMake will install the library into the `/usr/local/lib` directory.
Ubuntu does not include that in the default `LD_LIBRARY_PATH` environmental variable.
To include it, export the following variable:

```sh
$ export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib
```

## Running Example

The Dynamic Loader Library comes with an example usage of the library. To build
and run this example, add the `WITH_EXAMPLES` flag to your CMake command (as
shown below).

```sh
$ cmake -DWITH_EXAMPLES=ON ..
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
$ cmake -DWITH_TESTS=ON ..
```

Then, go into the `tests/` directory inside of your build directory and run
the command below:

```sh
$ ./dynlibload-tests
```
