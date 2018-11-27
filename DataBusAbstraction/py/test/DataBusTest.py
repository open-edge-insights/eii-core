"""
Copyright (c) 2018 Intel Corporation.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
"""

import time
import argparse
import sys
import os
path = os.path.abspath(__file__)
sys.path.append(os.path.join(os.path.dirname(path), "../"))
from DataBus import databus

# TODO: Need to come up with a generic test program that works with all
# databuses abstracted, not just opcua


class A:
    def cbFunc(self, topic, msg):
        print("In DataBusTest.py........")
        print("Msg: {} received on topic: {}".format(msg, topic))

    def main(self):
        p = argparse.ArgumentParser()
        p.add_argument('-endpoint', help="Provide the messagebus details")
        p.add_argument('-direction', help="one of \'PUB | SUB\'")
        p.add_argument('-ns', help="namespace")
        p.add_argument('-topic', help="topic names")
        p.add_argument('-certFile', help="provide server/client certificate\
                                         file path as value")
        p.add_argument('-privateFile', help="provide server or client private\
                                            key file pathas value")
        p.add_argument('-trustFile', help="provide ca cert file path as value")

        args = p.parse_args()

        contextConfig = {"endpoint": args.endpoint,
                         "direction": args.direction,
                         "name": args.ns,
                         "certFile": args.certFile,
                         "privateFile": args.privateFile,
                         "trustFile": args.trustFile}
        try:
            etadbus = databus()
            etadbus.ContextCreate(contextConfig)
            if args.direction == "PUB":
                topicConfig = {"name": args.topic, "type": "string"}
                for i in range(0, 20):
                    result = "Hello %d".format(i)
                    etadbus.Publish(topicConfig, result)
                    time.sleep(5)
            elif args.direction == "SUB":
                topicConfig = {"name": args.topic, "type": "string"}
                etadbus.Subscribe(topicConfig, "START", self.cbFunc)
                flag = "START"
                while True:
                    pass
            else:
                raise Exception("Wrong direction flag for databus!!!")
            etadbus.ContextDestroy()
        except Exception as e:
            raise


if __name__ == "__main__":
    a = A()
    a.main()
