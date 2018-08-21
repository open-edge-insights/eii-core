"""
Copyright (c) 2018 Intel Corporation.

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
"""

import time
import argparse
import sys
import os
path = os.path.abspath(__file__)
sys.path.append(os.path.join(os.path.dirname(path), "../"))

from DataBus import databus


def cbFunc(topic, msg):
    print("Msg: {} received on topic: {}".format(msg, topic))
    print("")


if __name__ == "__main__":
    p = argparse.ArgumentParser()
    p.add_argument('--endpoint', help="Provide the messagebus details")
    p.add_argument('--direction', help="one of \'PUB | SUB\'")
    p.add_argument('--ns', help="namespace")
    p.add_argument('--topic', nargs='+', help="topic names")
    p.add_argument('--msg', help="Message to sent")
    args = p.parse_args()

    contextConfig = {"endpoint": args.endpoint, "direction": args.direction,
                     "name": args.ns}
    try:
        etadbus = databus()
        etadbus.ContextCreate(contextConfig)
        if args.direction == "PUB":
            for _, topic in enumerate(args.topic):
                topicConfig = {"name": topic, "type": "string"}
                etadbus.Publish(topicConfig, args.msg)
        elif args.direction == "SUB":
            for _, topic in enumerate(args.topic):
                topicConfig = {"name": topic, "type": "string"}
                etadbus.Subscribe(topicConfig, "START", cbFunc)
            flag = "START"
        else:
            raise Exception("Wrong direction flag for databus!!!")
    except Exception as e:
        raise
    while True:
        var = raw_input("Enter CMDs['START', 'STOP', 'TERM']/MSG: ")
        try:
            if args.direction == "PUB":
                if var == "TERM":
                    etadbus.ContextDestroy()
                    break
                elif var == "START" or var == "STOP":
                    print("Not a valid CMD in PUB context!")
                else:
                    for _, topic in enumerate(args.topic):
                        topicConfig = {"name": topic, "type": "string"}
                        etadbus.Publish(topicConfig, var)
            elif args.direction == "SUB":
                if var == "STOP" and flag == "START":
                    for _, topic in enumerate(args.topic):
                        topicConfig = {"name": topic, "type": "string"}
                        etadbus.Subscribe(topicConfig, "STOP", cbFunc)
                    flag = "STOP"
                elif var == "START" and flag == "STOP":
                    for _, topic in enumerate(args.topic):
                        topicConfig = {"name": topic, "type": "string"}
                        etadbus.Subscribe(topicConfig, "START", cbFunc)
                    flag = "START"
                elif var == "TERM":
                    etadbus.ContextDestroy()
                    break
            continue
        except Exception as e:
            raise
