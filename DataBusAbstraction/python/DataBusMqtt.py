import paho.mqtt.client as mqtt


OK = 0
ERR = -1


class datab_mqtt:
    def createContext(self, context_config):
        self.context_type = context_config["direction"]
        if self.context_type == "PUB":
            self.client = mqtt.Client()
            # cb registrations
            # self.client.on_publish = self._on_publish
            # self.client.on_connect = self._on_connect
            # Create default endpoint protocol for mqtt from given endpoint
            endpoint = context_config["endpoint"]
            host = endpoint.split('//')[1].split(':')[0]
            port = endpoint.split('//')[1].split(':')[1].split('/')[0]
            print(host, port)
            self.client.connect(host, int(port))
            return OK
        if self.context_type == "SUB":
            return OK

    def startTopic(self, topic_config):
        if self.context_type == "PUB":
            return OK
        if self.context_type == "SUB":
            return OK

    def send(self, topic, data):
        if type(data) == str:
            self.client.publish(topic, data)
        return OK

    def receive(self, topic, handler):
        return OK

    def stopTopic(self, topic_config):
        if self.context_type == "PUB":
            return OK
        if self.context_type == "SUB":
            return OK

    def destroyContext(self):
        if self.context_type == "PUB":
            print("MQTT Client Disconnecting...")
            self.client.disconnect()
            print("MQTT Client Disconnected")
        # if self.context_type == "SUB":
        #    return
        return OK

    def __init__(self):
        self.client = None
        return
