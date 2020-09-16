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
"""EIS ConfigManager Python mapping
"""

# Python imports
from .exc import *
import json
import logging

# Cython imports
from .libneweisconfigmgr cimport *
from .publisher cimport Publisher
from .subscriber cimport Subscriber
from .app_config import AppCfg
from .server cimport Server
from .client cimport Client


cdef class ConfigMgr:
    """EIS ConfigManager context object
    """
    cdef app_cfg_t* app_cfg
    cdef pub_cfg_t* pub_cfg
    cdef sub_cfg_t* sub_cfg
    cdef server_cfg_t* server_cfg
    cdef client_cfg_t* client_cfg

    def __init__(self):
        """Constructor
        """
        self.app_cfg = app_cfg_new()

    def __cinit__(self, *args, **kwargs):
        """Basic C init
        """
        pass

    def __dealloc__(self):
        """Deconstructor
        """
        if self.app_cfg != NULL:
            app_cfg_config_destroy(self.app_cfg)
        if self.pub_cfg != NULL:
            pub_cfg_config_destroy(self.pub_cfg)
        if self.sub_cfg != NULL:
            sub_cfg_config_destroy(self.sub_cfg)
        if self.server_cfg != NULL:
            server_cfg_config_destroy(self.server_cfg)
        if self.client_cfg != NULL:
            client_cfg_config_destroy(self.client_cfg)

    def get_app_config(self):
        """Calling base C get_app_config in order to get the 
        respective applications config

        :return: Return object of class AppCfg
        :rtype: obj
        """
        cdef config_t* conf
        cdef char* config

        if self.app_cfg == NULL:
            logging.info("app_cfg is NULL in config_manager")

        conf = get_app_config(self.app_cfg.base_cfg)
        config = configt_to_char(conf)
        config_str = config.decode('utf-8')
        cfg = json.loads(config_str)

        obj= AppCfg(cfg)
        return obj

    def get_app_interface(self):
        """Calling bace c get_app_interface in order to get the 
        respective applications interface
        
        :return: Return object of class AppCfg
        :rtype: obj
        """
        cdef config_t* conf
        cdef char* config

        if self.app_cfg == NULL:
            logging.info("app_cfg is NULL in config_manager")

        conf = get_app_interface(self.app_cfg.base_cfg)
        config = configt_to_char(conf)
        config_str = config.decode('utf-8')
        interface = json.loads(config_str)

        obj= AppCfg(interface)
        return obj

    def get_publisher_by_name(self, name):
        """Calling bace c get_publisher_by_name in order to get
        respective publisher config

        :param name: Name of the publisher 
        :type: string
        :return: Publisher class object
        :rtype : obj
        """
        if self.app_cfg == NULL:
            logging.info("app_cfg is NULL in config_manager")

        bname = bytes(name, 'utf-8')
        self.pub_cfg = cfgmgr_get_publisher_by_name(self.app_cfg, bname)
        if self.pub_cfg == NULL:
            logging.info("pub_cfg is NULL in config_manager")

        # Create & return Publisher object
        return Publisher.create(self.app_cfg, self.pub_cfg)

    def get_publisher_by_index(self, index):
        """Calling bace c get_publisher_by_index in order to get
        respective publisher config

        :param index: Index of the publisher 
        :type: int
        :return: Publisher class object
        :rtype : obj
        """
        if self.app_cfg == NULL:
            logging.info("app_cfg is NULL in config_manager")

        self.pub_cfg = cfgmgr_get_publisher_by_index(self.app_cfg, index)
        if self.pub_cfg == NULL:
            logging.info("pub_cfg is NULL in config_manager")

        # Create & return Publisher object
        return Publisher.create(self.app_cfg, self.pub_cfg)

    def get_subscriber_by_name(self, name):
        """Calling bace c get_subscriber_by_name in order to get
        respective publisher config

        :param name: Name of the subscriber
        :type: string
        :return: Subscriber class object
        :rtype : obj
        """
        if self.app_cfg == NULL:
            logging.info("app_cfg is NULL in config_manager")

        bname = bytes(name, 'utf-8')
        self.sub_cfg = cfgmgr_get_subscriber_by_name(self.app_cfg, bname)
        if self.sub_cfg == NULL:
            logging.info("sub_cfg is NULL in config_manager")

        # Create & return Publisher object
        return Subscriber.create(self.app_cfg, self.sub_cfg)

    def get_subscriber_by_index(self, index):
        """Calling bace c get_subscriber_by_index in order to get
        respective subscriber config

        :param index: Index of the subscriber 
        :type: int
        :return: Subscriber class object
        :rtype : obj
        """
        if self.app_cfg == NULL:
            logging.info("app_cfg is NULL in config_manager")

        self.sub_cfg = cfgmgr_get_subscriber_by_index(self.app_cfg, index)
        if self.sub_cfg == NULL:
            logging.info("sub_cfg is NULL in config_manager")

        # Create & return Subscriber object
        return Subscriber.create(self.app_cfg, self.sub_cfg)

    def get_server_by_name(self, name):
        """Calling bace c get_server_by_name in order to get
        respective publisher config

        :param name: Name of the server
        :type: string
        :return: Server class object
        :rtype : obj
        """
        if self.app_cfg == NULL:
            logging.info("app_cfg is NULL in config_manager")

        bname = bytes(name, 'utf-8')
        self.server_cfg = cfgmgr_get_server_by_name(self.app_cfg, bname)
        if self.server_cfg == NULL:
            logging.info("server_cfg is NULL in config_manager")

        # Create & return Publisher object
        return Server.create(self.app_cfg, self.server_cfg)

    def get_server_by_index(self, index):
        """Calling bace c get_server_by_index in order to get
        respective publisher config

        :param index: Index of the server
        :type: string
        :return: Server class object
        :rtype : obj
        """
        if self.app_cfg == NULL:
            logging.info("app_cfg is NULL in config_manager")

        self.server_cfg = cfgmgr_get_server_by_index(self.app_cfg, index)
        if self.server_cfg == NULL:
            logging.info("server_cfg is NULL in config_manager")

        # Create & return Publisher object
        return Server.create(self.app_cfg, self.server_cfg)

    def get_client_by_name(self, name):
        """Calling bace c get_server_by_name in order to get
        respective publisher config

        :param name: Name of the client
        :type: string
        :return: Client class object
        :rtype : obj
        """

        if self.app_cfg == NULL:
            logging.info("app_cfg is NULL in config_manager")

        bname = bytes(name, 'utf-8')
        self.client_cfg = cfgmgr_get_client_by_name(self.app_cfg, bname)
        if self.client_cfg == NULL:
            logging.info("client_cfg is NULL in config_manager")

        # Create & return Publisher object
        return Client.create(self.app_cfg, self.client_cfg)

    def get_client_by_index(self, index):
        """Calling bace c get_server_by_index in order to get
        respective publisher config

        :param index: Index of the client
        :type: int
        :return: Client class object
        :rtype : obj
        """

        if self.app_cfg == NULL:
            logging.info("app_cfg is NULL in config_manager")

        self.client_cfg = cfgmgr_get_client_by_index(self.app_cfg, index)
        if self.client_cfg == NULL:
            logging.info("client_cfg is NULL in config_manager")

        # Create & return Publisher object
        return Client.create(self.app_cfg, self.client_cfg)
