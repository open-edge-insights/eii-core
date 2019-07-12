# Copyright (c) 2019 Intel Corporation.

# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.


from libs.ConfigManager.etcd.py.etcd_client import EtcdCli
import time
import argparse


def callback(key, value):
    print("callback called")
    print("watch function is watching the key: {0}\nnew value is: \
        \n{1}".format(key, value))


def main():
    p = argparse.ArgumentParser()
    p.add_argument('-endpoint', help="Provide the endpoint Eg:localhost:2379")
    p.add_argument('-certFile', help="provide client certificate")
    p.add_argument('-privateFile', help="provide client private key file")
    p.add_argument('-trustFile', help="provide ca cert file")
    p.add_argument('-key', help="provide etcd key")
    p.add_argument('-action', help="provide the action to be performed \
                    on etcd key Eg: get|watchkey|watchdir")
    args = p.parse_args()

    conf = {"endpoint": args.endpoint, "certFile": args.certFile,
            "keyFile": args.privateFile, "trustFile": args.trustFile}
    etcd = EtcdCli(conf)

    if args.action == "get":
        try:
            value = etcd.GetConfig(args.key)
            print("GetConfig is called and the value is", value)
        except Exception as e:
            print("GetConfig failure {}".format(e))
        return
    elif args.action == "watchkey":
        try:
            etcd.RegisterKeyWatch(args.key, callback)
            print("Watching on the key", args.key)
        except Exception as e:
            print("WatchKey failure {}".format(e))
    elif args.action == "watchdir":
        try:
            etcd.RegisterDirWatch(args.key, callback)
        except Exception as e:
            print("WatchDir failure {}".format(e))
    else:
        print("Provided action is not supported")
        return

    while True:
        time.sleep(1)


if __name__ == "__main__":
    main()
