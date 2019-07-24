# EIS Message Bus User Guide

## Overview

- [EIS Message Bus Architecture]()
- [EIS Message Bus Usage]()
    - [Compilation & Installtion]()
    - [Python Tutorial]()
    - [Go Tutorial]()
    - [C Tutorial]()
- [EIS Message Bus Development]()
    - [Developing Protocols]()

## EIS Message Bus Architecture


## EIS Message Bus Usage

### Compilation & Installation

The EIS Message Bus utilizes the CMake build tools for compiling the `C` and
`Python` libraries. To compile the library execute the following commands:

```sh
$ mkdir build
$ cd build
$ cmake ..
$ make
```

The EIS Message Bus adds the following flags to CMake for the build process
to enable and disable features in the message bus. The table below specifies
all of the additional flags which can be given to CMake while building the
message bus.

|      Flag       |                    Description                 |
| :-------------: | ---------------------------------------------- |
| `WITH_PYTHON`   | Compile the Python binding with the C binding. |
| `WITH_EXAMPLES` | Compile the C examples.                        |
| `WITH_TESTS`    | Compile the C unit tests.                      |

> **NOTE:** All of the flags are passed to CMake using the following CLI
> format: `-D<FLAG>=[ON | OFF]`. For example, `cmake -DWITH_PYTHON=ON ..`.

To install the message bus, execute the following command:

```sh
$ sudo make install
```

If the `WITH_PYTHON` flag was given to CMake during the compilation step, then
the Python binding will also be installed into the `dist-packages` in your
environment.

### Configuration

The EIS Message Bus is configured through a `key, value` pair interface. The
values can be objects, arrays, integers, floating point, boolean, or strings.
The keys that are required to be available in the configuration are largly
determined by the underlying protocol which the message bus will use. The
protocol is specified via the `type` key and currently must be one of the
following:

- `zmq_ipc` - ZeroMQ over IPC protocol
- `zmq_tcp` - ZeroMQ over TCP protocol

The following sections specify the configuration attributes expected for the
TCP and IPC ZeroMQ protocols.

#### ZeroMQ IPC Configuration

The ZeroMQ IPC protocol implementation only requires one configuration
attribute: `socket_dir`. The value of this attribute specifies the directory
where the message bus should create the Unix socket files to establish the IPC
based communication.

#### ZeroMQ TCP Configuration

The ZeroMQ TCP protocol has several configuration attributes which must be
specified based on the communication pattern the application is using and
based on the security the application wishes to enable for its communication.

##### Publishers

For an application which wishes to publish messages over specific topics, the
configuration must contain the key `zmq_tcp_publish`. This attribute must be
an object which has the following keys:

|          Key        |   Type   | Required |                         Description                      |
| :-----------------: | -------- | -------- | -------------------------------------------------------- |
| `host`              | `string` | Yes      | Specifies the host to publish as                         |
| `port`              | `int`    | Yes      | Specifies the port to publish messages on                |
| `server_secret_key` | `string` | No       | Specifies the secret key for the port for authentication |

The `server_secret_key` must be a Curve Z85 encoded string value that is
specified if the application wishes to use CurveZMQ authentication with to
secure incoming connections from subscribers.

##### Subscribers

To subscribe to messages coming from a publisher over TCP, the configuration
must contain a key for the topic you wish to subscribe to. For example, if
*Application 1* were publishing on topic `sensor-1`, then the subscribing
application *Application 2* would need to contain a configuration key `sensor-1`
which contains the keys required to configure the TCP connection to
*Application 1*.

The key that can be specified for a subscribers configuration are outline in the
table below.

|          Key        |   Type   | Required |                         Description                        |
| :-----------------: | -------- | -------- | ---------------------------------------------------------- |
| `host`              | `string` | Yes      | Specifies the host of the publisher                        |
| `port`              | `int`    | Yes      | Specifies the port of the publisher                        |
| `server_public_key` | `string` | No       | Specifies the publisher's public key for authentication    |
| `client_secret_key` | `string` | No       | Specifies the subscribers's secret key for authentication  |
| `client_public_key` | `string` | No       | Specifies the subcribers's public key for authentication   |

> **NOTE:** If one of the `*_key` values is specifed, then all of them must be
> specified.

##### Services

The configuration to host a service to receive and respond to requests is
similar to the configuration for doing publications on a message bus context.
The only difference, is the configuration for a service is placed under a key
which is the name of the service.

For example, if *Application 1* wishes to host a service named `example-service`,
then the configuration must contain an a key called `example-service`. The value
for that key must be an object containing the keys listed in the table of
the Publishers section.

##### Requesters

The configuration to issue requests to a service is the exact same as a
subscriber. In the case of a requester, instead of the configuration being
under the name of the topic, the configuration is placed under the name of
the service it wishes to connect to. For the details of the allowed values,
see the table in the Subscribers section above.

##### Using ZAP Authentication

For services and publishers additional security can be enabled for all incoming
connections (i.e. requesters and subscribers). This method utilizes the ZMQ
ZAP protocol to verify the incoming client public keys against a list of
whitelisted clients.

The list of allowed clients is given to the message bus via the `allowed_clients`
key. This key must be a list of Z85 encoded CurveZMQ keys.

### Python Tutorial

#### Publish/Subscribe

#### Request/Response

### Go Tutorial

#### Publish/Subscribe

#### Request/Response

### C Tutorial

#### Publish/Subscribe

#### Request/Response

## EIS Message Bus Development

### Developing Protocols
