import time
import sys
import os
path = os.path.abspath(__file__)
sys.path.append(os.path.join(os.path.dirname(path), "../"))

from DataBus import databus


def topic1handlerfn(val):
    print("topic1: {}".format(val))


def topic2handlerfn(val):
    print("topic2: {}".format(val))


def topic3handlerfn(val):
    print("topic3: {}".format(val))


if __name__ == "__main__":
    opc_context = {
        "direction": "SUB",
        "name": "streammanager",
        "endpoint": "opcua://0.0.0.0:4840/elephanttrunk/"
    }
    mqtt_context = {
        "direction": "SUB",
        "name": "streammanager",
        "endpoint": "mqtt://localhost:1883/"
    }
    topicConfig1 = {
        "name": "streammanager/first/time/test",
        "type": "string"
    }
    topicConfig2 = {
        "name": "streammanager/second/test2",
        "type": "string"
    }
    topicConfig3 = {
        "name": "streammanager/second/time/test3",
        "type": "string"
    }
    try:
        opc_datab = databus()
        mqtt_datab = databus()
        opc_datab.ContextCreate(opc_context)
        mqtt_datab.ContextCreate(mqtt_context)
    except Exception as e:
        print(e)
        sys.exit(-1)
    itr = 1
    while True:
        time.sleep(1)
        var = raw_input("Enter Cmd: ")
        print("Entered: ", var)
        if var == "OPC SUB START" or var == "SUB START":
            try:
                opc_datab.Subscribe(topicConfig1, "START", topic1handlerfn)
            except Exception as e:
                print(e)
                pass
            try:
                opc_datab.Subscribe(topicConfig2, "START", topic2handlerfn)
            except Exception as e:
                print(e)
                pass
            try:
                opc_datab.Subscribe(topicConfig3, "START", topic3handlerfn)
            except Exception as e:
                print(e)
                pass
        if var == "MQTT SUB START" or var == "SUB START":
            try:
                mqtt_datab.Subscribe(topicConfig1, "START", topic1handlerfn)
            except Exception as e:
                print(e)
                pass
            try:
                mqtt_datab.Subscribe(topicConfig2, "START", topic2handlerfn)
            except Exception as e:
                print(e)
                pass
            try:
                mqtt_datab.Subscribe(topicConfig3, "START", topic3handlerfn)
            except Exception as e:
                print(e)
                pass

        if var == "OPC SUB STOP" or var == "SUB STOP":
            try:
                opc_datab.Subscribe(topicConfig1, "STOP")
            except Exception as e:
                print(e)
                pass
            try:
                opc_datab.Subscribe(topicConfig2, "STOP")
            except Exception as e:
                print(e)
                pass
            try:
                opc_datab.Subscribe(topicConfig3, "STOP")
            except Exception as e:
                print(e)
                pass
        if var == "MQTT SUB STOP" or var == "SUB STOP":
            try:
                mqtt_datab.Subscribe(topicConfig1, "STOP")
            except Exception as e:
                print(e)
                pass
            try:
                mqtt_datab.Subscribe(topicConfig2, "STOP")
            except Exception as e:
                print(e)
                pass
            try:
                mqtt_datab.Subscribe(topicConfig3, "STOP")
            except Exception as e:
                print(e)
                pass
        if var == "Terminate":
            try:
                opc_datab.ContextDestroy()
                mqtt_datab.ContextDestroy()
                sys.exit(0)
            except Exception as e:
                print(e)
                sys.exit(-1)
