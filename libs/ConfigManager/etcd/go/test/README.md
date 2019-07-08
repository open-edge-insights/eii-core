# ConfigManager

ConfigManager provides go wrapper APIs `etcd` distributed key-value store to read EIS related config details.

The go example program demonstrates 3 basic functionalities :
* GetConfig - which returns value of the specified key from etcd.
* RegisterKeyWatch - which keeps a watch on the specified key in etcd and triggers the callback whenever the value of the key is changed.
* RegisterDirwatch - which keeps a watch on the prefix of a specified key in etcd and triggers the callback whenever the value of the key is changed.

## How to run unit tests from present working directory

### 1. Pre-requisite

* Install etcd service by following the link: [etcd service](https://computingforgeeks.com/how-to-install-etcd-on-ubuntu-18-04-ubuntu-16-04/)

* Install go-etcd3, go-etcd3 is a go etcd client lib, install this by running go get go.etcd.io/etcd/clientv3 from $GOPATH

## How to Test from present working directory
### 1. Testing with security enabled

* To test basic functionalities:

```sh
make etcdclient key=<provide a key> action=<provide the action to be performed on etcd key, possible options are get, watchkey, watchdir>
```

### 1. Testing with security disabled

* To test basic functionalities:

```sh
make etcdclient_insecure key=<provide a key> action=<provide the action to be performed on etcd key, possible options are get, watchkey, watchdir>
```