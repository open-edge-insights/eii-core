# ConfigMgr

ConfigMgr (Config manager) provides CPP/Python/Golang APIs to:

- fetch an applications configs values from the KV store.
- fetch an applications interface values from KV store for pub, sub, server and client.
- watch on application's config changes.
- generate EII MessageBus config.
- read and set /GlobalEnv variables.
- fetch env variables: appname, dev_mode

All these data are stored into the kv store of EII during the provisioning phase and can be changed   dynamically by the admin.

## Installation

The EII Config Manager library can be installed in two different ways.

1. Through published Debian, Fedora, or Alpine APK packages
2. Installing form source

If you are installing from one of the packages, select the package you wish to
install from the releases assets, and then run one of the following depending
on the OS you are installing on:

```sh
# Debian
sudo apt install libcjson1 libzmq5 zlib1g
sudo dpkg -i <debian package>

# Fedora
sudo dnf install cjson zeromq zlib
sudo rpm -i <rpm package>

# Alpine (NOTE: the depencies get automatically installed by the apk command)
sudo apk add --allow-untrusted <apk package>
```

In the above commands, installing the cJSON and ZeroMQ dependencies
is required, however, in general, installation of the dev module is not required
(i.e. the OS packages which include all of the headers for the libraries). If
you are compiling an application that is linking to this library, then it is
recommended that you install the dev versions of the libraries. For Ubuntu this
would mean installing `libcjson-dev libzmq3-dev zlib1g-dev`. For Fedora the
packages would be `cjson-devel zeromq-devel zlib-devel`. In Alpine, the packages
would be `cjson-dev zeromq-dev zlib-dev`.

If you want to compile the EII Config Manager from source, follow the
intructions below.

The EIIConfigMgr depends on CMake version 3.11+. For Ubuntu 18.04 this is not
the default version installed via `apt-get`. To install the correct version
of CMake and other ConfigMgr dependencies, please follow [eii_libs_installer README](../../README.md)

CMAKE_INSTALL_PREFIX needs to be set for the installation:

```sh
    export CMAKE_INSTALL_PREFIX="/opt/intel/eii"
```

ConfigMgr installs to `/opt/intel/eii/lib/`. On some platforms this is not included in the `LD_LIBRARY_PATH` by default. As a result, you must add this directory to your `LD_LIBRARY_PATH`,
otherwise you will encounter issues using the ConfigMgr. This can
be accomplished with the following `export`:

```sh
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/opt/intel/eii/lib/
```

> **NOTE:** You can also specify a different library prefix to CMake through
> the `CMAKE_INSTALL_PREFIX` flag. If different installation path is given via `CMAKE_INSTALL_PREFIX`, then `$LD_LIBRARY_PATH` should be appended by $CMAKE_INSTALL_PREFIX/lib.

To install the remaining dependencies for the message bus execute the following
command:

**Note**: It is highly recommended that you use a python virtual environment to
install the python packages, so that the system python installation doesn't
get altered. Details on setting up and using python virtual environment can
be found here: <https://www.geeksforgeeks.org/python-virtual-environment/>

```sh
sudo apt install libcjson-dev libzmq5 zlib1g-dev
```

> **NOTE:** For Fedora, the packages should be `cjson-devel zeromq-devel zlib-devel` and for
> Alpine it is `cjson-dev zeromq-dev zlib-dev`.

If you wish to compile the Python binding as well, then you must also install
the Python requirements. To do this, execute the following `pip` command:

```sh
pip3 install --user -r ./python/requirements.txt
```

## Packaging

This library supports being packaged as a Debian, RPM, or Alpine APK packages.
This is all accomplished via CMake. By default, packaging is disabled. To
enable packaging, add the `-DPACKAGING=ON` flag to your CMake command (see
Compilation section above). This command will look something like:

```sh
cmake -DPACKAGING=ON ..
```

By default, the packaging utilities will scan the system for the required
toolchains it needs to build each package type (Deb, RPM, and APK). If it does
not find the required toolsets, then it will disable that form of packaging.
The packaging utilities provide CMake flags to force packaging as any of the
supported package types. If a given package type, ex. APK, is set to be enabled
manually by its CMake flag and its required packaging toolchain does not exist,
then CMake will raise a fatal error.

The table below provides the required toolchains for each package type as well
as the CMake flag to set to `ON` to manually enable a packaging type:

| Package Type | Required Tools | Manual Package Flag |
| :----------: | :------------: | :-----------------: |
| `deb`        | `dpkg-deb`     | `PACKAGE_DEB`       |
| `rpm`        | `rpmbuild`     | `PACKAGE_RPM`       |
| `apk`        | `docker`       | `PACKAGE_APK`       |

> **NOTE:** Manually setting a given package type to be built (e.g. setting
> `-DPACKAGE_DEB=ON`) still requires that the `-DPACKAGING=ON` to be set.

After the required toolchains have been installed and CMake has been run with
some combination of the packaging flags, the library can be packaged with the
following commands:

```sh
make package
```

The command above will build the Debian and RPM packages (depending on the
specified CMake flags).

To build the Alpine APK package, execute the following command:

```sh
make package-apk
```

**IMPORTANT:**

The EII Config Manager depends on the EII Utils and EII Message Bus libraries.
In order to compile the Alpine APK package for the EII Config Manager it must
have the APK packages for the EII Utils and EII Message Bus modules.

To provide this, you must first build or download the Alpine APK package for the
EII Utils library (see it's repo [here](https://github.com/open-edge-insights/eii-c-utils)
to obtain the library) and also for the EII Message Bus (see it's repo
[here](https://github.com/open-edge-insights/eii-messagebus) to obtain the library).

Once you have the APKs, create an, "apks" directory at the top level of this
repository.

```sh
mkdir apks/
```

Next, place the EII Utils and EII Message Bus APK packages into the, "apks",
directory. Then execute the `make package-apk` command. If this is not done,
then the build will fail.

### A Note on Alpine APK Packaging

In order to package the library as an Alpine APK package, the packaging utility
must use a Docker container to have access to the proper Alpine APK toolchains.
This container will automatically be built when the CMake command is ran to
configure your build environment.

By default, Alpine 3.14 is used to build the package. However, this version
can be changed by setting the `APKBUILD_ALPINE_VERSION` CMake flag to the
version of Alpine you wish to use (ex. `-DAPKBUILD_ALPINE_VERSION=3.12`).

## Install ConfigMgr with Python bindings, Go bindings, Examples, Test suits and Debug Build

```sh
rm -rf build
mkdir build
cd build
cmake -DCMAKE_INSTALL_INCLUDEDIR=$CMAKE_INSTALL_PREFIX/include -DCMAKE_INSTALL_PREFIX=$CMAKE_INSTALL_PREFIX -DWITH_PYTHON=ON -DWITH_GO=ON -DWITH_EXAMPLES=ON -DWITH_TESTS=ON -DCMAKE_BUILD_TYPE=Debug ..
make
sudo make install
```

`WITH_PYTHON=ON`, `WITH_GO=ON`, `WITH_EXAMPLES=ON`, `WITH_TESTS=ON` and `CMAKE_BUILD_TYPE=Debug` to compile ConfigMgr with Python bindings, Go bindings, Examples, Unit Tests and Debug mode respectively.

# Interfaces

ConfigMgr parses the data from the kv store (eg: etcd) and the application environment variables for its functionality.
It supports Publisher, Subscriber, Server and Client interfaces. Below are the examples for providing
different interfaces and different usecases.

Please refer [different ways of giving endpoints](###**Note**-"endpoint"-can-be-given-in-different-ways:)

### **Publisher Interface**

```javascript
{
    "Publishers": [
        {
            "Name": "default",
            "Type": "zmq_ipc",
            "EndPoint": "/EII/sockets",
            "Topics": [
                "camera1_stream"
            ],
            "AllowedClients": [
                "*"
            ]
        },
        {
            "Name": "example",
            "Type": "zmq_tcp",
            "EndPoint": "127.0.0.1:65015",
            "Topics": [
                "*"
            ],
            "AllowedClients": [
                "Visualizer", "VideoAnalytics"
            ]
        }
    ]
}
```

|          Key        |    Type   | Required (Mandatory) |                         Description                          |
| :-----------------: | --------- | -------------------- | ------------------------------------------------------------ |
| `Publishers`         | `array`   | Yes                  | Entire publisher interface will be added with in the array. Multiple publish endpoints can be added by adding elements in the array.  |
| `Name`              | `string`  | Yes                  | Name of different publisher interfaces                   |
| `Type`              | `string`  | Yes                  | Specifies ZeroMQ protocol ("zmq_tcp" or "zmq_ipc") on which data will be published    |
| `EndPoint`          | `string` or `object`  | Yes                  | In case of TCP or IPC (socket directory only), endpoint should be string as shown in the above examples. In case IPC explicitly specifying socket file, either object or string can be used for EndPoint. Please refer [Different ways of specifying endpoint](###**Note**-"endpoint"-can-be-given-in-different-ways:)|
| `Topics`            | `array`   | Yes                  | Specifying the topics on which data will be published on. Multiple elements in this array can denote multiple topics published on the same endpoint   |
| `AllowedClients`    | `array`   | Yes                  | Specifying who can subscribe to the the topic on which data is published. If AllowedClients is "*", then all the provisioned services can receive the data published.                  |

### **Subscriber Interface**

```javascript
{
    "Subscribers": [

        // tcp usecase
        {
            "Name": "example",
            "Type": "zmq_tcp",
            "EndPoint": "127.0.0.1:65013",
            "PublisherAppName": "VideoIngestion",
            "Topics": [
                "*"
            ]
        },

        // ipc usecase
        {
            "Name": "default",
            "Type": "zmq_ipc",
            "EndPoint": ".socks",
            "PublisherAppName": "VideoIngestion",
            // Topics cannot be "*", if the only IPC directory is given
            // if it Topics "*" to be used in ipc, then socket file should be given explicitly.
            "Topics": [
                "camera1_stream"
            ]
        }
    ]
}
```

|          Key        |    Type   | Required (Mandatory) |                         Description                          |
| :-----------------: | --------- | -------------------- | ------------------------------------------------------------ |
| `Subscribers`        | `array`   | Yes                  | Entire subscriber interface will be added with in the array. Multiple subscribe endpoints can be added by adding elements in the array  |
| `Name`              | `string`  | Yes                  | Name of different subscriber interfaces                      |
| `Type`              | `string`  | Yes                  | Specifies ZeroMQ protocol ("zmq_tcp" or "zmq_ipc") through which subscription happens |
| `EndPoint`          | `string` or `object`  | Yes                  | In case of TCP or IPC (socket directory only), endpoint should be string as shown in the above examples. In Case IPC explicitly specifying socket file, either object or string can be used for EndPoint. Please refer [Different ways of specifying endpoint](###**Note**-"endpoint"-can-be-given-in-different-ways:) |
|`PublisherAppName`   | `string`  | Yes                  | Specifies the publisher's AppName through which data will be received|
| `Topics`            | `array`   | Yes                  | Specifying the topics on which data will be published on. If Topics is "*", the subscriber receives all the data published on the endpoint, irrespective of the topic names data is published on. Multiple elements in this array can denote multiple topics subscribed on the same endpoint   |

### **Server Interface**

```javascript
{
    "Servers": [
        // tcp usecase
        {
            "Name": "default",
            "Type": "zmq_tcp",
            "EndPoint": "127.0.0.1:9006",
            "AllowedClients": [
                "VideoAnalytics"
            ]
        },
        //ipc usecase
        {
            "Name": "example",
            "Type": "zmq_ipc",
            "EndPoint": "/EII/sockets",
            "AllowedClients": [
                "VideoAnalytics"
            ]
        }
    ]
}
```

|          Key        |    Type   | Required (Mandatory) |                         Description                          |
| :-----------------: | --------- | -------------------- | ------------------------------------------------------------ |
| `Servers`           | `array`   | Yes                  | Entire server interface will be added with in the array. Multiple server endpoints can be added by adding elements in the array   |
| `Name`              | `string`  | Yes                  | Name of different server interfaces                          |
| `Type`              | `string`  | Yes                  | Specifies ZeroMQ protocol ("zmq_tcp" or "zmq_ipc") through which Server pushes data   |
| `EndPoint`          | `string` or `object`  | Yes                  | In case of TCP or IPC (socket directory only), endpoint should be string as shown in the above examples. In case IPC explicitly specifying socket file, either object or string can be used for EndPoint. Please refer [Different ways of specifying endpoint](###**Note**-"endpoint"-can-be-given-in-different-ways:) |
| `AllowedClients`    | `array`   | Yes                  | Specifying who can get data is from the Server. If AllowedClients is "*", then all the provisioned services can connect to Server.                                                |

### **Client Interface**

```javascript
{
    "Clients": [
        // tcp usecase
        {
            "Name": "default",
            "ServerAppName": "VideoIngestion",
            "Type": "zmq_tcp",
            "EndPoint": "127.0.0.1:9006"
        },

        // ipc usecase
        {
            "Name": "example",
            "ServerAppName": "VideoIngestion",
            "Type": "zmq_ipc",
            "EndPoint": "/EII/sockets"
        }
    ]
}
```

|          Key        |    Type   | Required (Mandatory) |                         Description                          |
| :-----------------: | --------- | -------------------- | ------------------------------------------------------------ |
| `Clients`           | `array`   | Yes                  | Entire client interface will be added with in the array. Multiple client endpoints can be added by adding elements in the array   |
| `Name`              | `string`  | Yes                  | Name of different client interfaces                          |
| `ServerAppName`     | `string`  | Yes                  | Server's AppName to which client connection is established      |
| `Type`              | `string`  | Yes                  | Specifies ZeroMQ protocol ("zmq_tcp" or "zmq_ipc") through which client connection is established   |
| `EndPoint`          | `string` or `object`  | Yes                  | In case of TCP or IPC (socket directory only), endpoint should be string as shown in the above examples. In case IPC explicitly specifying socket file, either object or string can be used for EndPoint. Please refer [Different ways of specifying endpoint](###**Note**-"endpoint"-can-be-given-in-different-ways:) |

## Overriding of Type and EndPoint

In a given publisher, subscriber, client or server interfaces, the "Type" and "Endpoint" values mentioned in the respective interfaces can be overriden by setting them as env variables.

Let's take publisher as an example having multiple endpoints and below is its interface:

```javascript
{
    "Publishers": [
        {
            "Name": "default",
            "Type": "zmq_ipc",
            "EndPoint": "/EII/sockets",
            "Topics": [
                "camera1_stream"
            ],
            "AllowedClients": [
                "*"
            ]
        },
        {
            "Name": "example",
            "Type": "zmq_tcp",
            "EndPoint": "127.0.0.1:65015",
            "Topics": [
                "*"
            ],
            "AllowedClients": [
                "Visualizer", "VideoAnalytics"
            ]
        }
    ]
}
```

Let's say we want to override the Type ("zmq_ipc") and Endpoint ("/EII/sockets") of the first publisher's interface (interface having "Name":"default"), then env variable should be set with below syntax.

```sh
export PUBLISHER_default_TYPE="zmq_tcp"
export PUBLISHER_default_ENDPOINT="127.0.0.1:65013"
```

In the above command `default` is the `Name` of the interface.

Similarly, we can override subscriber's, client's and server's Type and Endpoint.

```sh
export SUBSCRIBER_default_TYPE="zmq_tcp"
export SUBSCRIBER_default_ENDPOINT="127.0.0.1:65013"

export SERVER_default_TYPE="zmq_tcp"
export SERVER_default_ENDPOINT="127.0.0.1:65013"

export CLIENT_default_TYPE="zmq_tcp"
export CLIENT_default_ENDPOINT="127.0.0.1:65013"
```

Let's take publisher as an example having single endpoint and below is its interface:

```javascript
{
    "Publishers": [
        {
            "Name": "default",
            "Type": "zmq_ipc",
            "EndPoint": "/EII/sockets",
            "Topics": [
                "camera1_stream"
            ],
            "AllowedClients": [
                "*"
            ]
        }
}
```

If we want to override the Type ("zmq_ipc") and Endpoint ("/EII/sockets") of publisher's interface, then env variable should be set with below syntax.

```sh
export PUBLISHER_TYPE="zmq_tcp"
export PUBLISHER_ENDPOINT="127.0.0.1:65013"
```

Similarly, we can override subscriber's, client's and server's Type and Endpoint.

```sh
export SUBSCRIBER_TYPE="zmq_tcp"
export SUBSCRIBER_ENDPOINT="127.0.0.1:65013"

export SERVER_TYPE="zmq_tcp"
export SERVER_ENDPOINT="127.0.0.1:65013"

export CLIENT_TYPE="zmq_tcp"
export CLIENT_ENDPOINT="127.0.0.1:65013"
```

Overriding feature of ConfigMgr will be used in orchestrated scenarios including Kubernetes.

## Broker Usecase

If publisher and subscriber wants to communicate via broker(ZmqBroker), i.e., if publisher publish data to ZmqBroker and subscriber subscribes from ZmqBroker, then the interfaces of ZmqBroker, subscriber and publisher with respect to `zmq_tcp` and `zmq_ipc` protocol as follows.

## zmq_tcp Protocol usecase

### ZmqBroker

```javascript
{
    //X-PUB
    "Publishers": [
        {
            "AllowedClients": [
                "*"
            ],
            "EndPoint": "127.0.0.1:5568",
            "Name": "backend",
            "Topics": [
                "*"
            ],
            "Type": "zmq_tcp"
        }
    ],
    // X-SUB
    "Subscribers": [
        {
            "AllowedClients": [
                "*"
            ],
            "EndPoint": "127.0.0.1:5569",
            "Name": "frontend",
            "PublisherAppName": "*",
            "Topics": [
                "*"
            ],
            "Type": "zmq_tcp"
        }
    ]
}

```

### Subscriber

```javascript
{
    "Subscribers": [
        {
            "Name": "default",
            "Type": "zmq_tcp",
            "EndPoint": "127.0.0.1:5568",

            // PublisherAppName will be "ZmqBroker" in case of brokered usecase
            "PublisherAppName": "ZmqBroker",
            "Topics": [
                "*"
            ]
        }
    ]
}
```

### Publisher

```javascript
{
    "Publishers": [
        {
            "Name": "default",
            "Type": "zmq_tcp",
            "EndPoint": "127.0.0.1:5569",
            "Topics": [
                "camera1_stream"
            ],
            // With broker usecase, AllowedClients in Publisher's interface is not required, as it acts as a subscriber to X-SUB

            // "brokered" and "BrokerAppName" should be added for ZmqBroker usecase
            "brokered": true,
            "BrokerAppName" : "ZmqBroker"
        }
    ]
}

```

## zmq_ipc Protocol usecase

### ZmqBroker

```javascript
{
    "Publishers": [
        {
            "Name": "backend",
            "Type": "zmq_ipc",
            "EndPoint": {
                "SocketDir": "/EII/sockets",
                "SocketFile": "backend-socket"
            },
            "Topics": ["*"],
            "AllowedClients": ["*"]
        }
    ],
    "Subscribers": [
        {
            "Name": "frontend",
            "Type": "zmq_ipc",
            "EndPoint": {
                "SocketDir": "/EII/sockets",
                "SocketFile": "frontend-socket"
            },
            "PublisherAppName": "*",
            "Topics": ["*"],
            "AllowedClients": ["*"]
        }
    ]
}

```

### Subscriber

```javascript
{
    "Subscribers": [
        {
            "Name": "default",
            "Type": "zmq_ipc",
            "EndPoint": {
                "SocketDir" : "/EII/sockets",
                "SocketFile": "backend-socket"
            },

            // PublisherAppName will be "ZmqBroker" in case of brokered usecase
            "PublisherAppName": "ZmqBroker",

            // Topics cannot be "*", if the only IPC directory is given
            // if it Topics "*" to be used in ipc, then socket file should be given explicitly.
            "Topics": [
                "*"
            ]
        }
    ]
}
```

### Publisher

```javascript
{
    "Publishers": [
        {
            "Name": "default",
            "Type": "zmq_ipc",
            "EndPoint": {
                "SocketDir" : "/EII/sockets",
                "SocketFile": "frontend-socket"
            },
            "Topics": [
                "camera1_stream"
            ],

            // With broker usecase, AllowedClients in Publisher's interface is not required, as it acts as a subscriber to X-SUB

            // "brokered" and "BrokerAppName" should be added for ZmqBroker usecase
            "brokered": true,
            "BrokerAppName" : "ZmqBroker"
        }
    ]
}

```

|          Key        |    Type   | Required (Mandatory) |                         Description                          |
| :-----------------: | --------- | -------------------- | ------------------------------------------------------------ |
| `brokered`          | `boolean` | Yes                   | (Required if publishing via ZmqBroker) Specifies if publisher is using broker or not. use "brokered": true for use with broker. |
| `BrokerAppName`     | `string`  | Yes                   | (Required if publishing via ZmqBroker) Specifies ZeroMQ Broker app name|

### **Note** "Endpoint" can be given in different ways

- zmq_tcp Endpoint should look like below:

    ```javascript
    "Endpoint":"127.0.0.1:65013"
    ```

- zmq_ipc Endpoint have different ways of giving.

  - Specifying just socket directory

        ```javascript
        "Endpoint":"/EII/sockets"
        ```

  - Specifying socket directory and socket file

        ```javascript
        "Endpoint":{
            "SocketDir" : "/EII/sockets",
            "SocketFile": "socketfile"
        }

        or

        "Endpoint":"/EII/sockets, socketfile"
        ```

## Running Examples

The ConfigMgr library also supports Cpp APIs and Python & Go bindings. These APIs/bindings can be used in Cpp and Python/Go services in the EII stack to fetch required config/interfaces/msgbus config.

Examples will only be compiled if the `WITH_EXAMPLES=ON` option is specified while running CMake.
Please refer [Examples installation](###-install-configMgr-with-examples,-test-suits-and-debug-build-enabled.)

Refer the interfaces of publisher and server in [./examples/configs/VideoIngestion_interfaces.json](./examples/configs/VideoIngestion_interfaces.json) and for subscriber and client, refer [./examples/configs/VideoAnalytics_interfaces.json](./examples/configs/VideoAnalytics_interfaces.json)

Examples on demonstrating the usage of these APIs in the bindings have been given in respective sections below.

- [CPP Examples](###-cpp-examples)

- [Python Examples](###-python-examples)
- [Go Examples](###-go-examples)

### CPP Examples

```sh
Navigate to [WORKDIR]/IEdgeInsights/common/libs/ConfigMgr
sudo rm -rf build
mkdir build
cd build
cmake -DCMAKE_INSTALL_INCLUDEDIR=$CMAKE_INSTALL_PREFIX/include -DCMAKE_INSTALL_PREFIX=$CMAKE_INSTALL_PREFIX -DWITH_EXAMPLES=ON ..
make
sudo make install
```

There are currently 5 CPP examples:

1. `examples/sample_pub.cpp`
2. `examples/sample_sub.cpp`
3. `examples/sample_server.cpp`
4. `examples/sample_client.cpp`
5. `examples/sample_getvalue.cpp`

All of the Cpp example executables are in the `build/examples/` directory. To run
them, execute the following command:

Before executing any of the examples, please run below command from `build/examples/`

```sh
 cd ../../examples/ && source ./env.sh && cd -
```

Publisher example.

```sh
./pub
```

Subscriber example.

```sh
./sub
```

Server example.

```sh
./server
```

Client example.

```sh
./client
```

Sample_getvalue used to get the values from application's config

```sh
./sample_app
```

### Python Examples

```sh
Navigate to [WORKDIR]/IEdgeInsights/common/libs/ConfigMgr
sudo rm -rf build
mkdir build
cd build
cmake -DCMAKE_INSTALL_INCLUDEDIR=$CMAKE_INSTALL_PREFIX/include -DCMAKE_INSTALL_PREFIX=$CMAKE_INSTALL_PREFIX -DWITH_PYTHON=ON ..
make
sudo make install
```

There are currently 5 Python examples:

1. `examples/publisher.py`
2. `examples/subscriber.py`
3. `examples/echo_service.py`
4. `examples/echo_client.py`
5. `examples/sample_get_value.py`

All of the py examples are in `python/examples/` directory. To run
them, execute the following command:

Before executing any of the examples, please run below command from `python/examples/`

```sh
 cd ../../examples/ && source ./env.sh && cd -
```

Publisher example.

```sh
python3 publisher.py
```

Subscriber example.

```sh
python3 subscriber.py
```

Server example.

```sh
python3 echo_service.py
```

Client example.

```sh
python3 echo_client.py
```

sample_get_value used to get the values from application's config

```sh
python3 sample_get_value.py
```

### Go Examples

```sh
Navigate to [WORKDIR]/IEdgeInsights/common/libs/ConfigMgr/go
cp -r ConfigMgr/ $GOPATH/src
cd ConfigMgr/examples
source go_env.sh
```

There are currently 5 Go examples:

1. `publisher.go`
2. `subscriber.go`
3. `echo_service.go`
4. `echo_client.go`
5. `app_config.go`

All of the go examples are in `go/examples/` directory. To run
them, execute the following command:

Before executing any of the examples, please run below command from `go/ConfigMgr/examples/`

```sh
 cd ../../../examples/ && source ./env.sh && cd -
 source ./go_env.sh
```

Publisher example.

```sh
go build publisher.go
./publisher
```

Subscriber example.

```sh
go build subscriber.go
./subscriber
```

Server example.

```sh
go build echo_service.go
./echo_service
```

Client example.

```sh
go build echo_client.go
./echo_client
```

app_config used to get the values from application's config

```sh
go build app_config.go
./app_config
```

## Running Unit Tests

> **NOTE:**

- The unit tests will only be compiled if the `WITH_TESTS=ON` option is specified when running CMake.
Please refer [Unit Test installation](###-install-configMgr-with-examples,-test-suits-and-debug-build-enabled.) installation.

- Provisioning should be done to start etcd server in dev/prod mode and to generate application specific certificates(only in prod mode).

Before executing any of the test files, please run below command from `build/tests/`

```sh
 cd ../../examples/ && source ./env.sh && cd -
```

- To run ConfigMgr unit tests

```
./config_manager_unit_tests
./kvstore_client-tests
```

## Creation of grpc .deb file (Optional)

**Note**: This is an optional as we have already created .deb file in the repo.
If user wants to create .deb file freshly, then one has to follow this step.

Navigate to `[WORKDIR]/IEdgeInsights/common/libs/ConfigMgr` and run the `grpc_deb.sh`

```sh
sudo ./grpc_deb.sh
```

By executing the above script, `grpc-1.29.0-Linux.deb` will be created in
`[WORKDIR]/IEdgeInsights/common/libs/ConfigMgr`.
