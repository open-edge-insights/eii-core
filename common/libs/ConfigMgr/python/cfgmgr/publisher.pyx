# Copyright (c) 2020 Intel Corporation.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
"""EII Message Bus Publisher wrapper object
"""

import json

from .libeiiconfigmanager cimport *
from libc.stdlib cimport malloc
from libc.stdlib cimport free
from .util cimport Util
import logging


cdef class Publisher:
    """EII Message Bus Publisher object
    """

    def __init__(self):
        """Constructor
        """
        pass

    def __cinit__(self, *args, **kwargs):
        """Cython base constructor
        """
        self.cfgmgr_interface = NULL

    @staticmethod
    cdef create(cfgmgr_interface_t* cfgmgr_interface):
        """Helper method for initializing the client object.

        :param cfgmgr_interface: Publisher config struct
        :type: struct
        :return: Publisher class object
        :rtype: obj
        """
        p = Publisher()
        p.cfgmgr_interface = cfgmgr_interface
        return p

    def __dealloc__(self):
        """Cython destructor
        """
        self.destroy()

    def destroy(self):
        """Destroy the publisher.
        """
        if self.cfgmgr_interface != NULL:
            cfgmgr_interface_destroy(self.cfgmgr_interface)

    def get_msgbus_config(self):
        """Constructs message bus config for Publisher

        :return: Messagebus config
        :rtype: dict
        """
        cdef char* config
        cdef config_t* msgbus_config
        try:
            msgbus_config = cfgmgr_get_msgbus_config(self.cfgmgr_interface)
            if msgbus_config is NULL:
                raise Exception("[Publisher] Getting msgbus config from base c layer failed")

            config = configt_to_char(msgbus_config)
            if config is NULL:
                raise Exception("[Publisher] config failed to get converted to char")

            config_str = config.decode('utf-8')
            free(config)
            config_destroy(msgbus_config)
            return json.loads(config_str)
        except Exception as ex:
            raise ex

    def get_interface_value(self, key):
        """To get particular interface value from Publisher interface config

        :param key: Key on which interface value will be extracted
        :type: string
        :return: Interface value
        :rtype: string
        """
        cdef config_value_t* value
        cdef char* config
        try:
            interface_value = None
            value = cfgmgr_get_interface_value(self.cfgmgr_interface, key.encode('utf-8'))
            if value is NULL:
                raise Exception("[Publisher] Getting interface value from base c layer failed")
                

            interface_value = Util.get_cvt_data(value)
            if interface_value is None:
                config_value_destroy(value)
                raise Exception("[Publisher] Getting cvt data failed")

            config_value_destroy(value)
            return interface_value
        except Exception as ex:
            raise ex

    def get_endpoint(self):
        """To get endpoint for particular publisher from its interface config

        :return: Endpoint config
        :rtype: string
        """
        cdef config_value_t* ep
        cdef char* c_endpoint
        try:
            ep = cfgmgr_get_endpoint(self.cfgmgr_interface)
            if ep is NULL:
                raise Exception("[Publisher] Getting end point from base c layer failed")

            if(ep.type == CVT_OBJECT):
                config = cvt_to_char(ep);
                if config is NULL:
                    config_value_destroy(ep)
                    raise Exception("[Publisher] Config cvt to char conversion failed")

                config_str = config.decode('utf-8')
                endpoint = json.loads(config_str)
            elif(ep.type == CVT_STRING):
                c_endpoint = ep.body.string
                if c_endpoint is NULL:
                    raise Exception("[Publisher] Endpoint getting string value failed")
                endpoint = c_endpoint.decode('utf-8')
            else:
                endpoint = None
                config_value_destroy(ep)
                raise TypeError("[Publisher] Type mismatch: EndPoint should be string or dict type")

            config_value_destroy(ep)
            return endpoint
        except TypeError as type_ex:
            raise type_ex
        except Exception as ex:
            raise ex

        
    def get_topics(self):
        """To get topics from publisher interface config on which data will be published
        :return: List of topics
        :rtype: List
        """
        topics_list = []
        cdef config_value_t* topics
        cdef config_value_t* topic_value
        cdef char* c_topic_value

        try:
            topics = cfgmgr_get_topics(self.cfgmgr_interface)
            if topics is NULL:
                raise Exception("[Publisher] Getting topics from base c layer failed")
            
            for i in range(config_value_array_len(topics)):
                topic_value = config_value_array_get(topics, i)
                if topic_value is NULL:
                    config_value_destroy(topics)
                    raise Exception("[Publisher] Getting array value from config for topics failed")

                c_topic_value = topic_value.body.string
                if c_topic_value is NULL:
                    config_value_destroy(topics)
                    raise Exception("[Publisher] String value of topics is NULL")

                topic_val = c_topic_value.decode('utf-8')

                topics_list.append(topic_val)
                config_value_destroy(topic_value)
            config_value_destroy(topics)
            return topics_list
        except Exception as ex:
            raise ex


    def get_allowed_clients(self):
        """To get the names of the clients allowed to get publishers data
        
        :return: List of clients
        :rtype: List
        """
        clients_list = []
        cdef config_value_t* clients
        cdef config_value_t* client_value
        cdef char* c_client_value

        try:
            clients = cfgmgr_get_allowed_clients(self.cfgmgr_interface)
            if clients is NULL:
                raise Exception("[Publisher] Getting alowed clients from base c layer failed")

            for i in range(config_value_array_len(clients)):
                client_value = config_value_array_get(clients, i)
                if client_value is NULL:
                    config_value_destroy(clients)
                    raise Exception("[Publisher] Getting array value from config for allowed clients failed.")

                c_client_value = client_value.body.string
                if c_client_value is NULL:
                    config_value_destroy(clients)
                    raise Exception("[Publisher] String value of client is NULL")

                client_val = c_client_value.decode('utf-8')
                clients_list.append(client_val)

                config_value_destroy(client_value)
            config_value_destroy(clients)
            return clients_list
        except Exception as ex:
            raise ex


    def set_topics(self, topics_list):
        """To set new topics for publisher in publishers interface config

        :return: whether topic is set
        :rtype: int
        """
        cdef int topics_set
        cdef char** topics_to_be_set
        try:
            # Allocate memory
            topics_to_be_set = <char**>malloc(len(topics_list) * sizeof(char*))
            # Check if allocation went fine
            if topics_to_be_set is NULL:
                raise Exception("[Publisher] Malloc Failed for New topics")
            # Convert str to char* and store it into our char**
            for i in range(len(topics_list)):
                topics_list[i] = topics_list[i].encode()
                topics_to_be_set[i] = topics_list[i]

            # Calling the base C cfgmgr_set_topics() API
            topics_set = cfgmgr_set_topics(self.cfgmgr_interface, topics_to_be_set, len(topics_list))
            if not topics_set :
                raise Exception("[Publisher] Set Topics in base c layer failed")
            free(topics_to_be_set)
            return topics_set
        except Exception as ex:
            raise ex
            