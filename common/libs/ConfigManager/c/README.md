# ConfigManager

ConfigManager provides C APIs for distributed key-value store like `etcd` to read EIS related config details.

## Compilation
The EIS Config Manager utilizes CMake as the build tool for compiling the C/C++
library. The simplest sequence of commands for building the library are
shown below.

```sh
$ ./prebuild.sh
$ cd build
$ cmake ..
$ make
```

If you wish to compile the EIS Config Manager along with C examples
```sh
$ cmake -DWITH_EXAMPLES=ON ..
```


If you wish to compile the EIS Config Manager in debug mode, then you can set
the `CMAKE_BUILD_TYPE` to `Debug` when executing the `cmake` command (as shown
below).

```sh
$ cmake -DCMAKE_BUILD_TYPE=Debug ..
```
## Installation

If you wish to install the EIS Config Manager on your system, execute the
following command after building the library:

```sh
$ sudo make install
```

By default, this command will install the EIS Config Manager C library into
`/usr/local/lib/`. On some platforms this is not included in the `LD_LIBRARY_PATH`
by default. As a result, you must add this directory to you `LD_LIBRARY_PATH`,
otherwise you will encounter issues using the EIS Message Bus. This can
be accomplished with the following `export`:

```sh
$ export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib/
```
> **NOTE:** You can also specify a different library prefix to CMake through
> the `CMAKE_INSTALL_PREFIX` flag.

## C APIs

1. **char\* init(char \*storage_type, char \*ca_cert, char \*cert_file, char \*key_file)**    
   ```
   init function to initialize config manager
   :param storage_type      - Type of key-value storage, Eg. etcd
   :param ca_cert           - config manager client ca cert
   :param cert_file         - config manager client cert file
   :param key_file          - config manager client key file
   :return char*            - on success of initialization "0", on failure "-1"
   ```

2. **char\* get_config(char \*key)**
    ```
    get_config function gets the value of a key from config manager
    :param key      - key to be queried on from config manager
    :return char*   - values returned from config manager based on key
    ```

3. **typedef void (\*callback_fcn)(char \*key, char \*value)**
    ```
    callback_fcn is a user callback function will be called on trigger of watch event
    :param key       - key on which watch event triggered
    :param value     - updated value of corrsponding key
    ```
4. **void register_watch_dir(char \*key, callback_fcn user_callback)**
    ```
    register_watch_dir function registers to a callback and keeps a watch on the prefix of a specified key

    :param key       - prefix of a key to keep a watch on
    ```
5. **void register_watch_key(char \*key, callback_fcn user_callback)**
    ```
    register_watch_key function registers to a callback and keeps a watch on a specified key
    :param key      - key to keep a watch on
    ```

To refer C examples follow [examples/](examples/)
