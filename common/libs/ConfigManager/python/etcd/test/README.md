# ConfigManager

ConfigManager provides py wrapper APIs `etcd` distributed key-value store to read EIS related config details.

The py example program demonstrates 3 basic functionalities :
1. GetConfig - which returns value of the specified key from etcd.
2. RegisterKeyWatch - which keeps a watch on the specified key in etcd and triggers the callback whenever the value of the key is changed.
3. RegisterDirwatch - which keeps a watch on the prefix of a specified key in etcd and triggers the callback whenever the value of the key is changed.

## How to run unit tests from present working directory

### 1. Pre-requisite

* Install etcd service by following the link: [etcdservice](https://computingforgeeks.com/how-to-install-etcd-on-ubuntu-18-04-ubuntu-16-04/)

* Install python-etcd3 which is a python etcd client lib, from sources(as the master supports for watchprefix feature) by following the link: [python-etcd3](https://python-etcd3.readthedocs.io/en/latest/installation.html#from-sources)
### Dependencies

* Set `PYTHONPATH` env variable to ConfigManager/etcd/py folder path
  * If executing from ConfigManager/etcd/py/test, set it as below:
    
    ```sh
    export PYTHONPATH=../../../../..
    ```

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
