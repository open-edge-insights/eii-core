from DataBusMqtt import datab_mqtt
from DataBusOpcua import datab_opc
from DataBusNats import datab_nats
# context_config
#   "direction": "PUB/SUB"
#   "name": "<unique string>"
#   "endpoint": "<address>"
#       eg: "opcua://0.0.0.0:4840/elephanttrunk/"
#       "mqtt://localhost:1883/"
#       "nats://127.0.0.1:4222/"
# topic_config
#   "name": "<root_name/sub1/sub2/sub3>"
#   "type": "string"
OK = 0
ERR = -1
datab_types = {"OPCUA": "opcua:", "MQTT": "mqtt:", "NATS": "nats:"}


class databus:
    def ContextCreate(self, context_config):
        '''Create a Messagebus context to workwith'''
        endpoint = context_config["endpoint"]
        if endpoint.split('//')[0] == datab_types["OPCUA"]:
            self.datab_type = datab_types["OPCUA"]
            self.datab = datab_opc()
            print(datab_types["OPCUA"])
        elif endpoint.split('//')[0] == datab_types["MQTT"]:
            self.datab_type = datab_types["MQTT"]
            self.datab = datab_mqtt()
            print(datab_types["MQTT"])
        elif endpoint.split('//')[0] == datab_types["NATS"]:
            self.datab_type = datab_types["NATS"]
            self.datab = datab_nats()
            print(datab_types["NATS"])
        else:
            return ERR
        print(context_config)
        self.datab.createContext(context_config)
        if context_config["direction"] == "PUB":
            self.datab_context = "CONTEXT_PUB"
        elif context_config["direction"] == "SUB":
            self.datab_context = "CONTEXT_SUB"
        else:
            return ERR
        return OK

    def Publish(self, topic_config, data):
        '''Publish data'''
        # TODO: Check for supported data type
        if self.datab_context == "CONTEXT_PUB":
            if not self.checkMsgType(topic_config["type"], data):
                return ERR
            if topic_config["name"] not in self.datab_topics.keys():
                self.datab.startTopic(topic_config)
                self.datab_topics[topic_config["name"]] = topic_config["type"]
            # Now datab_topics has the topic for sure. (either created now/was
            # present. Now check for type
            if self.datab_topics[topic_config["name"]] != topic_config["type"]:
                # TODO: This is an ERROR
                return ERR
            # We are good to publish now
            self.datab.send(topic_config["name"], data)
            return OK
        else:
            return ERR

    def Subscribe(self, topic_config, handler):
        if self.datab_context == "CONTEXT_SUB":
            # TODO:
            return OK

    def ContextDestroy(self):
        '''Destroy the Messagebus context created'''
        # self.datab.stop_topic()
        self.datab.destroyContext()
        return OK

    def checkMsgType(self, topicType, msgData):
        if type(msgData) == str:
            if topicType == "string":
                return True
            else:
                return False

    def __init__(self):
        self.datab_type = None
        self.datab_context = "CONTEXT_NONE"
        self.datab_topics = {}
        self.datab = None
