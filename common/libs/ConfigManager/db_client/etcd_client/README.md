# C++ EtcdClient

### Install gRPC
Make sure you have installed gRPC on your system. Follow the
[BUILDING.md](../../../BUILDING.md) instructions.

### Get the tutorial source code

The example code for this and our other examples lives in the `examples`
directory. Clone this repository to your local machine by running the
following command:


```sh
$ git clone -b $(curl -L https://grpc.io/release) https://github.com/grpc/grpc
```



### Install EtcdClient and build examples

```sh
$ mkdir build
$ cd build
$ cmake ..
$ make
$ sudo make install
```
