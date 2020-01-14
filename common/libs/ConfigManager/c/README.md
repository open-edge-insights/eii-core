# ConfigManager

ConfigManager provides C APIs for:
- distributed key-value store like `etcd` to read and watch on EIS related config details.
- To read EIS environment info needed to build EIS MessageBus config

## Prerequisites

To install the dependencies for the ConfigManager execute the following command:

```sh
$ sudo -E ./install.sh
```

ConfigManager also has dependency on EISUtils library. Follow the documentation of EISUtils to install it.
* [IntelSafeString](../../IntelSafeString/README.md)
* [EISUtils](../../../util/c/README.md)

## Compilation
The EIS Config Manager utilizes CMake as the build tool for compiling the C/C++
library. The simplest sequence of commands for building the library are
shown below.

```sh
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
otherwise you will encounter issues using the Config Manager. This can
be accomplished with the following `export`:

```sh
$ export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib/
```
> **NOTE:** You can also specify a different library prefix to CMake through
> the `CMAKE_INSTALL_PREFIX` flag.

## Running Unit Tests

> **NOTE:** The unit tests will only be compiled if the `WITH_TESTS=ON` option
> is specified when running CMake.

Run the following commands from the `build/tests` folder to cover the unit
tests. Make sure etcd daemon should be running, `etcdctl` is available in the folder `build/tests` and install `etcdctl` by following [https://github.com/etcd-io/etcd/releases](https://github.com/etcd-io/etcd/releases).

```sh
$ export ETCDCTL_API=3
$ ./configmgr-tests
$ ./env-config-tests
```

## C APIs

### APIs for reading from distributed key-value store

1. Creating a new config manager client

    `config_mgr_t* config_mgr_client = config_mgr_new(char *storage_type, char *cert_file, char *key_file, char *ca_cert)`

    
    **API documentation:**

    `config_mgr_t* config_mgr_new(char *storage_type, char *cert_file, char *key_file, char *ca_cert);`
    ```
        config_mgr_new function to creates a new config manager client
        @param storage_type      - Type of key-value storage, Eg. etcd
        @param cert_file         - config manager client cert file
        @param key_file          - config manager client key file
        @param ca_cert           - config manager client ca cert
        @return config_mgt_t     - config_mgr_t object on success, NULL on failure
    ```

2. Accessing value of a key stored in distributed store like etcd

    `char* value = config_mgr_client->get_config("/VideoIngestion/config");`

    **API documentation:**

    `char* get_config(char *key)`
    ```
        get_config function gets the value of a key from config manager
        @param key      - key to be queried on from config manager
        @return char*   - values returned from config manager based on key
    ```

3. Registers user callback function to keep a watch on key based on it's prefix

    `config_mgr_client->register_watch_dir("/VideoIngestion/config", user_callback);`

    **API documentation:**

    `void register_watch_dir(char *key, (*register_watch_dir_cb)(char* key, char* value) user_callback)`
    
    `Here user_callback is callback function to be passed as an argument and example is given below`

    `void user_callback(char* key, char *updated_value_on_watch){}`

    ```
        register_watch_dir function registers to a callback and keeps a watch on the prefix of a specified key
        @param key                                                 - prefix of a key to keep a watch on
        @param (*register_watch_dir_cb)(char* key, char* value)    - user callback to be called on watch event
                                                                     with updated value on the respective key
    ```
4. Registers user callback function to keep a watch on specified key

    `config_mgr_client->register_watch_key("/VideoIngestion/config", user_callback);`

    **API documentation:**

    `void register_watch_key(char *key, (*register_watch_key_cb)(char* key, char* value) user_callback)`

    `Here user_callback is callback function to be passed as an argument and example is given below`

    `void user_callback(char* key, char *updated_value_on_watch){}`

    ```
        register_watch_key function registers to a callback and keeps a watch on a specified key
        @param key                                                 - key to keep a watch on
        @param (*register_watch_key_cb)(char* key, char* value)    - user callback to be called on watch event
                                                                     with updated value on the respective key
    ```
5. Destroy config manager

    `config_mgr_config_destroy(config_mgr_client);`

    **API documentation:**
    `void config_mgr_config_destroy(config_mgr_t *config_mgr_config)`

    ```
        config_mgr_config_destroy function to delete the config_mgr_client instance
        @param config_mgt_t     -   config_mgr_client object
    ```

To refer C examples follow config_manager.c in [examples/](examples/)

**Steps to run example file:**

1. Navigate to ConfigManager/c
2. mkdir build
3. cd build
4. cmake -DWITH_EXAMPLES=ON ..
5. make
6. cd examples
7. `./config_manager`

### APIs for reading environment info to build EIS MessageBus config

1. Creating a new env config client

    `env_config_t* env_config_client = env_config_new();`

    **API documentation:**

    `env_config_t* env_config_new();`
    ```
        env_config_new function to creates a new env config client
        @return env_config_t     - env_config object
    ```

2. Getting publish/subscribe topics from env

    `char** pub_topics = env_config_client->get_topics_from_env("pub");`
    `char** sub_topics = env_config_client->get_topics_from_env("sub");`

    **API documentation:**

    `char** get_topics_from_env(const char* topic_type);`
    ```
        get_topics_from_env function gives the topics with respect to publisher/subscriber.
        @param topic_type - "pub" for publisher and "sub" for subscriber.
        @return topics    - topics returned from env config based on topic type
    ```

3. Getting EIS Message Bus config from ENV variables and config manager.

    `config_t* msgbus_config_pub = env_config_client->get_messagebus_config(config_mgr_client, pub_topics[], num_of_topics, "pub");`
    `config_t* msgbus_config_sub = env_config_client->get_messagebus_config(config_mgr_client, sub_topics[], num_of_topics, "sub");`

    **API documentation:**

    `config_t* get_messagebus_config(const config_mgr_t* configmgr, const char* topic[], size_t num_of_topics, const char* topic_type);`
    ```
        get_messagebus_config function gives the configuration that needs in connecting to EIS messagebus. In case of supporting multiple topics, the application will be responsible for providing the context for TCP & IPC connection. Example, in case of IPC the connection parameters which include socket directory & socket file are provided by the application.

        @param configmgr       - Config Manager object
        @param topic           - Array of Topics for which msg bus config needs                          to be constructed
                                 In case the topic is being published, it will be the stream name like `camera1_stream`
                                 and in case the topic is being subscribed, it will be of the format
                                 `[Publisher_AppName]/[stream_name]`.
                                 Eg: `VideoIngestion/camera1_stream`
        @param num_of_topics   - Number of topics
        @param topic_type      - TopicType for which msg bus config needs to be constructed
        @return config_t*      - JSON msg bus config of type config_t
    ```

4. Destroy Env Config

    `env_config_destroy(env_config_client);`

    **API documentation:**

    `void env_config_destroy(env_config_t* env_config_client);`
    ```
        env_config_destroy function to delete the env_config_client instance
        @param env_config_t     -   env_config_client object
    ```

To refer C examples follow env_config_example.c in [examples/](examples/)

**Steps to run example file:**

1. Navigate to ConfigManager/c
2. mkdir build
3. cd build
4. cmake -DWITH_EXAMPLES=ON ..
5. make
6. cd examples
7. `./env_config`
