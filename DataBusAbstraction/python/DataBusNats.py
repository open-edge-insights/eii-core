

OK = 0
ERR = -1


class datab_nats:
    def createContext(self, context_config):
        return OK

    def startTopic(self, topic_config):
        return OK

    def send(self, topic, data):
        return OK

    def receive(self, topic, handler):
        return OK

    def stopTopic(self, topic_config):
        return OK

    def destroyContext(self, context_config):
        return OK

    def __init__(self):
        return
