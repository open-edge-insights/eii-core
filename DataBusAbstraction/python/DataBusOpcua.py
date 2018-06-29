from opcua import ua, Server


OK = 0
ERR = -1


class datab_opc:
    def createContext(self, context_config):
        self.context_type = context_config["direction"]
        if self.context_type == "PUB":
            # For now no checking for existing server; Always assumes new server
            # for new context
            self.server = Server()
            # Create default endpoint protocol for opcua from given endpoint
            endpoint = context_config["endpoint"]
            endpoint = "opc.tcp://" + endpoint.split('//')[1]
            print(endpoint)
            self.server.set_endpoint(endpoint)
            self.ns = self.server.register_namespace(context_config["name"])
            self.server.start()
            return OK
        if self.context_type == "SUB":
            # Connect to server
            return OK
        # Set datab_state

    def startTopic(self, topic_config):
        if self.context_type == "PUB":
            objs = self.server.get_objects_node()
            # Search and create the topic as object
            obj_itr = objs
            topic_ls = topic_config["name"].split("/")
            for topic in topic_ls[0:]:
                try:
                    child = obj_itr.get_child("{}:{}".format(self.ns, topic))
                except Exception as e:
                    if type(e).__name__ == "BadNoMatch":
                        child = obj_itr.add_object(self.ns, topic)
                    else:
                        return ERR
                obj_itr = child
            # For now only String type
            var = obj_itr.add_variable(self.ns, "{}_var".format(topic_ls[-1]), "NONE", ua.VariantType.String)
            var.set_writable()
            return OK
        if self.context_type == "SUB":
            # TODO
            return OK

    def send(self, topic, data):
        if self.context_type == "PUB":
            objs = self.server.get_objects_node()
            obj_itr = objs
            topic_ls = topic.split("/")
            for topic in topic_ls[0:]:
                try:
                    child = obj_itr.get_child("{}:{}".format(self.ns, topic))
                except Exception as e:
                    print(type(e).__name__)
                    return ERR
                obj_itr = child
            # Now get the variable object
            var = obj_itr.get_child("{}:{}".format(self.ns,
                                                   "{}_var".format(topic_ls[-1])))
            if type(data) == str:
                var.set_value(data)
            return OK

    def receive(self, topic, handler):
        return OK

    def stopTopic(self, topic):
        # if self.context_type == "PUB":
        #    objs = self.server.get_objects_node()
        #    obj_itr = objs
        #    topic_ls = topic.split("/")
        #    for topic in topic_ls[0:]:
        #        try:
        #            child = obj_itr.get_child("{}:{}".format(self.ns, topic))
        #        except Exception as e:
        #            print(type(e).__name__)
        #            return
        #        obj_itr = child
        #    # Now get & delete the variable object
        #    var = obj_itr.get_child("{}:{}".format(self.ns,
        #                                               "{}_var".format(topic_ls[-1])))
        #    # delete the var & object
        #    var.delete()

        #    obj_itr.delete()
        return OK

    def destroyContext(self):
        if self.context_type == "PUB":
            print("OPCUA Server Stopping...")
            self.server.stop()
            print("OPCUA Server Stopped")
        # if self.context_type == "SUB":
            # return
        return OK

    def __init__(self):
        self.server = None
