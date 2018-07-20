import time
import sys
import os
path = os.path.abspath(__file__)
sys.path.append(os.path.join(os.path.dirname(path), "../"))

from DataBus import databus


if __name__ == "__main__":
    opc_context = {
        "direction": "PUB",
        "name": "streammanager",
        "endpoint": "opcua://0.0.0.0:4840/elephanttrunk/"
    }
    mqtt_context = {
        "direction": "PUB",
        "name": "streammanager",
        "endpoint": "mqtt://localhost:1883/"
    }
    topic_config = {
        "name": "streammanager/first/time/test",
        "type": "string"
    }
    topic_config2 = {
        "name": "streammanager/second/test2",
        "type": "string"
    }
    topic_config3 = {
        "name": "streammanager/second/time/test3",
        "type": "string"
    }
    opc_datab = databus()
    mqtt_datab = databus()
    opc_datab.ContextCreate(opc_context)
    mqtt_datab.ContextCreate(mqtt_context)
    itr = 1
    while True:
        time.sleep(1)
        var = raw_input("Enter New Value: ")
        if var == "Terminate":
            opc_datab.ContextDestroy()
            mqtt_datab.ContextDestroy()
            break
        if itr == 1:
            print("Publish \'{}\' to topic \'{}\'".format(var, topic_config["name"]))
            opc_datab.Publish(topic_config, var)
            mqtt_datab.Publish(topic_config, var)
            itr = 2
            continue
        if itr == 2:
            print("Publish \'{}\' to topic \'{}\'".format(var, topic_config2["name"]))
            opc_datab.Publish(topic_config2, var)
            mqtt_datab.Publish(topic_config2, var)
            itr = 3
            continue
        if itr == 3:
            print("Publish \'{}\' to topic \'{}\'".format(var, topic_config3["name"]))
            opc_datab.Publish(topic_config3, var)
            mqtt_datab.Publish(topic_config3, var)
            itr = 1
            continue
