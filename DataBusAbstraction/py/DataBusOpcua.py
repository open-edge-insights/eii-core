from opcua import ua, Server, Client


class subHandler(object):
    '''cb object on message arrival'''

    def datachange_notification(self, node, val, data):
        # print(val)
        if not self.ignore_notification:
            self.queue.put(val)
        else:
            # Only ignore first notification
            self.ignore_notification = False

    def event_notification(self, event):
        print(event)

    def __init__(self, queue):
        self.queue = queue
        self.ignore_notification = True


class databOpcua:
    '''Creates and manages a databus OPCUA context'''

    def createContext(self, contextConfig):
        '''Creates a new messagebus context
        Arguments:
            contextConfig<dict>: Messagebus params to create the context
                <fields>
                "direction": PUB/SUB/NONE - Mutually exclusive
                "name": context namespace (PUB/SUB context namespaces should match)
                "endpoint": messagebus endpoint address
                    <format> proto://host:port/, proto://host:port/.../
                    <examples>
                    OPCUA -> opcua://0.0.0.0:65003/elephanttrunk/
                    MQTT -> mqtt://localhost:1883/
                    NATS -> nats://127.0.0.1:4222/
        Return/Exception: Will raise Exception in case of errors'''

        self.direction = contextConfig["direction"]
        if self.direction == "PUB":
            # Create default endpoint protocol for opcua from given endpoint
            endpoint = contextConfig["endpoint"]
            endpoint = "opc.tcp://" + endpoint.split('//')[1]
            print("PUB:: ", endpoint)
            try:
                # TODO: For now no checking for existing server;
                # Always assumes new server for new context
                self.server = Server()
                self.server.set_endpoint(endpoint)
                # TODO: Check for existing namespace in same name
                self.ns = self.server.register_namespace(contextConfig["name"])
                self.server.start()
                self.objNode = self.server.get_objects_node()
            except Exception as e:
                print("{} Failure!!!".format(self.createContext.__name__))
                print(type(e).__name__)
                raise
        elif self.direction == "SUB":
            # Create default endpoint protocol for opcua from given endpoint
            endpoint = contextConfig["endpoint"]
            endpoint = "opc.tcp://" + endpoint.split('//')[1]
            print("SUB:: ", endpoint)
            try:
                # Connect to server
                self.client = Client(endpoint)
                self.client.connect()
                # TODO: Can we avoid/delay the exception if no such namespace?
                self.ns = self.client.get_namespace_index(contextConfig["name"])
                self.objNode = self.client.get_objects_node()
            except Exception as e:
                print("{} Failure!!!".format(self.createContext.__name__))
                print(type(e).__name__)
                raise
        else:
            raise Exception("Wrong Bus Direction!!!")

    def startTopic(self, topicConfig):
        '''Topic creation for the messagebus
        Arguments:
            topicConfig<dict>: Publish topic parameters
                <fields>
                "name": Topic name (in hierarchical form with '/' as delimiter)
                    <example> "root/level1/level2/level3"
                "type": Data type associated with the topic
        Return/Exception: Will raise Exception in case of errors'''

        if self.direction == "PUB":
            # objs = self.server.get_objects_node()
            # Search and create the topic as object
            obj_itr = self.objNode
            topic_ls = topicConfig["name"].split("/")
            for topic in topic_ls[0:]:
                try:
                    child = obj_itr.get_child("{}:{}".format(self.ns, topic))
                except Exception as e:
                    if type(e).__name__ == "BadNoMatch":
                        child = obj_itr.add_object(self.ns, topic)
                    else:
                        print("{} Failure!!!".format(self.startTopic.__name__))
                        print(type(e).__name__)
                        raise
                obj_itr = child
            try:
                # TODO: For now only String type
                var = obj_itr.add_variable(self.ns, "{}_var".format(topic_ls[-1]), "NONE", ua.VariantType.String)
                var.set_writable()
            except Exception as e:
                print("{} Failure!!!".format(self.startTopic.__name__))
                print(type(e).__name__)
                raise
            # TODO: Convert into opcua type
            self.pubElements[topicConfig["name"]] = {"node": var,
                                                     "type": topicConfig["type"]}
        elif self.direction == "SUB":
            # TODO: For now raise exception for non-existing server vars
            # We could avoid this exception by a polling mechanism till such
            # vars appear in server. But thats for later, may be
            topic_ls = topicConfig["name"].split("/")
            for idx, item in enumerate(topic_ls):
                topic_ls[idx] = "{}:{}".format(self.ns, item)
            topic_ls.append("{}_var".format(topic_ls[-1]))
            print(topic_ls)
            self.subElements[topicConfig["name"]] = {"node": None, "type": None}
            # TODO: Convert into opcua type
            self.subElements[topicConfig["name"]]["type"] = topicConfig["type"]
            try:
                self.subElements[topicConfig["name"]]["node"] = \
                    self.objNode.get_child(topic_ls)
                # TODO: Compare data type
            except Exception as e:
                print("{} Failure!!!".format(self.startTopic.__name__))
                print(type(e).__name__)
                raise
        else:
            raise Exception("Wrong Bus Direction!!!")

    def send(self, topic, data):
        '''Publish data on the topic
        Arguments:
            topic: topic name as in the topicConfig
            data: actual message
        Return/Exception: Will raise Exception in case of errors'''

        if self.direction == "PUB":
            # TODO: Support for different data types
            if type(data) == str:
                try:
                    self.pubElements[topic]["node"].set_value(data)
                except Exception as e:
                    print("{} Failure!!!".format(self.send.__name__))
                    print(type(e).__name__)
                    raise
            else:
                raise Exception("Wrong Data Type!!!")
        else:
            raise Exception("Wrong Bus Direction!!!")

    def receive(self, topic, trig, queue=None):
        '''Subscribe data from the topic
        Arguments:
            topic: topic name as in the topicConfig
            trig: START/STOP to start/stop the subscription
            queue: A queue to which the message should be pushed on arrival
        Return/Exception: Will raise Exception in case of errors'''

        if (self.direction == "SUB") and (trig == "START") and (queue is not None):
            # TODO: pass the queue
            handler = subHandler(queue)
            try:
                # TODO: Handle multiple subscription to same topic
                self.subElements[topic]["sub"] = self.client.create_subscription(250, handler)
                self.subElements[topic]["handle"] = \
                    self.subElements[topic]["sub"].subscribe_data_change(self.subElements[topic]["node"])
            except Exception as e:
                print("{} Failure!!!".format(self.receive.__name__))
                print(type(e).__name__)
                raise
        elif (self.direction == "SUB") and (trig == "STOP"):
            try:
                self.subElements[topic]["sub"].unsubscribe(self.subElements[topic]["handle"])
                self.subElements[topic]["sub"].delete()
                self.subElements[topic]["handle"] = None
                self.subElements[topic]["sub"] = None
            except Exception as e:
                print("{} Failure!!!".format(self.receive.__name__))
                print(type(e).__name__)
                raise
        else:
            raise Exception("Wrong Bus Direction or Trigger!!!")

    def stopTopic(self, topic):
        '''Delete topic
        Arguments:
            topicConfig<dict>: Topic parameters
                <fields>
                "name": Topic name (in hierarchical form with '/' as delimiter)
                    <example> "root/level1/level2/level3"
                "type": Data type associated with the topic
        Return/Exception: Will raise Exception in case of errors'''

        # TODO:
        # if self.direction == "PUB":
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
        return

    def destroyContext(self):
        '''Destroy the messagebus context'''

        if self.direction == "PUB":
            print("OPCUA Server Stopping...")
            try:
                self.server.stop()
            except Exception as e:
                print("{} Failure!!!".format(self.destroyContext.__name__))
                print(type(e).__name__)
                raise
            print("OPCUA Server Stopped")
        elif self.direction == "SUB":
            try:
                print("OPCUA Client Disconnecting...")
                self.client.disconnect()
                print("OPCUA Client Disconnected")
            except Exception as e:
                print("{} Failure!!!".format(self.destroyContext.__name__))
                print(type(e).__name__)
                raise
        else:
            raise Exception("Wrong Bus Direction!!!")

    def __init__(self):
        self.server = None
        self.objNode = None
        self.ns = None
        self.subElements = {}
        self.pubElements = {}
